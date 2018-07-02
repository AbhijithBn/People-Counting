#ifndef TRIAL_1_H
#define TRIAL_1_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include "Blob.h"
#include "Morph.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace cv;
using namespace std;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Matrices
Mat frame_1;//first frame
Mat frame_2;//second frame
Mat imgFrame2Copy;
Mat frame_gauss;//after gaussian blur
Mat frame_backsub;//after background subtraction
Mat frame_binary;//after thresholding
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int countup;// no of people going in
int countdown;// no of people going out
int threshold_value=254;// any value below this will be 0, and any value above this will be MAX_VALUE as given below
int MAX_VALUE=255;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//COLOURS
const Scalar SCALAR_BLACK=Scalar(0.0,0.0,0.0);
const Scalar SCALAR_WHITE=Scalar(255.0,255.0,255.0);
const Scalar SCALAR_PINK=Scalar(255.0,192.0,203.0);
const Scalar SCALAR_YELLOW=Scalar(0.0,255.0,255.0);
const Scalar SCALAR_GREEN=Scalar(0.0,200.0,0.0);
const Scalar SCALAR_RED=Scalar(0.0,0.0,255.0);
const Scalar SCALAR_BLUE=Scalar(255.0,0.0,0.0);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//functions
int main (void);
void display_output(string frame_name, Mat frame_matrix);// function to display the matrices
void drawcontour(Size imagesize, vector<vector<Point> > contours ,string image_name);// to draw contours taking input from contour array/vector
void drawcontour(Size imagesize, vector<Blob> blobs , string image_name);//draw contours when a blob (object) has been given
void matchCurrentFrameBlobsToExistingBlobs(vector<Blob> &existingBlobs,vector<Blob> &currentFrameBlobs);//match contour we have now to already existing ones 
void addNewBlob(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs);// if the new blobs center position is larger than (1/2)diagonal size then make this blob as new blob
void drawBlobInfoOnImage(std::vector<Blob> &blobs, cv::Mat &imgFrame2Copy);//this is to add numbers/counts and specific fonts on the image
double distanceBetweenPoints(cv::Point point1, cv::Point point2);//this is pythagoras theorem between the existing blob and the predicted next blob
void addBlobToExistingBlobs(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex);//if the new blobs center position is lesser than (1/2)diagonal size then add this blob to the existing blob
bool checkIfBlobsCrossedTheLine(std::vector<Blob> &blobs, int &refposition, int &countup, int &countdown); //keeping track of the center position of the blobs , check if they crossed the refline
void drawCountupOnImage(int &countup, cv::Mat &imgFrame2Copy);//to display the count on the top right corner of the frame
void drawCountdownOnImage(int &countdown, cv::Mat &imgFrame2Copy) ;//to display the count on the top right corner of the frame


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Pointer
Ptr<BackgroundSubtractor> pMOG2;//pointer of type BackgroundSubtractor from opencv library
  
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif