#include<iostream>
#include<sstream>
#include<iomanip>
#include<mpi.h>
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
return true;
}

/* print Result */
void printResult(float *** threeDimSpace, int xD, int yD, int zD)
{
stringstream ss1;
for(int x=0;x<xD;x++,ss1<<endl)
	for(int y=0;y<yD;y++,ss1<<endl)
		for(int z=0;z<zD;z++)
			{						
				ss1<<setw(10)<<fixed<<threeDimSpace[x][y][z]<<' ';			
			}
cout<<ss1.str();
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

int main(int argc, char* argv[])
{
int n, q,  tf;
float c0,c1,c2,c3; 
float *** threeDimSpace;

int workerCount = 4; /* This means we need workRatio workers */

int myRank, totWorkers;
MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &totWorkers);
MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

cout<<"My rank is "<<myRank<<" out of "<<totWorkers<<endl;

//while
if(myRank == 0)
	{
if(enableDebug)
	{
stringstream ss1;
ss1<<"I'm processor "<<myRank<<" doing all the work"<<endl;
cout<<ss1.str();
	}

if(cin>>q>>tf)	/* Take Arbitrary number of inputs through stdin */
	{
	n = q;//(1<<q);
	cin>>c0>>c1>>c2>>c3;
	initArray(threeDimSpace, n, n, n );    /* Initialize the array */
		
	for(int i=0;i<n;i++)	/* Get the Input-(original space) */
		for(int j=0;j<n;j++)
			for(int k=0;k<n;k++)
				cin>>threeDimSpace[i][j][k];

	//computeStencil(threeDimSpace, c0, c1, c2, c3, tf, n , n, n );
	
	printResult(threeDimSpace, n, n, n);	/* Print the Result after Computation */
	//cleanArray(threeDimSpace, n, n, n);	/* Cleanup the array */
	int dx = n/workerCount;
	cout<<dx<<" sized chunks for "<<workerCount<<" workers"<<endl;	

	for(int w=1;w<=workerCount;w++)
		{
		cout<<"Sending N to process "<<w<<endl;
		//MPI_Send(&n, 1, MPI_INT, w, 1, MPI_COMM_WORLD);
		}
	
	/*	
	for(int w=0;w<workerCount;w++)
		{
		cout<<"Sending workChunk to process "<<w<<endl;
		MPI_Send(&threeDimSpace[w*dx], dx*n*n, MPI_FLOAT, w, 2, MPI_COMM_WORLD);
		}
	*/

	}
	}
else if(myRank <= workerCount  ) {
	float *** workChunk;	
	cout<<"Process "<<myRank<<" thinks  N = "<<n<<endl;	

	MPI_Status recvStatus;
	
	//MPI_Recv(&n,1, MPI_INT, 0, 1 , MPI_COMM_WORLD, &recvStatus);
	cout<<"Process "<<myRank<<" recieved N = "<<n<<endl;	
	
	/*
	int dx = n/workerCount;
	initArray(workChunk, dx, n, n);
	
	MPI_Recv(&workChunk, dx*n*n, MPI_FLOAT, 0, 2 , MPI_COMM_WORLD, &recvStatus);
	
	cout<<"Process : "<<myRank<<" recieved data from process0 "<<endl;

	printResult(workChunk, dx, n, n);
	*/

		}
else {
if(enableDebug)
	{
stringstream ss1;
ss1<<"I'm processor "<<myRank<<" idle without any of the work"<<endl;
cout<<ss1.str();
	}
}

MPI_Barrier(MPI_COMM_WORLD);

stringstream ss2;
ss2<<"processor "<<myRank<<" finished the job"<<endl;
cout<<ss2.str();
/* We are donw tih the input . Now Lets start MPI */
MPI_Finalize();
return 0;
}
