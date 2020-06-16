#include <mpi.h>
#include <iostream>
using namespace std;

long long KomsulukBul(long long); //Prototip

int main(int argc, char* argv[])
{
	int myRank, 
        size;
	
	long long *komsuPtr,
			  *maxPtr;
		
	long long altLimit, ustLimit,localMax, globalMax,globalKomsu, donenDeger, komsuDeger;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	// alt limit ile üst limit aldım
	if(myRank==0)
	{
		cout<<"alt limit?: ";
		cin>>altLimit;
		cout<<"ust limit?: ";
		cin>>ustLimit;
		
		komsuPtr=new long long [size];
		maxPtr=new long long [size];
	}
	
	// Diğer rutinlerin alt ve ust limiti bilmesini sağladım
	MPI_Bcast(&altLimit,1,MPI_LONG_LONG,0,MPI_COMM_WORLD);
	MPI_Bcast(&ustLimit,1,MPI_LONG_LONG,0,MPI_COMM_WORLD);
	
	localMax = 0;
	komsuDeger = 0;
	
	// local maksimum hesaplandı
	for(int i=(altLimit+myRank);i<=ustLimit;i+=size)
	{
		donenDeger = KomsulukBul((long long)i);
		if(donenDeger >= komsuDeger)
		{
			komsuDeger = donenDeger;
			localMax = (long long)i;
		}
			
	}
	
	
	MPI_Gather(&komsuDeger,1,MPI_LONG_LONG,komsuPtr,1,MPI_LONG_LONG,0,MPI_COMM_WORLD);
	MPI_Gather(&localMax,1,MPI_LONG_LONG,maxPtr,1,MPI_LONG_LONG,0,MPI_COMM_WORLD);
	
	// yazdırıldı
	if(myRank==0)
	{	
		globalMax = 0;
		globalKomsu = 0;
		
		for(int i=0; i<size; i++){
			cout<<komsuPtr[i]<<" : "<< maxPtr[i]<<endl;
		}
		
		for(int i =0; i<size; i++){
			
			if(komsuPtr[i] > globalKomsu )
			{
				globalKomsu = komsuPtr[i];
				globalMax = maxPtr[i];
			}
			
			if(komsuPtr[i] == globalKomsu )
			{
				if(maxPtr[i] > globalMax)
				{	
					globalKomsu = komsuPtr[i];
					globalMax = maxPtr[i];
				}
			}
		}
	
		cout<<"En büyük sayı: "<<globalMax<< " en büyük komsuluk: "<<globalKomsu<<endl;
	}
		
	MPI_Finalize();
	
	return 0;
}

long long KomsulukBul(long long num)
{
	int index=0, toplam = 0;
	int a;
	long long temp;
	
	temp = num;
	
	// tekrar kontrolü için oluşturuldu
	int  dizi[10] = {0,0,0,0,0,0,0,0,0,0};
	int kontrol[10] ={false,false,false,false,false,false,false,false,false,false};
	
	while(temp > 0)
	{
		a = temp%10;
		
		if(kontrol[a] == true)
			return -1;
		else kontrol[a] = true;
		
		dizi[index]= a;
	
		index++;
		temp/=10;
	}
	
	for(int i=0; i<index-1; i++)
	{
		toplam += (dizi[i]*dizi[i+1]);
	}
	
	return toplam;
}
