////////////////////////////////////////////////////////////////////////
///////// ERGÜN ACUN 170707016 BILGISAYAR MUHENDISLIGI  ////////////////
///////// PARALEL PROGRAMLAMAYA GIRIS BM308 BAHAR 2020  ////////////////
/////////    PROJE 2 DR. OGR. UYESI DENIZ DAL           ////////////////
////////////////////////////////////////////////////////////////////////

#include <mpi.h>		// MPI kutuphanesi
#include <iostream>		// cout 
#include <fstream> 		// Dosya islemleri için kullanildi
#include <sstream>		// Stringleri parçalamak icin
#include <string>		// String islemleri icin
#include <boost/algorithm/string/replace.hpp> // replace fonksiyonu kullanmak icin
#include <vector> 		// 5*5 lik matrisi saklamada kullanilan vektor icin kullanildi.
#include <bits/stdc++.h> // vektorlerde siralama yapmak icin kullanildi.
using namespace std;
#define TAG 25
#define TAG1 26

int FindMedian(int position, int nrow, int ncol, int *matrixPtr); // Ortanca bulan fonksiyon
string GetFileName(string name);				// Çikis matrisi ismini olusturur.

int main(int argc, char* argv[]){
	
	int myRank,						// Degisken tanimlamalari
		size,
		nrow,
		ncol,
		newNrow,
		number,
		partialSize,
		totalPartialSize,
		remainderSize,
		totalRemainderSize;
		
	int *matrixPtr,				
		*partialMatrixPtr,
		*newPartialMatrixPtr,
		*newMatrixPtr;
	
	double startTime, endTime, elapsedTime;		// Zaman hesaplama 
	
	MPI_Status status;
	
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&myRank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	
	if(myRank==0){
		
		ifstream file(argv[1]);				// Dosya olusturuldu
		if (file.is_open()){				// Dosya acildi.
			file>>nrow>>ncol;				// Dosyadaki ilk iki deger satir ve sutuna atildi.
			matrixPtr = new int[nrow*ncol];	// Giris matrisi olusturuldu.
			newMatrixPtr = new int[nrow*ncol];
			for(int i=0;i<nrow;i++)			// Dosyadaki degerler okunup matrise atildi.
				for(int j=0;j<ncol;j++)
					{
						file>>number;
						matrixPtr[(i*ncol)+j]=number;
					}
			file.close();					// Dosya kapatildi.
		}
		
		startTime=MPI_Wtime();
		
		partialSize = (nrow/size)*ncol;                       // Makinalara paylasim 
		totalPartialSize = partialSize + 4*ncol;				// ilk iki ve son iki satir icin ayarlama
		remainderSize = (nrow*ncol) - partialSize*(size-1);		// son makinaya verilecek paylasim
		totalRemainderSize = remainderSize + 2*ncol;			// son makinaya verilen paylasimin ilk iki satiri icin ayarlama
		
		// Ara makinalara paylasimin boyutu gönderildi
		for(int i=1;i<(size-1);i++)	
			MPI_Send(&totalPartialSize,1,MPI_INT,i,TAG,MPI_COMM_WORLD);
		
		// Son makinaya paylasimin boyutu gönderildi
		MPI_Send(&totalRemainderSize,1,MPI_INT,(size-1),TAG1,MPI_COMM_WORLD);
		
		// Ara makinalara paylasim yapildi
		for(int i=1;i<(size-1);i++)
			MPI_Send(&matrixPtr[(i*partialSize)-(2*ncol)],totalPartialSize,MPI_INT,i,TAG,MPI_COMM_WORLD);
		
		// Son makinaya paylasim yapildi
		MPI_Send(&matrixPtr[(partialSize*(size-1))-(2*ncol)],totalRemainderSize,MPI_INT,(size-1),TAG1,MPI_COMM_WORLD);
		
		// Tum makinalara satir ve sutun bilgisi gönderildi
		for(int i=1;i<size;i++){
			MPI_Send(&nrow,1,MPI_INT,i,TAG,MPI_COMM_WORLD);
			MPI_Send(&ncol,1,MPI_INT,i,TAG,MPI_COMM_WORLD);
		}
		
		// master makina icin boyut ayarlamasi
		int rowForMaster = (nrow/size)+2;
		
		//Master makina filtreleme islemi
		for(int i=0;i<rowForMaster;i++)
			for(int j=0;j<ncol;j++)
			{	 
				 int result = FindMedian(((i*ncol)+j), rowForMaster, ncol, matrixPtr);
				 newMatrixPtr[((i*ncol)+j)] = result;
			}
		
		// Ara makinalardan filtrelenmis veriler toplandi ve birlestirildi
		for(int i=1;i<(size-1);i++)
			MPI_Recv(&newMatrixPtr[(i*partialSize)],(totalPartialSize-4*ncol),MPI_INT,i,TAG,MPI_COMM_WORLD,&status);
		
		// Son makinadaki filtrelenmis veriler de diger filtrelenmis verilere eklendi
		MPI_Recv(&newMatrixPtr[((size-1)*partialSize)],(totalRemainderSize-2*ncol),MPI_INT,(size-1),TAG,MPI_COMM_WORLD,&status);
		
		string  fileName = GetFileName(argv[1]);	// Cikis dosyasi ismi olusturuldu.
		ofstream newFile(fileName);				// Cikis matrisindeki degerler dosyaya yazdirildi.
		for(int i=0;i<nrow;i++)					
			for(int j=0;j<ncol;j++)
			{
				if ((j%ncol)== 0){
					newFile<<endl;
				}
			newFile<<newMatrixPtr[(i*ncol)+j]<<"	";
			}
		newFile.close();
		
		endTime=MPI_Wtime();//Kronometreyi Durdur
		elapsedTime=endTime-startTime; // Zaman farki bulundu ve ekrana basildi
		
		cout<<"\nGecen Sure (Saniye Cinsinden): "<<elapsedTime<<endl;
		
		delete [] matrixPtr, partialMatrixPtr, newPartialMatrixPtr, newMatrixPtr;  // Silme islemi
	}
	
	else{
		
		// Son makina icin islemler
		if(myRank==(size-1)){
			
			// Paylasimin boyutu alindi
			MPI_Recv(&totalRemainderSize,1,MPI_INT,0,TAG1,MPI_COMM_WORLD,&status);
			
			// giris ve cikis matrisleri tanimlandi
			partialMatrixPtr = new int[totalRemainderSize];
			newPartialMatrixPtr = new int[totalRemainderSize];
			
			// Paylasim giris matrisine alindi
			MPI_Recv(partialMatrixPtr,totalRemainderSize,MPI_INT,0,TAG1,MPI_COMM_WORLD,&status);
			
			// Boyut ve sutun bilgisi alindi
			MPI_Recv(&nrow,1,MPI_INT,0,TAG,MPI_COMM_WORLD,&status);
			MPI_Recv(&ncol,1,MPI_INT,0,TAG,MPI_COMM_WORLD,&status);
			
			// islem görecek satir sayisi belirlendi
			int newNrow = (nrow+2)-(nrow/size)*(size-1);
			
			// filtreleme islemi yapilip cikis matrisine aktarildi
			for(int i=0;i<newNrow;i++)
				for(int j=0;j<ncol;j++)
				{	 
			 		int result = FindMedian(((i*ncol)+j), newNrow, ncol, partialMatrixPtr);
			 		newPartialMatrixPtr[((i*ncol)+j)] = result;
				}
			
			// Cikis matrisi masteri makinaya gonderildi	
			MPI_Send(&newPartialMatrixPtr[2*ncol],(totalRemainderSize-2*ncol),MPI_INT,0,TAG,MPI_COMM_WORLD);
			
			// Silme islemi
			delete [] partialMatrixPtr;
			delete [] newPartialMatrixPtr;
		}
		
		// Ara makinalar icin islemler
		else{
			// Paylasimin boyutu alindi
			MPI_Recv(&totalPartialSize,1,MPI_INT,0,TAG,MPI_COMM_WORLD,&status);
			
			// giris ve cikis matrisleri tanimlandi
			partialMatrixPtr = new int[totalPartialSize];
			newPartialMatrixPtr = new int[totalPartialSize];
		
			// Paylasim giris matrisine alindi
			MPI_Recv(partialMatrixPtr,totalPartialSize,MPI_INT,0,TAG,MPI_COMM_WORLD,&status);
			
			// Boyut ve sutun bilgisi alindi
			MPI_Recv(&nrow,1,MPI_INT,0,TAG,MPI_COMM_WORLD,&status);
			MPI_Recv(&ncol,1,MPI_INT,0,TAG,MPI_COMM_WORLD,&status);
			
			// islem görecek satir sayisi belirlendi
			int newNrow = (nrow/size)+4;
			
			// filtreleme islemi yapilip cikis matrisine aktarildi 
			for(int i=0;i<newNrow;i++)
				for(int j=0;j<ncol;j++)
				{	 
			 		int result = FindMedian(((i*ncol)+j), newNrow, ncol, partialMatrixPtr);
			 		newPartialMatrixPtr[((i*ncol)+j)] = result;
				}
			
			// Cikis matrisi masteri makinaya gonderildi
			MPI_Send(&newPartialMatrixPtr[2*ncol],(totalPartialSize-4*ncol),MPI_INT,0,TAG,MPI_COMM_WORLD);
			
			// Silme islemi
			delete [] partialMatrixPtr;
			delete [] newPartialMatrixPtr;
		}
		
	}
	
	MPI_Finalize();	
	return 0;	
} 

