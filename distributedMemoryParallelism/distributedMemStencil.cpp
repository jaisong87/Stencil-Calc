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
bool enableDebug = false; /* detailed debug flag - dump prints to myStream */
bool dbugFlag2 = false; /* basic debug flag - dump prints to myStream */
bool dumpSelfStream = false; /* Set this to see dbug prints from every worker */

int rank;
stringstream myStream; /* Stream of one worker,  */
//#define myStream cout

/* Just an Init-Routine - Performs malloc  
 */
bool initArray(float ***& threeDimSpace, int xD, int yD, int zD)
{
	if(enableDebug)
		myStream<<"Creating "<<xD<<"X"<<yD<<"X"<<zD<<" space for stencil computations"<<endl;

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
		myStream<<"Freeing "<<xD<<"X"<<yD<<"X"<<zD<<" space for stencil computations"<<endl;

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

/* print the serial buffer for debug purposes */
void printSerialBuffer(float *buf, int len)
{
stringstream ss1;
ss1<<"SerialStream : ";
for(int i=0;i<len;i++)
	ss1<<buf[i]<<' ';
myStream<<ss1.str()<<endl;
return;
}

/* print the 3-D space */
void printResult(float *** threeDimSpace, int xD, int yD, int zD)
{
stringstream ss1;
ss1<<"3D Grid ";
for(int x=0;x<xD;x++,ss1<<endl)
	for(int y=0;y<yD;y++,ss1<<endl)
		for(int z=0;z<zD;z++)
			{						
				ss1<<setw(10)<<fixed<<threeDimSpace[x][y][z]<<' ';			
			}
myStream<<ss1.str();
return;
}

/* Copy original Space to a buffer 
 * There is no check for mallocs, frees, wild-ptrs etc
 * Use all the functions carefully
 */
void copySpace(float ***& src, float***& dst, int xD, int yD, int zD)
{
	if(enableDebug)
		myStream<<"Copying from "<<xD<<"X"<<yD<<"X"<<zD<<" original space for stencil computations"<<endl;

for(int x=0;x<xD;x++)
	for(int y=0;y<yD;y++)
		for(int z=0;z<zD;z++)
			dst[x][y][z] = src[x][y][z];
return;
}

/*Perform serial-stenicl Compuatations */
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
return;
}

/* serialize the buffer */
void serialize(float*& tmp,float*** origSpace, int dx, int dy, int dz)
{
int ctr= 0;
for(int i=0;i<dx;i++)
	for(int j=0;j<dy;j++)
		for(int k=0;k<dz;k++)
			{
				tmp[ctr] = origSpace[i][j][k];
				ctr++;			
			}
return;
}

/* deserialize back the buffer */
bool deserializeBuffer(float***& mySpace, float* buf, int dx, int dy, int dz)
{
	int ctr = 0;
	for(int i=0;i<dx;i++)
		for(int j=0;j<dy;j++)
			for(int k=0;k<dz;k++)
				{
					mySpace[i][j][k] = buf[ctr];
					ctr++;
				}
return true;
}

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
		initArray(threeDimSpace, n, n, n );    /* Initialize the array */

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
			serialize(buf, &threeDimSpace[(w-1)*dx], dx, n, n);
			printSerialBuffer( buf , dx*n*n);	

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
	initArray(workChunk, dx, n, n);	
	
	if(dbugFlag2) myStream<<"In process :"<<myRank<</*" tmpRank = "<<tmpRank<<*/" dx = "<<dx<<" n = "<<n<<endl;
	if(dbugFlag2) myStream<<"Process "<<myRank<<" : expecting serialized buffer of size "<<dx*n*n<<" recieved from process0 "<<endl;	
		
	buf = new float[dx*n*n];	

	MPI_Recv((void*)buf, dx*n*n , MPI_FLOAT, 0, 2 , MPI_COMM_WORLD, &recvStatus);
	if(dbugFlag2) myStream<<"process "<<myRank<<" : serialized buffer of size "<<dx*n*n<<" recieved from process0 "<<endl;	

	deserializeBuffer(workChunk, buf, dx, n, n);
	
	printResult(workChunk, dx, n, n);	

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
initArray(tmpChunk, dx, n, n);

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
initArray(finalResult, n, n, n);

MPI_Status recvStatus;

float* buf = new float[dx*n*n];
for(int i=1;i<=workerCount;i++)
		{
		MPI_Recv((void*)buf, dx*n*n, MPI_FLOAT, i, 3,  MPI_COMM_WORLD, &recvStatus);
		/* deserialize the buffer */
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
	printResult(finalResult, n, n, n);
	}
else if(myRank>=1 && myRank<=workerCount)
	{
	float* buf = new float[dx*n*n];	
	serialize(buf, workChunk, dx, n, n);	
	MPI_Send((void*)buf, dx*n*n, MPI_FLOAT, 0, 3, MPI_COMM_WORLD);
	}

MPI_Barrier(MPI_COMM_WORLD);
MPI_Finalize();

if(dumpSelfStream)
	{
cout<<myStream.str();
	}

if(myRank == 0)
	{
		for(int i=0;i<n;i++,cout<<endl)
			for(int j=0;j<n;j++,cout<<endl)
				for(int k=0;k<n;k++)
					cout<<setw(10)<<fixed<<finalResult[i][j][k]<<' ';
	}
return 0;
}
