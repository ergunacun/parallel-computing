////////////////////////////////////////////////////////////////////////
///////// ERGÜN ACUN 170707016 BILGISAYAR MUHENDISLIGI  ////////////////
///////// PARALEL PROGRAMLAMAYA GIRIS BM308 BAHAR 2020  ////////////////
/////////    PROJE 3 DR. OGR. UYESI DENIZ DAL           ////////////////
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

int FindMedian(int position, int nrow, int ncol, int *matrixPtr); // Ortanca bulan fonksiyon
string GetFileName(string name);								  // Çikis matrisi ismini olusturur.

int main(int argc, char* argv[])
{
	int myRank,      // rank
        size,        // process sayısı
		partialSize, // kısmi matris boyutu
		newNrow,	 // kısmi matris sayir sayısı 
		remainder, 	 // kalan
		nrow,		// satir sayisini
		ncol,		// sütun sayisi
		rem,
		number;  

	int *partialMatrixPtr,		// Kısmi Giriş matris
		*newPartialMatrixPtr,	// Kısmi Çıkış matris
		*matrixPtr,				// Giriş matrisi 
		*newMatrixPtr;			// Çıkış matrisi

	int *sendcounts,    // scatterv fonksiyonu icin tanımlamalar
		*rcvcounts,
		*displs;
	
	double startTime, endTime, elapsedTime;		// Zaman hesaplama 
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&myRank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);

	if(myRank == 0)
	{	
		ifstream file(argv[1]);					// Dosya olusturuldu
		if (file.is_open())						// Dosya acildi.
		{				
			file>>nrow>>ncol;					// Dosyadaki ilk iki deger satir ve sutuna atildi.
			matrixPtr = new int[nrow*ncol];		// Giris matrisi olusturuldu.
			for(int i=0;i<nrow;i++)				// Dosyadaki degerler okunup matrise atildi.
				for(int j=0;j<ncol;j++)
					{
						file>>number;
						matrixPtr[(i*ncol)+j]=number;
					}
			
			file.close();					// Dosya kapatildi.
		}
		
		
		startTime=MPI_Wtime();			// süre başlatildi.
		
		sendcounts = new int[size]; 	// scatterv ve gatherv fonksiyonları icin boyutlar belirlendi.
		displs = new int[size];
		rcvcounts = new int[size]; 
			
		newNrow = nrow/size;			// kısmi boyut bulundu
		partialSize = newNrow*ncol;
		
		rem = nrow%size;				// kalan bulundu
		int sum = 4*ncol;
		
		// kalan degere gore scatterv ve gatherv icin matrislerin ici dolduruldu.
		for (int i = 0; i < size; i++) 
		{
			sendcounts[i] = partialSize + 2*ncol;
			if (rem > 0) 
			{
				sendcounts[i]= sendcounts[i] + ncol ;
				rem = rem -1;
			}
			
			sum = sum-4*ncol;
			displs[i] = sum;
			sum = sum + sendcounts[i];
		}
		
		//cikis matrisi boyutlandirildi.
		newMatrixPtr = new int[nrow*ncol];
		remainder= sum/ncol;
		
		for(int i=0;i<size;i++)
			rcvcounts[i]=(partialSize+3*ncol);
	}	
	
	// kismi boyut, sutun sayisi ve kalan broadcast yapildi.
	MPI_Bcast(&partialSize,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&ncol,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&rem,1,MPI_INT,0,MPI_COMM_WORLD);
	
	// kalan degere gore alici taraftaki kismi matris boyutu ayarlandi
	if(myRank<rem)
	{
		partialSize = partialSize+3*ncol;
		partialMatrixPtr = new int[partialSize];
		newPartialMatrixPtr = new int[partialSize];	
	}	
	else
	{
		partialSize = partialSize+2*ncol;
		partialMatrixPtr = new int[partialSize];
		newPartialMatrixPtr = new int[partialSize];	
	}
	
	// makinalara dagitim yapildi.
	MPI_Scatterv(matrixPtr, sendcounts, displs, MPI_INT, partialMatrixPtr, partialSize, MPI_INT, 0, MPI_COMM_WORLD);
	
	// kismi matrisler icin yeni satir sayisi broadcast yapildi.
	MPI_Bcast(&newNrow,1,MPI_INT,0,MPI_COMM_WORLD);
	
	// kalan degere göre olusan farklı satir sayilari degerlendirilerek filtreleme islemi yapildi.
	if(myRank<rem)
	{
		for(int i=0;i<(newNrow+3);i++)
			for(int j=0;j<ncol;j++)
			{	 
				 int result = FindMedian(((i*ncol)+j), (newNrow+3), ncol, partialMatrixPtr);
				 newPartialMatrixPtr[((i*ncol)+j)] = result;
			}
	}
	
	else
	{
		for(int i=0;i<(newNrow+2);i++)
			for(int j=0;j<ncol;j++)
			{	 
				 int result = FindMedian(((i*ncol)+j), (newNrow+2), ncol, partialMatrixPtr);
				 newPartialMatrixPtr[((i*ncol)+j)] = result;
			}
	}
	
	// filtrelenen degerler toplandi.
	MPI_Gatherv(newPartialMatrixPtr, partialSize, MPI_INT, newMatrixPtr, sendcounts, displs, MPI_INT,0, MPI_COMM_WORLD);
	
	// matrisler silindi
	delete [] partialMatrixPtr;
	delete [] newPartialMatrixPtr;

	// dosyaya yazdirma islemi yapildi.
	if(myRank == 0)
	{	
		string  fileName = GetFileName(argv[1]);	// Cikis dosyasi ismi olusturuldu.
		ofstream newFile(fileName);					// Cikis matrisindeki degerler dosyaya yazdirildi.
		for(int i=0;i<remainder;i++)					
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
		
		//matrisler silindi.
		delete [] rcvcounts;
		delete [] sendcounts;
		delete [] displs;
		delete [] matrixPtr;
		delete [] newMatrixPtr;
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
    fileName = fileName + "_filtered3.txt";			// _filtered.txt eklendi.
	return fileName;					// deger  return edildi.
}
