
#include <opencv2/opencv.hpp>
#include <iostream>
//#include "background_segm.hpp"

using namespace cv;
using namespace std;

int thresh=100;
int max_thresh=255;
RNG rng(12345);

//init all the frames
Mat input_frame;//input frame from the video
Mat frame_gauss;// frame after gaussian blur
Mat frame_backsub;//frame after background subtraction
Mat frame_binary;//frame after binary threshold
Mat frame_dilate;// frame after dilation
Mat frame_contour_ip;//frame input to contour finder

Mat inputframeprocessing(int,void*);
void contourfinder(int, void* );
//void display();


Ptr< BackgroundSubtractorMOG2> pMOG2;// this is a pointer to the object that is  created when background subtraction is called




int main(int argc, char* argv[])
{
	 //open the video file for reading
 	VideoCapture cap("/home/pi/Videos/sample-02.mp4"); 

	 // if not success, exit program
 	if (cap.isOpened() == false)  
	 {
  		cout << "Cannot open the video file" << endl;
  		cin.get(); //wait for any key press
  		return -1;
	 }



 	//get the frames rate of the video
 	double fps = cap.get(CAP_PROP_FPS); 
 	cout << "Frames per seconds : " << fps << endl;


	// String window_name_2 = "Processed Video";
	 String window_name_1 = "Input Video";
	 namedWindow(window_name_1, WINDOW_NORMAL); //create a window



 	while (true)
	 {

  		bool bSuccess = cap.read(input_frame); // read a new frame from video 
 	 	//Breaking the while loop at the end of the video
 	 	if (bSuccess == false) //  check if frame is grabbed  
		{
   			cout << "Found the end of the video" << endl;
   			break;
  		}

	        pMOG2 = createBackgroundSubtractorMOG2(3000,128);

		inputframeprocessing(0,0);// this is callback function for tuning the frame

 		// this is to create a trackbar 
		createTrackbar( " Contour thresh:", "Contours", &thresh, max_thresh, contourfinder );//contour thresh is name of the trackbar
	  	//the trackbar is created in "Contours" window
  		//&thresh is a pointer to thresh
  		// max_thresh is the maximum threshold value that can be given
  		// threshcallback is the callback function that is referred here



	  	contourfinder( 0, 0 );// this is a callback function
       		//void display();




  		//show the frame in the created window
  		imshow(window_name_1,input_frame);
		//imshow(window_name_2,frame_dilate);


	  	if (waitKey(10) == 27)
  		{
   			cout << "Esc key is pressed by user. Stopping the video" << endl;
   			break;
  		}
 	}
	return 0;

}


Mat inputframeprocessing(int,void*)// this is a callback function which processes the input frame
{

  //this is for gaussian blur
  GaussianBlur(input_frame,frame_gauss,Size(5,5),3);


  // resize the image
  resize(frame_gauss,frame_backsub, Size(frame_gauss.size().width ,frame_gauss.size().height));


  // background subtraction
  pMOG2->apply(frame_backsub,frame_backsub);


  //this is binary thresholding 
  threshold(frame_backsub, frame_binary, 160 , 255, CV_THRESH_BINARY);
  namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
  imshow( "Contours", frame_backsub );

  // this is to dilate the binary image 
  dilate(frame_binary, frame_dilate, getStructuringElement(MORPH_RECT, Size(3, 3)));

  // for reswizing before finding contours
  resize(frame_dilate,frame_contour_ip,Size(frame_dilate.size().width*4 ,frame_dilate.size().height*4));// resize the input dilated image and sends an output image of name  frame_contour_ip

  return(frame_contour_ip);
}




void contourfinder(int, void* )
{
  Mat canny_output;// matrix after applying canny edge detection

  vector<vector<Point> > contours;// this is a 2 dimensional integer vector
  vector<Vec4i> hierarchy;// this is a 4 dimensional vector

  /// Detect edges using canny
  Canny( frame_contour_ip, canny_output, thresh, thresh*2, 3 );//here 3 is the kernel size,thresh->lower threshold , thresh*2->lower threshold, hence this gives a range where to find the object  , values lower than threshold will be discarded, values between the higher and the lower threshold will be considered

  /// Find contours
  findContours( canny_output, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );//takes input image matrix and gives an edge detected matrix

   /// Get the moments
  vector<Moments> mu(contours.size() );
  for( int i = 0; i < contours.size(); i++ )
     { mu[i] = moments( contours[i], false ); }// this is to find the moments



  //after finding contours you need to find vectors so that you con find the centre of mass 
  vector<Point2f> mc( contours.size() ); //here mc refers to mass center
  for( int i = 0; i < contours.size(); i++ ) // for loop with respect to contour size
     { mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 ); } //this is the formula that is used to find the ceter of mass of the object in the image or video


 
   /// Approximate contours to polygons + get bounding rects and circles
  vector<vector<Point> > contours_poly( contours.size() );//  Template class for 2D points specified by its coordinates x and y 
  vector<Rect> boundRect( contours.size() );// represents a 2D rectangle with coordinates specified
  vector<Point2f>center( contours.size() );// vector pointer to the center of the circle
  vector<float>radius( contours.size() );// vector pointer to the radius of the circle

  double dblDiagonalSize;
  double dblAspectRatio;




  for( int i = 0; i < contours.size(); i++ )
     { approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );//this approximates the polygonal curve with specified precision.. 3 is epsilon which has to be changed for better precision
       boundRect[i] = boundingRect( Mat(contours_poly[i]) );//has to be bound by rectangles
       minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );//the contour has to be bound by a circle 
     }


   /// Draw polygonal contour + bounding rects + circles
   Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );// drawing is a matrix which is by default filled with 0 zeros , takes arguments as size of the output of the canny function
   for(int i = 0; i< contours.size(); i++ )//for loop to draw the contours 
     {
       printf(" * Contour[%d] - Area (M_00) = %.2f - Area OpenCV: %.2f  \n", i, mu[i], contourArea(contours[i]) );// display the area 
       Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );// scalar is used to pass the pixel values
       drawContours( drawing, contours_poly, i, color, 2, 8, hierarchy, 0, Point() );//this draws contours/edges around the object desired
       if(contourArea(contours[i])>1500 && contourArea(contours[i])<90000)//area condition for objects 
       {
         rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );//draw a rectangle with dynamically created length and breadth
         // circle(drawing,Point(center.x,center.y),5,color,2,8,0);

          Point center = Point((boundRect[i].x + boundRect[i].width)/2, (boundRect[i].y + boundRect[i].height)/2);//this is a pointer to the middle of every square


          line(drawing,Point(center.x,center.y),Point(center.x,center.y-25),color,2,8,0);//line is used to draw lines 
	  line(drawing,Point(center.x,center.y),Point(center.x,center.y+25),color,2,8,0);
	  line(drawing,Point(center.x,center.y),Point(center.x-25,center.y),color,2,8,0);
	  line(drawing,Point(center.x,center.y),Point(center.x+25,center.y),color,2,8,0);
 	  putText(drawing,"object is here ",Point(center.x,center.y),1,1,color,2,8,0);   

         // bool blnStillBeingTracked = true;
         // bool blnCurrentMatchFoundOrNewBlob = true;
         // int NumOfConsecutiveFramesWithoutAMatch = 0;

       }
     }

  bool object_present;// check if there is any person walking by checking if there are and detected edges
  if(contours.size()>0)// this part of the code is not mandatory
	object_present=true;
  else
	object_present=false;



  /// Show in a window


}

