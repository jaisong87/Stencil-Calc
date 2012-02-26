#include<iostream>
#include<iomanip>
using namespace std;

int main()
{


int seed = 131 , step = 27 , mod = 897;
int q;
float c0 = 1, c1= 1, c2= 1, c3=1;

int N, Ft;
cin>>N>>Ft;
cin>>c0>>c1>>c2>>c3;

q = (1<<N);
cout<<N<<" "<<Ft<<endl;
cout<<c0<<" "<<c1<<" "<<c2<<" "<<c3<<endl;

for(int i=0;i<q;i++)
	for(int j=0;j<q;j++,cout<<endl)
		for(int k=0;k<q;k++)
			{
			 	seed+=step;
				seed%=mod;
				cout<<setw(6)<<float(seed)/10;
			}
return 0;
}
