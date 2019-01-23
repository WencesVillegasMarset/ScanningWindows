//#define CERES_FOUND true
#include "Keypoints.h"
#include "patchClass.h"
#include "clasificacionExacta.h"

#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
using namespace std;
using namespace cv;


int getdir(const string _filename, std::vector<string> &files)
{
  std::ifstream myfile(_filename.c_str());
  if (!myfile.is_open()) {
    cout << "Unable to read file: " << _filename << endl;
    exit(0);
  } else {
    size_t found = _filename.find_last_of("/\\");
    string line_str, path_to_file = _filename.substr(0, found);
    while ( getline(myfile, line_str) )
      files.push_back(path_to_file+string("/")+line_str);
  }
  return 1;
}

bool clasificador(Mat desc){
	srand(desc.rows);
	int num=rand()%100;
	if(num<50)return false; else return true;
}


int main(int argc, char** argv)
{
	// muestra la ayuda
	if (argc !=11 ) {
		cout
		//dezpl es un float [0,1]
		//ver porcentajeOverlap
		<< " Uso: ./ScanningWindows <rutaArchivoImagenes.txt> <anchoPatch> <altoPatch> <desplazamiento> <anchoImagen> <altoImagen> <rutaSalida> <patchArchivoR>"
				"<porcentajeOverlap> <rutaYemas>\n"
		<< "Ejemplo: ./ScanningWindows ../img.txt 300 300 0.5 720 1280 ../salida "
		<< endl;
		return 0;
		// patchArchivoR es la ruta al archivo patchClassR
		//porcentajeOverlap proporcion minima entre yema y fondo para ser considerado 1 al patch
		//rutaYemas es el path al csv que describe el corpus (patches)
		//rutaArchivoImagenes.txt se pone en la misma carpeta que las imagenes
		//rutasalida carpeta para outputs
		/*
			./ScanningWindows ./src/images/images/full_images.txt 300 300 0.5 2000 2000 ./output ./src/patchClass.R 0.5
		*/
	}

	char* rutaTxt = argv[1];
	int xWindow, yWindow, despX, despY;
	xWindow = atoi(argv[2]);
	yWindow = atoi(argv[3]);
	despX = (int)(atof(argv[4])*xWindow);
	despY = (int)(atof(argv[4])*yWindow);
	int ancho = atoi(argv[5]);
	int alto = atoi(argv[6]);
	char* rutaOut = argv[7];
	char* rFile = argv[8];
	std::vector<Point2d> inicioPatches; //ontiente coordenadas 2D de la esq sup izq de todos los patches
	//establece la esquina superior izquierda de los Patches
	for(int j=0;j<alto/despY;j++){
		for(int i=0;i<ancho/despX;i++){
			int startX = i*despX + 500;
			int startY = j*despY + 500;
			int endX = startX + xWindow;
			int endY = startY + yWindow;

			if(endX >= ancho) {endX=ancho-1; startX = endX-xWindow;}
			if(endY >= alto) {endY=alto-1; startY = endY-yWindow;}

			Point2d inicioPatch = Point2d(startX,startY);
			inicioPatches.push_back(inicioPatch);
		}
	}
	//Lee las imágenes y detecta sus keypoints
	vector<string> images_paths;
	getdir( rutaTxt, images_paths ); // el txt tiene que estar en la misma carpeta que las imagenes

	Ptr<SIFT> edetector = xfeatures2d::SIFT::create(); //objeto que maneja sift
	vector<Mat> descriptorImages;
	vector<Mat> keypointsCount;
	for (int i=0; i < images_paths.size(); ++i) {
		Mat im_cv = cv::imread(images_paths[i], IMREAD_GRAYSCALE);
		vector<KeyPoint> keypointsImg;
		edetector -> detect( im_cv, keypointsImg ); //busca keypoints en la imagen leida con sift
		Mat descriptores; //keypoints x 128 para cada imagen (una matriz -> una imagen)
		keypointsCount.push_back(descriptores); // numImagenes x keypoints(variable) x 128
		edetector->compute(im_cv, keypointsImg, descriptores); //calcula descriptores y los guarda en el ultimo argumento
		guardarCoordenadas(descriptores,keypointsImg);
		descriptorImages.push_back(descriptores); //
	}
	//arma los descriptores de cada patch y clasifica
	vector<char*> filasCsvR,filasCsvH;
	int idPatch = 0;

	for (int image = 0;image<descriptorImages.size();image++) {
		bool clasificacionR = false; // SVM
		bool clasificacionH = false; // ground truth
		Mat coor = descriptorImages[image].colRange(0,2).clone(); // saca las xy de los kp
		Mat desc = descriptorImages[image].colRange(8, descriptorImages[image].cols).clone(); // 128 (descriptores)
		for(int i =0; i<inicioPatches.size(); i++){
			Mat descriptoresPatch; //guarda los descriptores que estan dentro de este patch
			Mat coordenadasPatch; // coordenadas de los descr que estan dentro de este patch
			for(int j=0; j<descriptorImages[image].rows;j++){ //filtra los keypoints y se queda los que estan en este patch
				if(descriptorImages[image].at<float>(j,0)>inicioPatches[i].x && descriptorImages[image].at<float>(j,0)<inicioPatches[i].x+xWindow &&
						descriptorImages[image].at<float>(j,1)>inicioPatches[i].y && descriptorImages[image].at<float>(j,1)<inicioPatches[i].y+yWindow){

					descriptoresPatch.push_back(desc.row(j));
					coordenadasPatch.push_back(coor.row(j));
				}
			}
			char* rutaDescriptor = new char[500];
			sprintf(rutaDescriptor,"%sdescriptores/descriptorImg%dPatch%d-%d.csv", rutaOut,image,(int)inicioPatches[i].x,(int)inicioPatches[i].y);
			/*
			DIR *dirp; //crea el directorio (commented out on first encounter)
			if( (dirp= opendir(rutaOut)) == NULL ){
				char* comando = new char[510];
				sprintf(comando, "mkdir %sdescriptores", rutaOut);
				std::system(comando);
			}*/

			size_t found = images_paths[image].find_last_of("/\\"); // se queda 
			string nombreImg = images_paths[image].substr(found+1,string::npos); //se queda con el nombre de la foto s/path
			guardarDescriptor(descriptoresPatch,rutaDescriptor); //persiste el descriptor en un csv
				clasificacionR = test(descriptoresPatch,rutaOut,rFile);
				clasificacionH = clasificar(inicioPatches[i],Point2d(xWindow,yWindow),argv[10],nombreImg,atof(argv[9]));

				cout<<"la clasificacion humana dio: "<<clasificacionH<<endl;
			if (clasificacionR){
				for( int j=0; j<coordenadasPatch.rows;j++){
						keypointsCount[image].push_back(coordenadasPatch.row(j)); //acumula las coordenadas de los keypoints del patch
				}
			}
			char* filaR = new char[500];
			sprintf(filaR,"%d,%d,%f,%f,%d,%d,%s,%s",idPatch,image,inicioPatches[i].x,inicioPatches[i].y,xWindow,yWindow,clasificacionR? "true" : "false",nombreImg.c_str());
			filasCsvR.push_back(filaR);
			char* filaH = new char[500];
			sprintf(filaH,"%d,%d,%f,%f,%d,%d,%s,%s",idPatch,image,inicioPatches[i].x,inicioPatches[i].y,xWindow,yWindow,clasificacionH? "true" : "false",nombreImg.c_str());
			filasCsvH.push_back(filaH);
			//guarda en filasCSVh los resultados de clasificacion H y R
			idPatch++;
		}
		size_t found2 = string(rutaOut).find_last_of("/\\");
		string path_to_file = string(rutaOut).substr(0, found2);
		char* rutaCountsKp = new char[300];
		sprintf(rutaCountsKp, "%s/countImg%d.csv", rutaOut,image);
		std::ofstream fs(rutaCountsKp);
		char* encabezado = new char[100];
		sprintf(encabezado,"xPunto,yPunto,count");
		fs << encabezado;
		fs << "\n";
		fs << format(keypointsCount[image], Formatter::FMT_CSV) << endl;
		fs.close();
		//persiste las coordenadas de los keypoints para ese patch
	}

	//guarda el csv con la clasificación de cada patch
	char* rutaScanningWindowR = new char[300];
	sprintf(rutaScanningWindowR, "%s/scanningWindowR.csv", rutaOut);
	std::ofstream fs(rutaScanningWindowR);
	char* encabezado = new char[500];
	sprintf(encabezado,"idPatch,n°Imagen,inicioPatch.x,inicioPatch.y,anchoPatch,altoPatch,clasificación,nombreImg\n");
	fs << encabezado;
	for(int i=0;i<filasCsvR.size();i++){
		fs << filasCsvR[i];
		fs << "\n";
	}
	fs.close();

	char* rutaScanningWindowH = new char[300];
	sprintf(rutaScanningWindowH, "%s/scanningWindowH.csv", rutaOut);
	std::ofstream fs2(rutaScanningWindowH);
	fs2 << encabezado;
	for(int i=0;i<filasCsvH.size();i++){
		fs2 << filasCsvH[i];
		fs2 << "\n";
	}
	fs2.close();

	return 0;
}


