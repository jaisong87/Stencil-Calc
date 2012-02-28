/******************************************
 *
 * distributedMemStencil.cpp - Calculates Stencil
 * on distributed memory architecture using MPI
 *
 *****************************************/
#include<iostream>
#include<sstream>
#include<iomanip>
#include<mpi.h>
using namespace std;
bool enableDebug = false;
bool dbugFlag2 = false;
bool dumpSelfStream = false;
int rank;
stringstream myStream;

extern "C++" bool initArray_CPP(float ***& threeDimSpace, int xD, int yD, int zD);
extern "C++" bool cleanArray_CPP(float ***& threeDimSpace, int xD, int yD, int zD);
extern "C++" void printSerialBuffer_CPP(float *buf, int len);

extern "C++" void printResult_CPP(float *** threeDimSpace, int xD, int yD, int zD);
extern "C++" void copySpace_CPP(float ***& src, float***& dst, int xD, int yD, int zD);
//extern "C++" void computeStencil_CPP(float ***& threeDimSpace, float c0, float c1, float c2, float c3, int tf, int xD, int yD, int zD);

extern "C++" void serialize_CPP(float*& tmp,float*** origSpace, int dx, int dy, int dz);
extern "C++" bool deserializeBuffer_CPP(float***& mySpace, float* buf, int dx, int dy, int dz);

/* Main Code */
int main(int argc, char* argv[])
{
int n=4, q,  dx;
float c0 =1,c1=1,c2=1,c3=1, tf=1.0; 
float *** threeDimSpace;
float*** finalResult;
float *** workChunk, ***tmpChunk;	
float *buf;
int workerCount = 4; /* This means we need workRatio workers */

int myRank, totWorkers;
MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &totWorkers);
MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

