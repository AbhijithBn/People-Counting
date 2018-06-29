//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <opencv2/opencv.hpp>
#include <iostream>
#include "Blob.h"// this is the included class 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//these are included namespaces
using namespace cv;
using namespace std;

//hi




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//these are to specify the colour
const Scalar SCALAR_BLACK = Scalar(0.0, 0.0, 0.0);

const Scalar SCALAR_PINK = Scalar(255.0, 192.0, 203.0);

const Scalar SCALAR_WHITE = Scalar(255.0, 255.0, 255.0);

const Scalar SCALAR_YELLOW = Scalar(0.0, 255.0, 255.0);

const Scalar SCALAR_GREEN = Scalar(0.0, 200.0, 0.0);

const Scalar SCALAR_RED = Scalar(0.0, 0.0, 255.0);

const Scalar SCALAR_BLUE = Scalar(255.0, 0.0, 0);


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//these are the functions


void matchCurrentFrameBlobsToExistingBlobs(std::vector<Blob> &existingBlobs, std::vector<Blob> &currentFrameBlobs);

void addBlobToExistingBlobs(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex);

void addNewBlob(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs);

double distanceBetweenPoints(cv::Point point1, cv::Point point2);

void drawContour(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName);//draw contour with input as contour

void drawContour(cv::Size imageSize, std::vector<Blob> blobs, std::string strImageName);//draw contour with input as object to a class

void drawBlobInfoOnImage(std::vector<Blob> &blobs, cv::Mat &imgFrame2Copy);



bool checkIfBlobsCrossedTheLine(std::vector<Blob> &blobs, int &intRefLinePosition, int &countup, int &countdown);

void drawCountupOnImage(int &countup, cv::Mat &imgFrame2Copy);

void drawCountdownOnImage(int &countdown, cv::Mat &imgFrame2Copy);





void calcCircles(const Mat &input, vector<Vec3f> &circles);

void drawCircle(Mat &input, const vector<Vec3f> &circles);



void display_output(String frame_name,Mat frame_matrix);//this is to display the output but not contours



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// These are matrices

Mat imgFrame1; //First frame 
Mat imgFrame2; //Second frame
Mat frame_gauss;// after gaussian blur
Mat frame_backsub;//after background subtraction
Mat structuringElement5x5;//is a structuring element matrix for binary thresholding

Mat imgFrame1Copy,imgFrame2Copy;//clones of frame 1 and frame 2

Mat imgThresh; //after binary threshold
Mat imgThreshCopy;//used for finding contours

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//these are pointers
cv::Ptr<BackgroundSubtractor> pMOG2; //MOG2 Background subtractor

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//these are global variables

int num_threshold = 254;// this is threshold value for Binary thresholding

int MAX = 255;// this is the maximum value give if it exceeds the threshold value





Mat img_background; //MOG2 background



