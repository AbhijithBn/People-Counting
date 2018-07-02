#ifndef MORPH_H
#define MORPH_H

// this program is used to tune the input image using gaussian blur, background subtraction , binary threshold  taking the input frame

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

class morph// this programs contains only definition to all the operations
{

	public:
	morph(Mat input_matrix);
    
	Mat input_frame;// this is the copy of the input matrix
	Mat frame_gauss;//after gaussian blur
	Mat frame_backsub;//after background subtraction
	Mat frame_binary;//after thresholding

	void display_output(string frame_name,Mat frame_matrix)
	{
  		namedWindow(frame_name,WINDOW_NORMAL);
		imshow(frame_name,frame_matrix);
	}
};


#endif