/* The code takes only one instance of Input - For multiple input , we can have a flag and a while loop here for the whole code*/
if(myRank == 0)
{
	if(enableDebug)
	{
		stringstream ss1;
		if(dbugFlag2) ss1<<"I'm processor "<<myRank<<" co-ordinating all the work"<<endl;
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

		dx = n/workerCount;
		if(dbugFlag2) myStream<<dx<<" sized chunks for "<<workerCount<<" workers"<<endl;	

		for(int w=1;w<=workerCount;w++)
		{
			if(dbugFlag2) myStream<<"Sending N to process "<<w<<endl;
			MPI_Send(&n, 1, MPI_INT, w, 1, MPI_COMM_WORLD);
		}
		
		float params[5];
		params[0] = c0; 
		params[1] = c1; 
		params[2] = c2; 
		params[3] = c3; 
		params[4] = tf; 
		
		/* send N to all processors */	
		for(int w=1;w<=workerCount;w++)
                {
                        if(dbugFlag2) myStream<<"Sending N to process "<<w<<endl;
                        MPI_Send(&params, 5, MPI_FLOAT, w, 5, MPI_COMM_WORLD);
                }


		buf = new float[dx*n*n];	
		
		/* Send the workChunk to all the processors */
		for(int w=1;w<=workerCount;w++)
		{
			serialize_CPP(buf, &threeDimSpace[(w-1)*dx], dx, n, n);
			printSerialBuffer_CPP( buf , dx*n*n);	

			if(dbugFlag2) myStream<<"Sending workChunk of size "<<dx*n*n<<" to process "<<w<<endl;
			MPI_Send((void*)buf, dx*n*n , MPI_FLOAT, w, 2, MPI_COMM_WORLD);
		}
	}
}
else if(myRank>=1 && myRank <= workerCount  ) {
	myStream<<" ======================== SLAVE# "<<myRank<<"======================"<<endl;	
	
	MPI_Status recvStatus;
	
	MPI_Recv(&n,1, MPI_INT, 0, 1 , MPI_COMM_WORLD, &recvStatus);
	
	 float params[5];
         MPI_Recv(&params, 5, MPI_FLOAT, 0, 5, MPI_COMM_WORLD, &recvStatus);
                c0 = params[0];
                c1 = params[1];
                c2 = params[2];
                c3 = params[3];
                tf = params[4];
	
	dx = n/workerCount;
	initArray_CPP(workChunk, dx, n, n);	
	
	if(dbugFlag2) myStream<<"In process :"<<myRank<</*" tmpRank = "<<tmpRank<<*/" dx = "<<dx<<" n = "<<n<<endl;
	if(dbugFlag2) myStream<<"Process "<<myRank<<" : expecting serialize_CPPd buffer of size "<<dx*n*n<<" recieved from process0 "<<endl;	
		
	buf = new float[dx*n*n];	

	MPI_Recv((void*)buf, dx*n*n , MPI_FLOAT, 0, 2 , MPI_COMM_WORLD, &recvStatus);
	if(dbugFlag2) myStream<<"process "<<myRank<<" : serialized buffer of size "<<dx*n*n<<" recieved from process0 "<<endl;	

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

/* We are done with the input . Now Lets start MPI - Phase2 All Calculations */
if(myRank>=1 && myRank<=workerCount)
	{
MPI_Request send_req[2], recv_req[2];
MPI_Status stat;

float* histopLayer = new float[n*n];
float* mytopLayer = new float[n*n];
float* hisbotLayer = new float[n*n];
float* mybotLayer = new float[n*n];

/* Initialize tmpChunk */
initArray_CPP(tmpChunk, dx, n, n);

for(int t=0;t< tf;t++)
		{

/* copy Space */
for(int tx=0;tx<dx;tx++)
	for(int ty=0;ty<n;ty++)
		for(int tz=0;tz<n;tz++)
			tmpChunk[tx][ty][tz] = workChunk[tx][ty][tz];
	
	/* Perform Computations */
	for(int x=0;x<dx;x++)
		for(int y=0;y<n;y++)
			for(int z=0;z<n;z++)
				{	
						
				float tmp = c0*tmpChunk[x][y][z];
				
                                if(z<(n-1)) tmp+=(c1+c2+c3)*tmpChunk[x][y][z+1];
                                if(z>0) tmp+=(c1+c2+c3)*tmpChunk[x][y][z-1];


                                if(y<(n-1)) tmp+=(c1+c2+c3)*tmpChunk[x][y+1][z];
                                if(y>0) tmp+=(c1+c2+c3)*tmpChunk[x][y-1][z];

                                if(x<(dx-1)) tmp+=(c1+c2+c3)*tmpChunk[x+1][y][z];
                                if(x>0) tmp+=(c1+c2+c3)*tmpChunk[x-1][y][z];
				
                                workChunk[x][y][z] = tmp;			
				}
		
	/* Construct own topLayer and BottomLayer */
	for(int i=0;i<n;i++)
		for(int j=0;j<n;j++)
			{
			mytopLayer[i*n+j] = tmpChunk[dx-1][i][j];
			mybotLayer[i*n+j] = tmpChunk[0][i][j];
			}	
		
	/* Send and recieve top/bottom layers among themselves */
		if(myRank < workerCount) {
					MPI_Isend(mytopLayer, n*n, MPI_FLOAT, myRank+1 , 10 + 2 * t, MPI_COMM_WORLD , & send_req[ 0 ] );
					MPI_Irecv(hisbotLayer, n*n, MPI_FLOAT, myRank +1, 11 + 2 * t, MPI_COMM_WORLD , & recv_req[ 0 ] );
					}		
		if(myRank > 1)
			{			
				MPI_Isend(mybotLayer, n*n, MPI_FLOAT, myRank -1, 11 + 2 * t, MPI_COMM_WORLD , & send_req[ 1 ] );
				MPI_Irecv(histopLayer, n*n, MPI_FLOAT, myRank -1, 10 + 2 * t, MPI_COMM_WORLD , & recv_req[ 1 ] );
			}
		
		if(myRank < workerCount) MPI_Wait(&recv_req[0], &stat);
		if(myRank > 1)	MPI_Wait(&recv_req[1], &stat);
	
		if(myRank < workerCount) MPI_Wait(&send_req[0], &stat);
		if(myRank > 1)	MPI_Wait(&send_req[1], &stat);
		
	/* Do upate on topLayes/bottom layer by using info gained from neighbour */	
		for(int i=0;i<n;i++)
			for(int j=0;j<n;j++)
				{
				if(myRank > 1)
					workChunk[0][i][j]+= (c1+c2+c3)*(histopLayer[i*n+j]);
				
				if(myRank < workerCount)
					workChunk[dx-1][i][j]+= (c1+c2+c3)*(hisbotLayer[i*n+j]);
				}
		
	}
/* Deleye buffers */
delete[] mytopLayer;
delete[] histopLayer;
delete[] hisbotLayer;
delete[] mybotLayer;
	}

MPI_Barrier(MPI_COMM_WORLD);

/* Phase 3 - Assemble it back and print*/
if(myRank == 0)
	{
initArray_CPP(finalResult, n, n, n);

MPI_Status recvStatus;

float* buf = new float[dx*n*n];
for(int i=1;i<=workerCount;i++)
		{
		MPI_Recv((void*)buf, dx*n*n, MPI_FLOAT, i, 3,  MPI_COMM_WORLD, &recvStatus);
		/* deserialize_CPP the buffer */
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

if(dumpSelfStream)
	{
//cout<<myStream.str();
	}

if(myRank == 0)
	{
	cout<<n<<"X"<<n<<"X"<<n<<endl;
		for(int i=0;i<n;i++,cout<<endl)
			for(int j=0;j<n;j++,cout<<endl)
				{
				for(int k=0;k<n;k++)
					cout<<setw(10)<<fixed<<finalResult[i][j][k]<<' ';
				}
	}
return 0;
}
