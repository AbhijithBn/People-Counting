#include "Morph.h"

using namespace cv;
using namespace std;

extern Ptr<BackgroundSubtractor> pMOG2;
extern int threshold_value;
extern int MAX_VALUE;

morph::morph(Mat input_matrix)
{
	input_frame=input_matrix;

	GaussianBlur(input_frame,frame_gauss,Size(5,5),3);

	//display_output("gauss",frame_gauss);

	pMOG2->apply(frame_gauss,frame_backsub);

	//display_output("BackgroundSubtractor",frame_backsub);

	Mat structuringElement=getStructuringElement(MORPH_RECT,Size(5,5));

	threshold(frame_backsub,frame_binary,threshold_value,MAX_VALUE,THRESH_BINARY);

	//display_output("Binary",frame_binary);
	
	for(int i=0;i<2;i++)
    {
      erode(frame_binary,frame_binary,structuringElement);
      dilate(frame_binary,frame_binary,structuringElement);
      dilate(frame_binary,frame_binary,structuringElement);
    
    }

    
}
void display_output(string frame_name,Mat frame_matrix)
{
  	namedWindow(frame_name,WINDOW_NORMAL);
	imshow(frame_name,frame_matrix);
}