int main(void) {

    

    VideoCapture cap("/home/pi/Videos/sample-02.mp4"); //input video

    

    Mat imgFrame1;//frame 1

    Mat imgFrame2;//frame 2

    

    vector<Blob> blobs;//"blobs" object of that class Blob

    



    
    namedWindow("imgFrame2Copy",CV_WINDOW_FULLSCREEN);

    

    int countup = 0;//count of the number of people coming in

    int countdown = 0;//count of the number of people going out

    if (!cap.isOpened()) {                                                 // if unable to open video file

        cout << "error reading video file" << endl << endl;      // show error message

                return(0);                                                              // and exit program

    }

    else
    {

        cout<< "success"<<endl;

    }

    if (cap.get(CV_CAP_PROP_FRAME_COUNT) < 2) 
    {

        cout << "error: video file must have at least two frames";

        return(0);

    }



    cap.read(imgFrame1);

    cap.read(imgFrame2);




    //this section of the code is to add the reference line
    Point refLine[2];
    // 2 lines at the middle of the screen which is used as reference for counting purpose
    // refLine[0]has 2 parts refLine[0].x and refLine[0].y
    // refLine[1]has 2 parts refLine[1].x and refLine[1].y


    int intRefLinePosition = (int)round((double)imgFrame1.rows * 0.80);



    refLine[0].x = 0;//from 0th column

    refLine[0].y = intRefLinePosition;



    refLine[1].x = imgFrame1.cols - 1;// to the last column

    refLine[1].y = intRefLinePosition;// thickness along y axis is 0





    char chCheckForEscKey = 0;//this is to abort the program and initially it is 0



    bool blnFirstFrame = true;//boolean to check if it is the first frame

    int frameCount = 2;//now number of frames =2

    
    pMOG2 = createBackgroundSubtractorMOG2(3000, 128, false); //MOG2 approach, true with shadow, false without shadow

    

    while(true)
    {

        vector<Blob> currentFrameBlobs;//object currentFrameBlobs beongs to class Blob
	
	GaussianBlur(imgFrame1,frame_gauss,Size(5,5),3);//Gaussian blur using 5X5 kernel



	//display_output( "Gaussian",frame_gauss);


        pMOG2->apply(frame_gauss, frame_backsub);//background subtraction using MOG2

        
	//display_output("BACKGROUND_SUB",frame_backsub);


	//this is to create a structuring element for binary thresholding

        structuringElement5x5 = getStructuringElement(MORPH_RECT, Size(5, 5));

        Mat structuringElement7x7 = getStructuringElement(MORPH_RECT, Size(7, 7));
	Mat structuringElement3x3 = getStructuringElement(MORPH_RECT, Size(3, 3));


	//Binary thresholding

        threshold(frame_backsub, imgThresh, num_threshold, MAX, THRESH_BINARY); // from 0~MAX num>num_threshold set white, num<num_threshold set black

       

        //display_output("BINARY IMAGE",imgThresh);

        for (unsigned int i = 0; i < 2; i++) 
	{

            erode(imgThresh, imgThresh, structuringElement3x3);

            dilate(imgThresh, imgThresh, structuringElement3x3);

            dilate(imgThresh, imgThresh, structuringElement3x3);


            

        }


        
	//display_output("DILATION AND EROSION",imgThresh);


        imgThreshCopy = imgThresh.clone();//cloning of matrix(duplicte)

        

        vector<vector<Point> > contours;//contours is a 2D vector which contains all the contours after finding them

        

        findContours(imgThreshCopy, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);//RETR_EXTERNAL retrieves only extreme outer contours
	// 3rd parameter is contour retrieval mode,4th parameter approximates the edges

	//CHAIN_APPROX_SIMPLE approximates the contour to small rectangles

        

        drawContour(imgThresh.size(), contours, "imgContours");//sends the contour array to the function to draw contours

        




        vector<vector<Point> > convexHulls(contours.size());//convexHulls is similar to contours but creates a whole approxiated image from the irregularly shaped image

        

        for (unsigned int i = 0; i < contours.size(); i++) 
	{

            convexHull(contours[i], convexHulls[i],false);

        }

        

        //drawContour(imgThresh.size(), convexHulls, "imgConvexHulls");//convexHulls is an array

        

        for (auto &convexHull : convexHulls) 
	{

            Blob possibleBlob(convexHull);
	    printf("rect_area= %d  ,aspect ratio = %.1f ,rect width= %d ,rect_height = %d ,diagonal size = %.1f \n\n\n ",possibleBlob.currentBoundingRect.area(),possibleBlob.dblCurrentAspectRatio,possibleBlob.currentBoundingRect.width,possibleBlob.currentBoundingRect.height,possibleBlob.dblCurrentDiagonalSize);

            

            if (possibleBlob.currentBoundingRect.area() > 600 &&

                possibleBlob.dblCurrentAspectRatio > 0.6 &&

                possibleBlob.dblCurrentAspectRatio < 4.0 &&

                possibleBlob.currentBoundingRect.width > 25 && //imgblock width

                possibleBlob.currentBoundingRect.height > 25 && //imgblock height

                possibleBlob.dblCurrentDiagonalSize > 60.0 &&

                (cv::contourArea(possibleBlob.currentContour) / (double)possibleBlob.currentBoundingRect.area()) > 0.50) {

                currentFrameBlobs.push_back(possibleBlob);

            }

        }

        

        drawContour(imgThresh.size(), currentFrameBlobs, "imgCurrentFrameBlobs");//currentFrameBlobs is an object to a function 





        if (blnFirstFrame == true) //if it is the first frame
	{

            for (auto &currentFrameBlob : currentFrameBlobs) //for every blob in the current frame
	    {

                blobs.push_back(currentFrameBlob);//push that blob into blobs for tracking

            }

        } 
	else 
	{

            matchCurrentFrameBlobsToExistingBlobs(blobs, currentFrameBlobs);// else match it with an already existing blob

        }//by passing the object blobs of class Blob and current Blob we are working on

        

        //drawContour(imgThresh.size(), blobs, "imgBlobs");

        

        imgFrame2Copy = imgFrame2.clone();          // get another copy of frame 2 since we changed the previous frame 2 copy in the processing above

        

        //drawBlobInfoOnImage(blobs, imgFrame2Copy);

        

        bool blnAtLeastOneBlobCrossedTheLine = checkIfBlobsCrossedTheLine(blobs, intRefLinePosition, countup, countdown);

        

        if (blnAtLeastOneBlobCrossedTheLine == true) {

            cv::line(imgFrame2Copy, refLine[0], refLine[1], SCALAR_GREEN, 2);

        }

        else {

            cv::line(imgFrame2Copy, refLine[0], refLine[1], SCALAR_RED, 2);

        }

        

        drawCountupOnImage(countup, imgFrame2Copy);

        drawCountdownOnImage(countdown, imgFrame2Copy);

        cv::imshow("imgFrame2Copy", imgFrame2Copy);

        

        //cv::waitKey(0);                 // uncomment this line to go frame by frame for debugging

        

        // now we prepare for the next iteration

        

        currentFrameBlobs.clear();

        

        imgFrame1 = imgFrame2.clone();           // move frame 1 up to where frame 2 is

        

        if ((cap.get(CV_CAP_PROP_POS_FRAMES) + 1) < cap.get(CV_CAP_PROP_FRAME_COUNT)) {

            cap.read(imgFrame2);

        }

        else {

            std::cout << "end of video\n";

            break;

        }

        

        blnFirstFrame = false;

        frameCount++;

        chCheckForEscKey = cv::waitKey(1);

	if (waitKey(10) == 27)
	{
		cout << "Esc key is pressed by user. Stopping the video" << endl;
		break;
	}

    }





    return(0);

}





