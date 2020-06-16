#include <iostream>				// cout
#include <sys/time.h> 				// struct timeval, gettimeofday
#include <fstream> 				// Dosya islemleri icin kullanildi
#include <sstream>				// Stringleri parcalamak icin
#include <string>				// String islemleri icin
#include <boost/algorithm/string/replace.hpp> 	// replace fonksiyonu kullanmak icin
#include <vector> 				// 5*5 lik matrisi saklamada kullanilan vektor icin kullanildi.
#include <bits/stdc++.h> 			// vektorlerde siralama yapmak icin kullanildi.

using namespace std;
int FindMedian(int position, int nrow, int ncol, int *matrixPtr); // Ortanca bulan fonksiyon
string GetFileName(string name);				  // _filtered.txt isimli dosya yapan fonksiyon

int main(int argc, char* argv[]) {
	struct timeval currentTime;
	double startTime, endTime, elapsedTime;	// Zaman hesaplamak degiskenleri
	int number = 0;    	 		// Dosyadan deger okunurken kullanilan ara degisken
	int nrow = 0;				// Satir sayisini tutan degisken
	int ncol = 0;				// Sutun sayisini tutan degisken
	int *matrixPtr;				// Giris matrisi
	int *newMatrixPtr;			// Cikis matrisi

	ifstream file(argv[1]);				// Dosya olusturuldu
	if (file.is_open()){				// Dosya acildi.
		file>>nrow>>ncol;			// Dosyadaki ilk iki deger satir ve sutuna atildi.
		matrixPtr = new int[nrow*ncol];		// Giris matrisi olusturuldu.
		for(int i=0;i<nrow;i++)			// Dosyadaki degerler okunup matrise atildi.
			for(int j=0;j<ncol;j++)
			{
				file>>number;
				matrixPtr[(i*ncol)+j]=number;
			}
		file.close();					// Dosya kapatildi.
	}

	newMatrixPtr = new int[nrow*ncol];			// Cikis matrisi olusturuldu.
	int result;						// FindMedian fonksiyonundan donen degeri tutmak icin kullanilan degisken

	gettimeofday(&currentTime, NULL);
	startTime = currentTime.tv_sec + (currentTime.tv_usec/1000000.0);

	for(int i=0;i<nrow;i++)
		for(int j=0;j<ncol;j++)
		{	 //FindMedian fonksiyonundan donen deger Cikis Matrisine atildi.
			 result = FindMedian(((i*ncol)+j), nrow, ncol, matrixPtr);
			 newMatrixPtr[((i*ncol)+j)] = result;
		}

	gettimeofday(&currentTime, NULL);
	endTime = currentTime.tv_sec + (currentTime.tv_usec/1000000.0);
	elapsedTime = endTime - startTime;
	cout<<"Total Time Taken: "<<elapsedTime<<endl;


	string  fileName = GetFileName(argv[1]);		// Cikis dosyasi ismi olusturuldu.
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
	delete [] matrixPtr;						// Giris Matrisi silindi.
	delete [] newMatrixPtr;						// Cikis Matrisi silindi.
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
	int newPosition;							// Arguman olarak verilen konumum kaybetmemek icin tanimlandi.
	int value;								// array degiskenine atilacak deger icin tanimlandi.

	for(int i = -2; i<3; i++){						// Distaki for dongusu konumu ustteki iki satirdan alttaki iki
										//satira kadar goturuyor.
		newPosition = (position + (ncol*i));				// newPosition ile satirlar arasinda dolasiyoruz.
		for(int j=-2; j<3; j++){					// Icteki for dongusu satirin icinden 5 deger alinmasini sagliyor.
			value = matrixPtr[(newPosition + j)];			// Alinan deger value degiskenine atiliyor.
			array.push_back(value);					// Deger array vektorune atiliyor.
		}
	}

	sort(array.begin(), array.end());			// Vektor icindeki degerleri siralamak icin kullanilan built-in sort fonksiyonu
	return array[12];					// Siralama sonucundaki medyan donduruluyor.
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
        fileName = fileName + "_filtered.txt";			// _filtered.txt eklendi.
	return fileName;					// deger  return edildi.
}

