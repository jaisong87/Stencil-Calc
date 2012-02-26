#include<iostream>
#include<iomanip>
using namespace std;
bool enableDebug = true;

/* check for mem-issues*/
bool initArray(float ***& threeDimSpace, int xD, int yD, int zD)
{
	if(enableDebug)
		cout<<"Creating "<<xD<<"X"<<yD<<"X"<<zD<<" space for stencil computations"<<endl;

	threeDimSpace = new float**[xD];

	for(int i=0;i<xD;i++)
	{
		threeDimSpace[i] = new float*[yD];
		for(int j=0;j<yD;j++)
			threeDimSpace[i][j] = new float[zD];
	}
	return true; /* Success! */
}

/* To Deallocate the array */
bool cleanArray(float ***& threeDimSpace, int xD, int yD, int zD)
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
void printResult(float *** threeDimSpace, int xD, int yD, int zD)
{
for(int x=0;x<xD;x++,cout<<endl)
	for(int y=0;y<yD;y++,cout<<endl)
		for(int z=0;z<zD;z++)
			{						
				cout<<setw(10)<<threeDimSpace[x][y][z];			
			}
return;
}

/* Copy original Space to a buffer 
 * There is no check for mallocs, frees, wild-ptrs etc
 * Use all the functions carefully
 */
void copySpace(float ***& src, float***& dst, int xD, int yD, int zD)
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
void computeStencil(float ***& threeDimSpace, float c0, float c1, float c2, float c3, int tf, int xD, int yD, int zD)
{
float *** tmpSpace;
initArray(tmpSpace, xD, yD, zD);
for(int t=1;t<=tf;t++)
	{
	copySpace(threeDimSpace, tmpSpace, xD, yD, zD);
	
	for(int x =0;x<xD;x++)
		for(int y=0;y<yD;y++)
			for(int z=0;z<zD;z++)
				{
				float tmp = c0*tmpSpace[x][y][z];

				if(z<(zD-1)) tmp+=(c1+c2+c3)*tmpSpace[x][y][z+1];
				if(z>0) tmp+=(c1+c2+c3)*tmpSpace[x][y][z-1];
	
			
				if(y<(yD-1)) tmp+=(c1+c2+c3)*tmpSpace[x][y+1][z];
				if(y>0) tmp+=(c1+c2+c3)*tmpSpace[x][y-1][z];
				
				if(x<(xD-1)) tmp+=(c1+c2+c3)*tmpSpace[x+1][y][z];
				if(x>0) tmp+=(c1+c2+c3)*tmpSpace[x-1][y][z];
				
				threeDimSpace[x][y][z] = tmp;
				}
	}
}

int main()
{
int n, tf;
float c0,c1,c2,c3; 
float *** threeDimSpace;
while(cin>>n>>tf)	/* Take Arbitrary number of inputs through stdin */
	{
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
