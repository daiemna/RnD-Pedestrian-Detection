#include <opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int captureStream(int argc,char* argv[]);

int main(int argc,char* argv[]){

	return captureStream(argc,argv);
}

int captureStream(int argc,char* argv[]){
	VideoCapture cap(0); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
        return -1;

    Mat edges;
    namedWindow("edges",1);
    for(;;)
    {
        Mat frame;
        cap >> frame; // get a new frame from camera
        // cvtColor(frame, edges, CV_BGR2GRAY);
        GaussianBlur(frame, edges, Size(7,7), 1.5, 1.5);
        // Canny(edges, edges, 0, 30, 3);
        imshow("edges", edges);
        if(waitKey(30) >= 0) break;
    }
    return 0;
}