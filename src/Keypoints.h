/* This file was automatically generated.  Do not edit! */
/*
#include "FlannMatcher.h"
#include "FlannMatcherSinDistancia.h"
*/
#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/ml.hpp"
#include <opencv/cv.h>
#include <opencv2/imgproc.hpp>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <limits>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

using namespace boost::filesystem;
using namespace boost;
using namespace cv;
using namespace std;
using namespace xfeatures2d;



void calcularDescriptor(Mat& img, vector<KeyPoint> keypoints, Mat& descriptor, Ptr<SIFT>& detector,  string nombreImg);
void guardarCoordenadas(Mat& descriptor, vector<KeyPoint> keypoints);
std::vector<KeyPoint> detectarKpSIFT(Mat& img, Ptr<SIFT>& detector);
void guardarDescriptor(Mat& descriptor, char* nombreDescriptor);
std::vector<KeyPoint> cornerHarris(Mat imggrey, int thresh);
std::vector<KeyPoint> recuperarCoordenadasKeypoint(Mat& descriptor);
void drawKeypoints2( Mat& image, const std::vector<KeyPoint>& keypoints, Mat& outImage,const Scalar& color, int radius);

