#include <opencv/cv.h>
#include <opencv2/highgui.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/ml.hpp>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <limits>
#include <stdexcept>
#include <stdio.h>
#include <string>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "patchClass.h"

using namespace boost::filesystem;
using namespace boost;
using namespace cv;
using namespace std;
using namespace xfeatures2d;



std::string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

/**
 * Carga el diccionario BoF desde el archivo YML.
 */
void readBoFDictionary(string outputDict, int dictSize, Mat& dictionary) {
	//prepare BOW descriptor extractor from the dictionary
	//to store the input file names
	char * filedic = new char[100];
	//create the file name of an image
	sprintf(filedic, "%sdict-s%d.yml", outputDict.c_str(), dictSize);
	FileStorage fs(filedic, FileStorage::READ);
	fs["vocabulary"] >> dictionary;
	fs.release();
}

/**
 * Clasifica los patches según sus SIFT descriptors
 */
bool patchClass(Mat SIFTdescriptors, string modelsPath, int dictSize, int iter, char* salida, char* rFile) {
//llamado por patch 
	if (SIFTdescriptors.empty())
		return false;//si no tiene keypoints sale

	cout<< "en este patch cayeron "<<SIFTdescriptors.rows<<" keypoints \n";
	//To store the dictionary
	Mat dictionary;
	//Read the stored dictionary
	readBoFDictionary(modelsPath, dictSize, dictionary); //

	//To store the BoF representation of the image
	Mat bowDescriptor;	
	//create a nearest neighbor matcher and SIFT feature descriptor
	Ptr<DescriptorMatcher> matcher(new FlannBasedMatcher); //flannmatcher obj para 
	cv::Ptr<xfeatures2d::SIFT> detector = xfeatures2d::SIFT::create(); 

	//create BoF descriptor extractor
	BOWImgDescriptorExtractor bowDE(detector, matcher); // extractor de descr bow
	//Set the dictionary with the vocabulary we created in the first step
	bowDE.setVocabulary(dictionary);
	//extract BoF descriptor from given image
	bowDE.compute(SIFTdescriptors, bowDescriptor); //le pasa los sift descr de este patch y le devuelve el descriptor bow

	bool pred = false;
	//BoF descriptors classification
	if (!(bowDescriptor.empty())) {	
		//To store the descriptor file name
		char * filecsv = new char[200];
		//the descriptor file with the location.
		sprintf(filecsv, "%s/hist-s25-it1.csv",salida); // persiste en un csv los descriptores bow
		//open the file to write the resultant descriptor
		std::ofstream fs1(filecsv);
		//image's info
		fs1 << format(bowDescriptor, Formatter::FMT_CSV)<< endl;
		fs1.close();

		char* r_cmd = new char[10000];
		sprintf(r_cmd, "R -q -e  \"source('%s');patchClassifier('%s', '%s', %d, %d)\"",rFile,
						modelsPath.c_str(), filecsv, dictSize, iter); //console command
		const char* com = r_cmd;
		string result = exec(com);
		cout << result<<endl; //muestra la clasificacion resultante para este patch
		size_t found = result.find_last_of("]");
		result = result.substr(found+2,string::npos);
		(atof(result.c_str())>0.5)?pred=true:pred=false; //aplica threshold sobre la prediccion
		cout<< "resultado del clasificador "<<(pred? " true" : " false")<<endl;
		char* comando = new char[510];
		sprintf(comando, "rm %s", filecsv);
		std::system(comando);
	} else 
		cout << "Error al calcular el BoF descriptor\n";
		//todo umbral de clasificación
	return pred;
}

/**
 *
 */
bool test(Mat SIFTdescriptors,char* salida, char* rFile) {
	string modelsPath = "/home/wences/Documents/data/modelos/";
	int dictSize = 25;
	int iter = 1;
	bool pred = patchClass(SIFTdescriptors,modelsPath,dictSize,iter,salida, rFile);
	return pred;
}
