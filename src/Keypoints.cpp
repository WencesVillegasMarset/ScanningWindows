/*
 * BoFSIFT2.cpp
 *
 *  Created on: 15 oct. 2016
 *      Author: seba
 */

#include "Keypoints.h"


std::vector<KeyPoint> detectarKpSIFT(Mat& img, Ptr<SIFT>& detector) {
	vector<KeyPoint> keypoints;
	detector -> detect(img, keypoints);
	return keypoints;
}

void calcularDescriptor(Mat& img, vector<KeyPoint> keypoints, Mat& descriptor, Ptr<SIFT>& detector, string nombreImg) {

	if (img.empty()) {
		printf("\t#ERROR: No se puede cargar la imagen '%s'\n", nombreImg.c_str());
	} else {
		//Calcula el descriptor para cada keypoint y lo guarda en la matriz descriptor
		cout << "Computando kpoints\n";
		detector->compute(img, keypoints, descriptor);
	}
}

void guardarCoordenadas(Mat& descriptor, vector<KeyPoint> keypoints) { // keyPoint x (8 + 128)
		Mat coordKP(descriptor.rows, 8, descriptor.type()); //Crea una matriz de nx8 de coordenadas
		cout <<"Guardando coordenadas\n";
		for (int i = 0; i < coordKP.rows; i++) {
			coordKP.at<float>(i, 0) = keypoints[i].pt.x;
			coordKP.at<float>(i, 1) = keypoints[i].pt.y;
			coordKP.at<float>(i,2)    = -1.0; //esta columna se usará para guardar el landmark en el flannmatcher
			coordKP.at<float>(i, 3) = keypoints[i].angle;
			coordKP.at<float>(i, 4) = keypoints[i].class_id;
			coordKP.at<float>(i, 5) = keypoints[i].octave;
			coordKP.at<float>(i, 6) = keypoints[i].response;
			coordKP.at<float>(i, 7) = keypoints[i].size;
		}
		hconcat(coordKP, descriptor, descriptor);
}


void guardarDescriptor(Mat& descriptor, char* nombreDescriptor) {

	//Escribe el descriptor en el archivo
	cout << "Guardando descriptor en "<<nombreDescriptor<<"\n";
	std::ofstream fs1(nombreDescriptor);

	//Información de la imagen
	fs1 << format(descriptor, Formatter::FMT_CSV) << endl;
	fs1.close();

}

std::vector<KeyPoint> cornerHarris(Mat src_gray, int thresh){
	 Mat dst, dst_norm, dst_norm_scaled;
	 std::vector<KeyPoint> goodKeypoints;
	  dst = Mat::zeros( src_gray.size(), CV_32FC1 );
	  int blockSize = 2;
	  int apertureSize = 3;
	  double k = 0.04;
	  cornerHarris( src_gray, dst, blockSize, apertureSize, k, BORDER_DEFAULT );
	  normalize( dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat() );
	  convertScaleAbs( dst_norm, dst_norm_scaled );
	  for( int j = 0; j < dst_norm.rows ; j++ )
	     { for( int i = 0; i < dst_norm.cols; i++ )
	          {
	            if( (int) dst_norm.at<float>(j,i) > thresh )
	              {
	            	goodKeypoints.push_back(cv::KeyPoint(i,j, 1.f));
	              }
	          }
	     }
	  return(goodKeypoints);
}

std::vector<KeyPoint> recuperarCoordenadasKeypoint(Mat& descriptor) {
	//recupera punto por punto los keypoints del descriptor y los guarda en el vector
		cv::Point2f punto;
		vector<KeyPoint> coordenadasKeypoints;

		for (int j=0; j < descriptor.rows; j++){
			punto = cv::Point2f(descriptor.at<float>(j,0),descriptor.at<float>(j,1));
			coordenadasKeypoints.push_back(cv::KeyPoint(punto, 1.f));
		}
	return coordenadasKeypoints;
}

void drawKeypoints2( Mat& image, const std::vector<KeyPoint>& keypoints, Mat& outImage,const Scalar& color, int radius){

	Point pt1;
	if (image.channels()<3){
		cv::cvtColor(image, image, COLOR_GRAY2BGR);
	}
	for (int i=0; i < (int)keypoints.size(); i++){
		pt1 = Point(keypoints[i].pt.x, keypoints[i].pt.y);
		circle(image, pt1, radius, color, 2, 8, 0);
	}
	outImage = image.clone();

}