void calcCircles(const Mat &input, vector<Vec3f> &circles){

    Mat contours;

    Canny(input,contours,50,150);

    HoughCircles(contours, circles, CV_HOUGH_GRADIENT, 1, 100, 200, 10,15,20);

}



void drawCircle(Mat &input, const vector<Vec3f> &circles){

    for(int i=0; i<circles.size(); i++){

        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));

        int radius = cvRound(circles[i][2]);

        circle(input, center, radius, SCALAR_BLUE, 3, 8, 0 );

    }

}







void matchCurrentFrameBlobsToExistingBlobs(std::vector<Blob> &existingBlobs, std::vector<Blob> &currentFrameBlobs) 
{//so blobs will have all the existing blobs

    

    for (auto &existingBlob : existingBlobs) 
    {

        

        existingBlob.blnCurrentMatchFoundOrNewBlob = false;//init this match with false so that you can compare 

        // and this blnCurrentMatchFoundOrNewBlob is in blob.cpp


        existingBlob.predictNextPosition();//take this blob and predict the next poisiton in blob.cpp using that algorithm

    }

    

    for (auto &currentFrameBlob : currentFrameBlobs) //for every blob we have now
    {

        

        int intIndexOfLeastDistance = 0;//index value

        double dblLeastDistance = 100000.0;//least dist value

        

        for (unsigned int i = 0; i < existingBlobs.size(); i++) 
	{

            

            if (existingBlobs[i].blnStillBeingTracked == true) 
	    {

                

                double dblDistance = distanceBetweenPoints(currentFrameBlob.centerPositions.back(), existingBlobs[i].predictedNextPosition);

                

                if (dblDistance < dblLeastDistance) {

                    dblLeastDistance = dblDistance;

                    intIndexOfLeastDistance = i;

                }

            }

        }

        

        if (dblLeastDistance < currentFrameBlob.dblCurrentDiagonalSize * 0.5) {

            addBlobToExistingBlobs(currentFrameBlob, existingBlobs, intIndexOfLeastDistance);

        }

        else {

            addNewBlob(currentFrameBlob, existingBlobs);

        }

        

    }

    

    for (auto &existingBlob : existingBlobs) {

        

        if (existingBlob.blnCurrentMatchFoundOrNewBlob == false) {

            existingBlob.intNumOfConsecutiveFramesWithoutAMatch++;

        }

        

        if (existingBlob.intNumOfConsecutiveFramesWithoutAMatch >= 5) {

            existingBlob.blnStillBeingTracked = false;

        }

        

    }

    

}