// Fonksiyon arguman olarak, Giris Matrisindeki degerlerin konumlarini, satir ve sutun sayisini ve Giris Matrisini alir.
int FindMedian(int position, int nrow, int ncol, int *matrixPtr){

	// Arguman olarak verilen konumun ilk iki sutun ya da son iki sutunda olup olmadigina bakiliyor.
	int forTwoCol = position % ncol;
	if ((forTwoCol < 2) || (forTwoCol > (ncol-3)) ) return matrixPtr[position];

	// Arguman olarak verilen konumun ilk iki satirda olup olmadigina bakiliyor.
	int firstTwoRow = position + (-2 * ncol);
	if (firstTwoRow < 0) return matrixPtr[position];

	// Arguman olarak verilen konumun son iki satirda olup olmadigina bakiliyor.
	int lastTwoRow = position + (2 * ncol);
	if (lastTwoRow > (ncol*nrow)) return matrixPtr[position];

	vector<int> array (0);							// 5*5 lik matrisi tutmak icin tanimlandi
	int newPosition;								// Arguman olarak verilen konumum kaybetmemek icin tanimlandi.
	int value;										// array degiskenine atilacak deger icin tanimlandi.

	for(int i = -2; i<3; i++){						// Distaki for dongusu konumu ustteki iki satirdan alttaki ikisatira kadar goturuyor.
		newPosition = (position + (ncol*i));				// newPosition ile satirlar arasinda dolasiyoruz.
		for(int j=-2; j<3; j++){					// Icteki for dongusu satirin icinden 5 deger alinmasini sagliyor.
			value = matrixPtr[(newPosition + j)];			// Alinan deger value degiskenine atiliyor.
			array.push_back(value);					// Deger array vektorune atiliyor.
		}
	}

	sort(array.begin(), array.end());				// Vektor icindeki degerleri siralamak icin kullanilan built-in sort fonksiyonu
	return array[12];								// Siralama sonucundaki medyan donduruluyor.
}


string GetFileName(string name){
	string path1 = name;					// char* sorun cikarmamasi icin string degiskene atildi.
    boost::replace_all(path1, "/"," ");			// '/' karakteri yerine bosluk karakteri konuldu. Cunku sstream bosluga duyarli
    vector<string> path;					// vektor degiskeni olusturuldu
    stringstream ss(path1);					// path1 degiskeni streamstring ile bosluklara gore parcalandi
    string temp;
    while(ss>>temp)						// parcalanma ile olusan stringler vektore atildi.
        path.push_back(temp);
    string fileName = path[2];				// vektorun ikinci indisindeki  GoruntuMatrisi[1-11].txt var
    fileName.erase(fileName.end()-4, fileName.end());	// .txt karakterlerini cikarildi.
    fileName = fileName + "_filtered2.txt";			// _filtered.txt eklendi.
	return fileName;					// deger  return edildi.
}

