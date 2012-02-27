#include<iostream>
#include<sstream>
#include<iomanip>
#include<mpi.h>
using namespace std;
bool enableDebug = false;
bool dbugFlag2 = false;
int rank;
stringstream myStream;

extern "C++" bool initArray_CPP(float ***& threeDimSpace, int xD, int yD, int zD);
extern "C++" bool cleanArray_CPP(float ***& threeDimSpace, int xD, int yD, int zD);
extern "C++" void printSerialBuffer_CPP(float *buf, int len);

extern "C++" void printResult_CPP(float *** threeDimSpace, int xD, int yD, int zD);
extern "C++" void copySpace_CPP(float ***& src, float***& dst, int xD, int yD, int zD);
extern "C++" void computeStencil_CPP(float ***& threeDimSpace, float c0, float c1, float c2, float c3, int tf, int xD, int yD, int zD);

extern "C++" void serialize_CPP(float*& tmp,float*** origSpace, int dx, int dy, int dz);
extern "C++" bool deserializeBuffer_CPP(float***& mySpace, float* buf, int dx, int dy, int dz);


int main(int argc, char* argv[])
{
int n=4, q,  tf, dx;
float c0,c1,c2,c3; 
float *** threeDimSpace;
float*** finalResult;
float *** workChunk;	

int workerCount = 4; /* This means we need workRatio workers */

int myRank, totWorkers;
MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &totWorkers);
MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

//myStream<<"My rank is "<<myRank<<" out of "<<totWorkers<<endl;

//while
if(myRank == 0)
{
	if(enableDebug)
	{
		stringstream ss1;
		if(dbugFlag2) ss1<<"I'm processor "<<myRank<<" doing all the work"<<endl;
		myStream<<ss1.str();
	}

	if(cin>>q>>tf)	/* Take Arbitrary number of inputs through stdin */
	{
		myStream<<" =================== MASTER ========================= "<<endl;
		n = q;//(1<<q);
		cin>>c0>>c1>>c2>>c3;
		initArray_CPP(threeDimSpace, n, n, n );    /* Initialize the array */

		for(int i=0;i<n;i++)	/* Get the Input-(original space) */
			for(int j=0;j<n;j++)
				for(int k=0;k<n;k++)
					cin>>threeDimSpace[i][j][k];

		//computeStencil_CPP(threeDimSpace, c0, c1, c2, c3, tf, n , n, n );

		//printResult_CPP(threeDimSpace, n, n, n);	/* Print the Result after Computation */
		//cleanArray_CPP(threeDimSpace, n, n, n);	/* Cleanup the array */
		dx = n/workerCount;
		if(dbugFlag2) myStream<<dx<<" sized chunks for "<<workerCount<<" workers"<<endl;	

		for(int w=1;w<=workerCount;w++)
		{
			if(dbugFlag2) myStream<<"Sending N to process "<<w<<endl;
			MPI_Send(&n, 1, MPI_INT, w, 1, MPI_COMM_WORLD);
		}

		float* buf = new float[dx*n*n];	

		for(int w=1;w<=workerCount;w++)
		{
			serialize_CPP(buf, &threeDimSpace[(w-1)*dx], dx, n, n);
			printSerialBuffer_CPP( buf , dx*n*n);	

			if(dbugFlag2) myStream<<"Sending workChunk of size "<<dx*n*n<<" to process "<<w<<endl;
			MPI_Send((void*)buf, dx*n*n , MPI_FLOAT, w, 2, MPI_COMM_WORLD);
			//printResult_CPP(&threeDimSpace[w*dx], dx, n, n);

		}
	}
}
else if(myRank>=1 && myRank <= workerCount  ) {
	myStream<<" ======================== SLAVE# "<<myRank<<"======================"<<endl;	
	//int tmpRank = myRank;
	
	MPI_Status recvStatus;
	
	MPI_Recv(&n,1, MPI_INT, 0, 1 , MPI_COMM_WORLD, &recvStatus);
	n= 4;
	
	dx = n/workerCount;
	initArray_CPP(workChunk, dx, n, n);	
	
	if(dbugFlag2) myStream<<"In process :"<<myRank<</*" tmpRank = "<<tmpRank<<*/" dx = "<<dx<<" n = "<<n<<endl;
	if(dbugFlag2) myStream<<"Process "<<myRank<<" : expecting serialize_CPPd buffer of size "<<dx*n*n<<" recieved from process0 "<<endl;	
		
	float* buf = new float[dx*n*n];	

	MPI_Recv((void*)buf, dx*n*n , MPI_FLOAT, 0, 2 , MPI_COMM_WORLD, &recvStatus);
	if(dbugFlag2) myStream<<"process "<<myRank<<" : serialized buffer of size "<<dx*n*n<<" recieved from process0 "<<endl;	
	//printSerialBuffer_CPP( buf , dx*n*n);	
	deserializeBuffer_CPP(workChunk, buf, dx, n, n);
	
	printResult_CPP(workChunk, dx, n, n);	

		}
else {
if(enableDebug)
	{
stringstream ss1;
if(dbugFlag2) ss1<<"I'm processor "<<myRank<<" idle without any of the work"<<endl;
myStream<<ss1.str();
	}
}

stringstream ss2;
if(dbugFlag2) ss2<<"processor "<<myRank<<" finished the job"<<endl;
myStream<<ss2.str();

myStream<<"==============================================================="<<endl;

MPI_Barrier(MPI_COMM_WORLD);

/* We are donw tih the input . Now Lets start MPI - Phase2 All Calculations */



/* Phase 3 - Assemble it back and print*/
if(myRank == 0)
	{
initArray_CPP(finalResult, n, n, n);

MPI_Status recvStatus;

float* buf = new float[dx*n*n];
for(int i=1;i<=workerCount;i++)
		{
		MPI_Recv((void*)buf, dx*n*n, MPI_FLOAT, i, 3,  MPI_COMM_WORLD, &recvStatus);
		//deserializeBuffer_CPP(&finalResult[(i-1)*dx], buf, dx, n, n);		
		int ctr = 0;
		for(int x=0;x<dx;x++)
			for(int y=0;y<n;y++)
				for(int z=0;z<n;z++)
					{
					finalResult[(i-1)*dx+x][y][z] = buf[ctr];
					ctr++;
					}

		myStream<<" Process#"<<myRank<<"Recieved data from process "<<i<<endl;
		}
	printResult_CPP(finalResult, n, n, n);
	}
else if(myRank>=1 && myRank<=workerCount)
	{
	float* buf = new float[dx*n*n];	
	serialize_CPP(buf, workChunk, dx, n, n);	
	MPI_Send((void*)buf, dx*n*n, MPI_FLOAT, 0, 3, MPI_COMM_WORLD);
	}

MPI_Barrier(MPI_COMM_WORLD);
MPI_Finalize();
//cout<<myStream.str();
if(myRank == 0)
	{
		for(int i=0;i<n;i++)
			for(int j=0;j<n;j++,cout<<endl)
				for(int k=0;k<n;k++)
					cout<<finalResult[i][j][k]<<' ';
	}
return 0;
}