///////////////////////////////////////////////////////////////////////////////////////////////////

void addBlobToExistingBlobs(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex) {

    

    existingBlobs[intIndex].currentContour = currentFrameBlob.currentContour;

    existingBlobs[intIndex].currentBoundingRect = currentFrameBlob.currentBoundingRect;

    

    existingBlobs[intIndex].centerPositions.push_back(currentFrameBlob.centerPositions.back());

    

    existingBlobs[intIndex].dblCurrentDiagonalSize = currentFrameBlob.dblCurrentDiagonalSize;

    existingBlobs[intIndex].dblCurrentAspectRatio = currentFrameBlob.dblCurrentAspectRatio;

    

    existingBlobs[intIndex].blnStillBeingTracked = true;

    existingBlobs[intIndex].blnCurrentMatchFoundOrNewBlob = true;

}



///////////////////////////////////////////////////////////////////////////////////////////////////

void addNewBlob(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs) {

    

    currentFrameBlob.blnCurrentMatchFoundOrNewBlob = true;

    

    existingBlobs.push_back(currentFrameBlob);

}



///////////////////////////////////////////////////////////////////////////////////////////////////

double distanceBetweenPoints(cv::Point point1, cv::Point point2) {

    

    int intX = abs(point1.x - point2.x);

    int intY = abs(point1.y - point2.y);

    

    return(sqrt(pow(intX, 2) + pow(intY, 2)));

}



///////////////////////////////////////////////////////////////////////////////////////////////////

void drawContour(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName) 
{  //this drawsContour taking input of the contour itself

    
    Mat image(imageSize, CV_8UC3, SCALAR_BLACK);//8bit unsigned integer 3 channel (colour image)

    

    drawContours(image, contours, -1, SCALAR_WHITE, -1);//image is destination and contours is the source, -ve and hence contours are filled

    

    imshow(strImageName, image);//for output display

}



///////////////////////////////////////////////////////////////////////////////////////////////////

void drawContour(cv::Size imageSize, std::vector<Blob> blobs, std::string strImageName) {

    

    cv::Mat image(imageSize, CV_8UC3, SCALAR_BLACK);

    

    std::vector<std::vector<cv::Point> > contours;

    

    for (auto &blob : blobs) {

        if (blob.blnStillBeingTracked == true) 
	{

            contours.push_back(blob.currentContour);

        }

    }

    

    cv::drawContours(image, contours, -1, SCALAR_WHITE, -1);

    

    cv::imshow(strImageName, image);

}



///////////////////////////////////////////////////////////////////////////////////////////////////

