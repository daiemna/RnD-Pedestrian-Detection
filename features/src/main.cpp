#include <opencv2/opencv.hpp>
#include <iostream>

#include "hog_features.h"

using namespace cv;

int test_features(int argc, char **argv);
int testFilter(int argc, char **argv);

int main(int argc, char **argv) {
	return test_features(argc,argv);
}

int test_features(int argc, char **argv){
	if(argc < 2){
		cout << "usage is : " << argv[0] << " path\\to\\image" << endl;
		return -1;
	}
	cout << "Image name : " << argv[1] << endl;
	Mat img = imread(argv[1],CV_LOAD_IMAGE_GRAYSCALE);
	cout << "image size : " << img.cols << "x" << img.rows << endl;
	feat::HOGEvaluator fe; //= new feat::HOGEvaluator();
	if(fe.setImage(img,Size(0,0))){
		fe.generate_features();
		cout << "Features : " << fe.features_ << endl;
	}else{
		cout << "setImage Failed" << endl;
	}
	cout << "exiting..." << endl;
	return 0;
}

int testFilter(int argc, char **argv){
	/// Declare variables
	  Mat src,dst, dst_1,dst_2;

	  Mat kernel_x,kernel_y;
	  Point anchor;
	  double delta;
	  int ddepth;
	  int kernel_size;
	  string window_name = "filter2D Demo";
	  Mat frame;

	  int c=0;

	  /// Load an image
	  frame = imread( argv[1] );

	  if( !frame.data )
	  { return -1; }

	  /// Create window
	  namedWindow( window_name.c_str(), CV_WINDOW_AUTOSIZE );

	  /// Initialize arguments for the filter
	  anchor = Point( -1, -1 );
	  delta = 0;
	  ddepth = -1;
	  // Enable for video capture
//	  VideoCapture cap(0); // open the default camera
//	  float ratio = 720/1080;
//	  float width = 200;
//	  cap.set(CV_CAP_PROP_FRAME_WIDTH,200);
//	  cap.set(CV_CAP_PROP_FRAME_HEIGHT,200);
//	  if(!cap.isOpened()){  // check if we succeeded
//		printf("Unable to open Video Cam");
//		return -1;
//	  }

	  /// Loop - Will filter the image with different kernel sizes each 0.5 seconds
	  int ind = 0;
	  while( (char)c != 27 )
	    {
	      c = waitKey(500);

//		  cap >> frame; // get a new frame from camera
		  cvtColor(frame, dst, CV_BGR2GRAY);
	      /// Update kernel size for a normalized box filter
	        kernel_x = Mat::zeros(1,3,CV_32F);
	        kernel_x.at<float>(0) = -1;
	        kernel_x.at<float>(2) = 1;
	        kernel_y = (-kernel_x.clone()).t();
	      /// Apply filter
	      filter2D(dst, dst_1, ddepth , kernel_x, anchor, delta, BORDER_DEFAULT );
	      filter2D(dst, dst_2, ddepth , kernel_y, anchor, delta, BORDER_DEFAULT );
	      imshow( window_name, dst_1 | dst_2 );
	      ind++;
	    }

	  return 0;
}
