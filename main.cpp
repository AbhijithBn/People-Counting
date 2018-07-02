



#include "main.h"

using namespace cv;
using namespace std;



int main(void)
{

  VideoCapture cap("/home/pi/Videos/sample-02.mp4");//input video files 
  if (cap.isOpened() == false)  
  {
    cout << "Cannot open the video file" << endl;
    cin.get();
    return -1;
  }

  vector<Blob> blobs;// this object has all the blobs(already existing blobs)

  cap.read(frame_1);//first frame
  cap.read(frame_2);//second frame

  Point refline[2];// this is for the reference line 
  int refposition=(int)round((double)frame_1.rows*0.6);

  refline[0].x=0;//0th column
  refline[1].x=frame_1.cols-1;// to the last column
  refline[0].y=refposition;// no thickness is given to the line
  refline[1].y=refposition;

  double fps = cap.get(CAP_PROP_FPS);//output frames per second
  cout << "Frames per seconds : " << fps << endl;

  pMOG2=createBackgroundSubtractorMOG2(3000,128,false);//false : no shadows
  
  bool blnFirstFrame=true;// since there is only one frame

  int frameCount = 2;

  countup=0;
  countdown=0;//initially both the counts are 0

  

  while (true)
  {
    morph morphobject(frame_1);
    morphobject.input_frame=frame_1;

    frame_binary=morphobject.frame_binary;
    
    vector<Blob> currentFrameBlobs;// current frames as objects to class Blob


    vector<vector<Point> > contours;// contours is a 2D vector: has contours

    findContours(frame_binary,contours,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);// this tracks the contours

    drawcontour(frame_binary.size(),contours,"Contour Window");// this functions draws the contours based on vector input

    vector<vector<Point> > convexHulls(contours.size());// vector which the contours after convexHull operation is performed on them

    for(int i=0;i<contours.size();i++)
    {
      convexHull(contours[i],convexHulls[i],false);// this operation performs element by element 
    }

    drawcontour(frame_binary.size(),convexHulls,"ConvexHull_contour");// to draw contours after convexhull algo has been implemented

    for(auto &convexhull: convexHulls)// convexHull is taken from convexHulls array
    {
      Blob possibleBlob(convexhull);//Blob is class; possibleBlob is object and youre passin these blobs to that class to get the COM etc
      printf("Rectangle area = %d,aspect ratio = %0.1f,Rectangle_width = %d, Rectangle_height = %d,Diagonal_Size = %0.1f \n\n\n",possibleBlob.currentBoundingRect.area(),possibleBlob.dblCurrentAspectRatio,possibleBlob.currentBoundingRect.width,possibleBlob.currentBoundingRect.height,possibleBlob.dblCurrentDiagonalSize);
      
      if(possibleBlob.currentBoundingRect.area()>1500 && possibleBlob.dblCurrentAspectRatio>0.6 && possibleBlob.dblCurrentAspectRatio<4.0 && possibleBlob.currentBoundingRect.width>25 && possibleBlob.currentBoundingRect.height>25 && possibleBlob.dblCurrentDiagonalSize>60.0 && (contourArea(possibleBlob.currentContour)/(double)possibleBlob.currentBoundingRect.area())>0.50)      
      {//check the area,width,height aspect ratio, diagonal size of the rect
        currentFrameBlobs.push_back(possibleBlob);//append these contours(blobs) which satisfy the condition to current frame blobs 
      }

    }// so now currentFrameBlobs will have the blobs which satify the conditions 

   drawcontour(frame_binary.size(),currentFrameBlobs,"blob_satisfy_condition");// to display the blobs that satisfy the conditions taking input from  the object currentFrmeBlobs

    if(blnFirstFrame==true)
    {
      for(auto &currentFrameBlob: currentFrameBlobs)// every blob we have now
      {
        blobs.push_back(currentFrameBlob);// blobs gets rectangles, center locations ,and flags
      }
    }
    else
    {
      matchCurrentFrameBlobsToExistingBlobs(blobs,currentFrameBlobs);// matching it with old blobs
    }


    imgFrame2Copy = frame_2.clone();          // get another copy of frame 2 since we changed the previous frame 2 copy in the processing above
        
    drawBlobInfoOnImage(blobs, imgFrame2Copy);// draws rectangle, fonts on the image
        
    bool blnAtLeastOneBlobCrossedTheLine = checkIfBlobsCrossedTheLine(blobs, refposition, countup, countdown);
        
    if (blnAtLeastOneBlobCrossedTheLine == true) 
    {
        line(imgFrame2Copy, refline[0], refline[1], SCALAR_GREEN, 2);//this makes the line green when the object crosses it 
    }
    else 
    {
        line(imgFrame2Copy, refline[0], refline[1], SCALAR_RED, 2);// else the line is red in colour
    }
        
    drawCountupOnImage(countup, imgFrame2Copy);// do display the incremented count on the image
    drawCountdownOnImage(countdown, imgFrame2Copy);//do display the decremented count on the image
    imshow("imgFrame2Copy", imgFrame2Copy);
        
        //cv::waitKey(0);                 // uncomment this line to go frame by frame for debugging
        
        // now we prepare for the next iteration
        
    currentFrameBlobs.clear();// there are no blobs being proessed now
        
    frame_1 = frame_2.clone();           // move frame 1 up to where frame 2 is
        
    if ((cap.get(CV_CAP_PROP_POS_FRAMES) + 1) < cap.get(CV_CAP_PROP_FRAME_COUNT)) 
    {
        cap.read(frame_2);
    }
    else 
    {
        std::cout << "end of video\n";
        break;
    }
       
    blnFirstFrame = false;
    frameCount++;
    




    if (waitKey(10) == 27)// if the user hits ESC , then stop the video
    {
      cout << "Esc key is pressed by user. Stopping the video" << endl;
      break;
    }
  } 

 return 0;

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void matchCurrentFrameBlobsToExistingBlobs(std::vector<Blob> &existingBlobs, std::vector<Blob> &currentFrameBlobs)// take existing blobs and current blob 
{
    
    for (auto &existingBlob : existingBlobs) // for all blobs that are already there
    {
     
        existingBlob.blnCurrentMatchFoundOrNewBlob = false;//no new blob
        
        existingBlob.predictNextPosition();// using existing blobs predict the next position using the algorithm
    }
    
    for (auto &currentFrameBlob : currentFrameBlobs) // for al blobs we have now
    {
        
        int intIndexOfLeastDistance = 0;//index =0
        double dblLeastDistance = 100000.0;//init least distance
        
        for (unsigned int i = 0; i < existingBlobs.size(); i++) 
        {
            
            if (existingBlobs[i].blnStillBeingTracked == true) 
            {
                
                double dblDistance = distanceBetweenPoints(currentFrameBlob.centerPositions.back(), existingBlobs[i].predictedNextPosition);
                
                if (dblDistance < dblLeastDistance) 
                {
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
    
    for (auto &existingBlob : existingBlobs) 
    {
        
        if (existingBlob.blnCurrentMatchFoundOrNewBlob == false) 
        {
            existingBlob.intNumOfConsecutiveFramesWithoutAMatch++;
        }
        
        if (existingBlob.intNumOfConsecutiveFramesWithoutAMatch >= 5) 
        {
            existingBlob.blnStillBeingTracked = false;
        }
        
    }
    
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void addNewBlob(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs)// for a new blob
{
    
    currentFrameBlob.blnCurrentMatchFoundOrNewBlob = true;// new blob
    
    existingBlobs.push_back(currentFrameBlob);//append new frame/contour
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawBlobInfoOnImage(std::vector<Blob> &blobs, cv::Mat &imgFrame2Copy) //from blobs write info on 2nd frame
{
    
    for (unsigned int i = 0; i < blobs.size(); i++) 
    {
        
        if (blobs[i].blnStillBeingTracked == true) // if the blob is still in the frame and youre still tracking it
        {
            cv::rectangle(imgFrame2Copy, blobs[i].currentBoundingRect, SCALAR_RED, 2);//bound blob in red rectangle
            
            int intFontFace = CV_FONT_HERSHEY_SIMPLEX;//font
            double dblFontScale = blobs[i].dblCurrentDiagonalSize / 60.0;
            int intFontThickness = (int)std::round(dblFontScale * 1.0);
            
            cv::putText(imgFrame2Copy, std::to_string(i), blobs[i].centerPositions.back(), intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);
        }
   }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
double distanceBetweenPoints(cv::Point point1, cv::Point point2) {//pythagoras theorem
    
    int intX = abs(point1.x - point2.x);
    int intY = abs(point1.y - point2.y);
    
    return(sqrt(pow(intX, 2) + pow(intY, 2)));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void addBlobToExistingBlobs(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex)// if the least distance is less than the diagonal size of the rect
 {
    
    existingBlobs[intIndex].currentContour = currentFrameBlob.currentContour;//add the current contour to existing blobs
    existingBlobs[intIndex].currentBoundingRect = currentFrameBlob.currentBoundingRect;//current rectangle is added to already existing rectangles
    
    existingBlobs[intIndex].centerPositions.push_back(currentFrameBlob.centerPositions.back());//center postions are appended
    
    existingBlobs[intIndex].dblCurrentDiagonalSize = currentFrameBlob.dblCurrentDiagonalSize;//diagonal size is appended
    existingBlobs[intIndex].dblCurrentAspectRatio = currentFrameBlob.dblCurrentAspectRatio;//aspect ratio is appended
    
    existingBlobs[intIndex].blnStillBeingTracked = true;//we are tracking for new blobs
    existingBlobs[intIndex].blnCurrentMatchFoundOrNewBlob = true;//new blob is found
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*void display_output(string frame_name,Mat frame_matrix)
{
  namedWindow(frame_name,WINDOW_NORMAL);
  imshow(frame_name,frame_matrix);
}*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void drawcontour(Size imagesize, vector<vector<Point> > contours,string image_name )// displays contours when contours are passed to this function
{
  //from the given imagesize create a matrix 
  Mat image(imagesize,CV_8UC3,SCALAR_BLACK); 
  drawContours(image,contours,-1,SCALAR_WHITE,-1);// from the array , contours are drawn onto the matrix "image" 
  display_output(image_name, image);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void drawcontour(Size imagesize,vector<Blob> blobs, string  image_name)// for the blobs that satisfy the conditions
{
  Mat image(imagesize,CV_8UC3,SCALAR_BLACK); // this is an image

  vector<vector<Point> > contours;
  
  for(auto &blob: blobs)// for every element in blobs
  {
    if (blob.blnStillBeingTracked==true)// if they are still being tracked and added to the array containing good contours
    {
      contours.push_back(blob.currentContour);
    }
  }

  drawContours(image,contours,-1,SCALAR_WHITE,-1);// draw these contours
  display_output(image_name , image);// display the window
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool checkIfBlobsCrossedTheLine(std::vector<Blob> &blobs, int &refposition, int &countup, int &countdown) 
{
    bool blnAtLeastOneBlobCrossedTheLine = false;
    
    for (auto blob : blobs) 
    {
        
        if (blob.blnStillBeingTracked == true && blob.centerPositions.size() >= 2) //blob is still on screen and there are more that 2 blob center as in it is moving
        {
            int prevFrameIndex = (int)blob.centerPositions.size() - 2;//index value
            int currFrameIndex = (int)blob.centerPositions.size() - 1;//index value
            
            if (blob.centerPositions[prevFrameIndex].y > refposition && blob.centerPositions[currFrameIndex].y <= refposition) 
            {
                countup++; //in
                blnAtLeastOneBlobCrossedTheLine = true;
            }
            else if (blob.centerPositions[prevFrameIndex].y < refposition && blob.centerPositions[currFrameIndex].y >= refposition) {
                countdown++; //out
                blnAtLeastOneBlobCrossedTheLine = true;
            }

        }
        
    }
    
    return blnAtLeastOneBlobCrossedTheLine;//return boolean if blobs crossed line
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
void drawCountupOnImage(int &countup, cv::Mat &imgFrame2Copy) //pass the count to this function
{
    
    int intFontFace = CV_FONT_HERSHEY_SIMPLEX;// this is the font
    double dblFontScale = (imgFrame2Copy.rows * imgFrame2Copy.cols) / 300000.0;// this is the size of the font
    int intFontThickness = (int)std::round(dblFontScale * 1.5);// font thickness
    
    Size textSize = getTextSize(std::to_string(countup), intFontFace, dblFontScale, intFontThickness, 0);//Calculates the width and height of a text string
    
    Point ptTextBottomLeftPosition;// this is a Point to the bottom left corner of the screen and has x & y co-ordinates
    
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