bool checkIfBlobsCrossedTheLine(std::vector<Blob> &blobs, int &intRefLinePosition, int &countup, int &countdown) 
{

    bool blnAtLeastOneBlobCrossedTheLine = false;//just initialising this boolean to be false, we're not saying blob.boolean is false

    

    for (auto blob : blobs) {

        

        if (blob.blnStillBeingTracked == true && blob.centerPositions.size() >= 2) 
	{

            int prevFrameIndex = (int)blob.centerPositions.size() - 2;

            int currFrameIndex = (int)blob.centerPositions.size() - 1;

            

            if (blob.centerPositions[prevFrameIndex].y > intRefLinePosition && blob.centerPositions[currFrameIndex].y <= intRefLinePosition) {

                countdown++; //in

                blnAtLeastOneBlobCrossedTheLine = true;

            }

            else if (blob.centerPositions[prevFrameIndex].y < intRefLinePosition && blob.centerPositions[currFrameIndex].y >= intRefLinePosition) {

                countup++; //out

                blnAtLeastOneBlobCrossedTheLine = true;

            }



        }

        

    }

    

    return blnAtLeastOneBlobCrossedTheLine;

}



///////////////////////////////////////////////////////////////////////////////////////////////////

void drawBlobInfoOnImage(std::vector<Blob> &blobs, cv::Mat &imgFrame2Copy) {

    

    for (unsigned int i = 0; i < blobs.size(); i++) {

        

        if (blobs[i].blnStillBeingTracked == true) {

            cv::rectangle(imgFrame2Copy, blobs[i].currentBoundingRect, SCALAR_RED, 2);

            

            int intFontFace = CV_FONT_HERSHEY_SIMPLEX;

            double dblFontScale = blobs[i].dblCurrentDiagonalSize / 60.0;

            int intFontThickness = (int)std::round(dblFontScale * 1.0);

            

            cv::putText(imgFrame2Copy, std::to_string(i), blobs[i].centerPositions.back(), intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);

        }

   }

}



///////////////////////////////////////////////////////////////////////////////////////////////////

void drawCountupOnImage(int &countup, cv::Mat &imgFrame2Copy) {

    

    int intFontFace = CV_FONT_HERSHEY_SIMPLEX;

    double dblFontScale = (imgFrame2Copy.rows * imgFrame2Copy.cols) / 300000.0;

    int intFontThickness = (int)std::round(dblFontScale * 1.5);

    

    cv::Size textSize = cv::getTextSize(std::to_string(countup), intFontFace, dblFontScale, intFontThickness, 0);

    

    cv::Point ptTextBottomLeftPosition;

    

    ptTextBottomLeftPosition.x = imgFrame2Copy.cols - 1 - (int)((double)textSize.width * 6);

    ptTextBottomLeftPosition.y = (int)((double)textSize.height * 1.25);

    

    cv::putText(imgFrame2Copy, "in : "+std::to_string(countup), ptTextBottomLeftPosition, intFontFace, dblFontScale, SCALAR_BLACK, intFontThickness);

    

}

void drawCountdownOnImage(int &countdown, cv::Mat &imgFrame2Copy) 
{

    

    int intFontFace = CV_FONT_HERSHEY_SIMPLEX;

    double dblFontScale = (imgFrame2Copy.rows * imgFrame2Copy.cols) / 300000.0;

    int intFontThickness = (int)std::round(dblFontScale * 1.5);

    

    cv::Size textSize = cv::getTextSize(std::to_string(countdown), intFontFace, dblFontScale, intFontThickness, 0);

    

    cv::Point ptTextBottomLeftPosition;

    

    ptTextBottomLeftPosition.x = imgFrame2Copy.cols - 1 - (int)((double)textSize.width * 7);

    ptTextBottomLeftPosition.y = (int)((double)textSize.height * 2.5);

    

    cv::putText(imgFrame2Copy, "out : "+std::to_string(countdown), ptTextBottomLeftPosition, intFontFace, dblFontScale, SCALAR_BLACK, intFontThickness);

    

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void display_output(String frame_name,Mat frame_matrix)
{
	namedWindow(frame_name, WINDOW_NORMAL);
	imshow(frame_name,frame_matrix);
}

