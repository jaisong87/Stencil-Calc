#include<iostream>
#include<iomanip>
using namespace std;
bool enableDebug = false;

/* check for mem-issues*/
bool initArray(double ***& threeDimSpace, int xD, int yD, int zD)
{
	if(enableDebug)
		cout<<"Creating "<<xD<<"X"<<yD<<"X"<<zD<<" space for stencil computations"<<endl;

	threeDimSpace = new double**[xD];

	for(int i=0;i<xD;i++)
	{
		threeDimSpace[i] = new double*[yD];
		for(int j=0;j<yD;j++)
			threeDimSpace[i][j] = new double[zD];
	}
	return true; /* Success! */
}

/* To Deallocate the array */
bool cleanArray(double ***& threeDimSpace, int xD, int yD, int zD)
{
	if(enableDebug)
		cout<<"Freeing "<<xD<<"X"<<yD<<"X"<<zD<<" space for stencil computations"<<endl;

	for(int i=0;i<xD;i++)
	{
		for(int j=0;j<yD;j++)
		{
			delete[] threeDimSpace[i][j];
		}	
		delete[] threeDimSpace[i];
	}
	delete[] threeDimSpace;
}

/* print Result */
void printResult(double *** threeDimSpace, int xD, int yD, int zD)
{
for(int x=0;x<xD;x++,cout<<endl)
	for(int y=0;y<yD;y++,cout<<endl)
		for(int z=0;z<zD;z++)
			{						
				cout<<setw(10)<<fixed<<threeDimSpace[x][y][z]<<' ';			
			}
return;
}

/* Copy original Space to a buffer 
 * There is no check for mallocs, frees, wild-ptrs etc
 * Use all the functions carefully
 */
void copySpace(double ***& src, double***& dst, int xD, int yD, int zD)
{
	if(enableDebug)
		cout<<"Copying from "<<xD<<"X"<<yD<<"X"<<zD<<" original space for stencil computations"<<endl;

for(int x=0;x<xD;x++)
	for(int y=0;y<yD;y++)
		for(int z=0;z<zD;z++)
			dst[x][y][z] = src[x][y][z];
return;
}

/*Perform stenicl Compuatations */
void computeStencil(double ***& threeDimSpace, double c0, double c1, double c2, double c3, int tf, int xD, int yD, int zD)
{
double xaxis, yaxis, zaxis;
double *** tmpSpace;
double C[4] = { c0,c1,c2,c3};
initArray(tmpSpace, xD, yD, zD);
for(int t=1;t<=tf;t++)
	{
	copySpace(threeDimSpace, tmpSpace, xD, yD, zD);
	
	for(int x =0;x<xD;x++)
		for(int y=0;y<yD;y++)
			for(int z=0;z<zD;z++)
			{
				xaxis = yaxis = zaxis = 0;
				double tmp = 0; 
				for(int dis=1;dis<=3;dis++)
				{
				xaxis = yaxis = zaxis = 0;

					if(z<(zD-dis)) { tmp+=(C[dis])*tmpSpace[x][y][z+dis];
								zaxis += tmpSpace[x][y][z+dis];
							}
					if(z>=dis) { tmp+=(C[dis])*tmpSpace[x][y][z-dis];
						zaxis += tmpSpace[x][y][z-dis];
							}

					if(y<(yD-dis)) { tmp+=(C[dis])*tmpSpace[x][y+dis][z];
								yaxis += tmpSpace[x][y+dis][z];
							}
					
					if(y>=dis) { tmp+=(C[dis])*tmpSpace[x][y-dis][z];
								yaxis += tmpSpace[x][y-dis][z];
							}

					if(x<(xD-dis)) { tmp+=(C[dis])*tmpSpace[x+dis][y][z];
								xaxis += tmpSpace[x+dis][y][z];
							}
					if(x>=dis) { tmp+=(C[dis])*tmpSpace[x-dis][y][z];
								xaxis += tmpSpace[x-dis][y][z];
							}

				//cout<<"<"<<x<<","<<y<<","<<z<<"> => "<<tmpSpace[x][y][z]<<' '<<xaxis<<' '<<yaxis<<' '<<zaxis<<' '<<C[dis]<<" => "<<tmp<<endl;
				}
				tmp+= c0*tmpSpace[x][y][z];
				//cout<<"<"<<x<<","<<y<<","<<z<<"> => "<<tmpSpace[x][y][z]<<' '<<xaxis<<' '<<yaxis<<' '<<zaxis<<' '<<c0<<" => "<<tmp<<endl;
				threeDimSpace[x][y][z] = tmp;
			}
	}
}

int main()
{
int n, q,  tf;
double c0,c1,c2,c3; 
double *** threeDimSpace;
if(cin>>q>>tf)	/* Take Arbitrary number of inputs through stdin */
	{
	n = q;//(1<<q);
	cin>>c0>>c1>>c2>>c3;
	initArray(threeDimSpace, n, n, n );    /* Initialize the array */
		
	for(int i=0;i<n;i++)	/* Get the Input-(original space) */
		for(int j=0;j<n;j++)
			for(int k=0;k<n;k++)
				cin>>threeDimSpace[i][j][k];

	computeStencil(threeDimSpace, c0, c1, c2, c3, tf, n , n, n );
	
	printResult(threeDimSpace, n, n, n);	/* Print the Result after Computation */
	cleanArray(threeDimSpace, n, n, n);	/* Cleanup the array */
	}
return 0;
}
