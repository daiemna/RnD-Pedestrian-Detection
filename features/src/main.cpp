#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include "hog_features.h"

using namespace cv;
#define ERROR_LOG printf
#define DEBUG_LOG printf
#define DEBUG_STREAM cout

int test_features(int argc, char **argv);
int testFilter(int argc, char **argv);
int testFile();
int read_image_dir(int argc, char **argv);
int genrate_cv_HOG(int argc, char **argv);

int main(int argc, char **argv) {
//	test_features(argc,argv);
//	testFile();
	read_image_dir(argc,argv);
}
int genrate_cv_HOG(int argc, char **argv){
//	if(argc != 3){
//			ERROR_LOG("Usage id ./Main path/to/image/folder path/to/csv/file.csv\n");
//			return -1;
//		}
//		string dir(argv[1]), delm = " ";
//		DIR *dp,*odp;
//		struct dirent *dirp;
//		struct stat filestat;
//		ofstream out_file;
//
//		dp = opendir(argv[1]);
//		if (dp == NULL){
//			ERROR_LOG("Error(%d): opening %s\n",errno,argv[1]);
//			return errno;
//		}
//
//		cv::HOGDescriptor* fe = new cv::HOGDescriptor(Size(64,128),Size(),Size(),Size(),9);
//		while((dirp = readdir( dp ))){
//			string filepath = dir + "/" + dirp->d_name;
//
//			// If the file is a directory (or is in some way invalid) we'll skip it
//			if (stat(filepath.c_str(), &filestat)) continue;
//			if (S_ISDIR(filestat.st_mode))         continue;
//
//			DEBUG_LOG("reading file %s\n",filepath.c_str());
//			Mat frame;
//			frame = imread(filepath.c_str(),CV_LOAD_IMAGE_GRAYSCALE);
//			resize(frame,frame,Size(64,128));
//			if(!frame.data){
//				ERROR_LOG("Cannot open image %s\n",filepath.c_str());
//				continue;
//			}else{
//				DEBUG_STREAM << "image read!" << endl;
//			}
//
//			if(fe->setImage(frame,Size(0,0))){
//				fe->generate_features();
//				fe->write_features(argv[2]);
//				DEBUG_STREAM << "Features : " << fe->features_.rows << endl;
//			}else{
//				DEBUG_STREAM << "setImage Failed" << endl;
//			}
//		}
//		closedir(dp);
////		out_file.close();
//		return 0;
}
int read_image_dir(int argc, char **argv){
	if(argc != 3){
			ERROR_LOG("Usage id ./Main path/to/image/folder path/to/csv/file.csv\n");
			return -1;
		}
		string dir(argv[1]), delm = " ";
		DIR *dp,*odp;
		struct dirent *dirp;
		struct stat filestat;
		ofstream out_file;

		dp = opendir(argv[1]);
		if (dp == NULL){
			ERROR_LOG("Error(%d): opening %s\n",errno,argv[1]);
			return errno;
		}
		feat::HOGEvaluator* fe = new feat::HOGEvaluator();
		while((dirp = readdir( dp ))){
			string filepath = dir + "/" + dirp->d_name;

			// If the file is a directory (or is in some way invalid) we'll skip it
			if (stat(filepath.c_str(), &filestat)) continue;
			if (S_ISDIR(filestat.st_mode))         continue;

			DEBUG_LOG("reading file %s\n",filepath.c_str());
			Mat frame;
			frame = imread(filepath.c_str(),CV_LOAD_IMAGE_GRAYSCALE);
			resize(frame,frame,Size(64,128));
			if(!frame.data){
				ERROR_LOG("Cannot open image %s\n",filepath.c_str());
				continue;
			}else{
				DEBUG_STREAM << "image read!" << endl;
			}

			if(fe->setImage(frame,Size(0,0))){
				fe->generate_features();
				fe->write_features(argv[2]);
				DEBUG_STREAM << "Features : " << fe->features_.rows << endl;
			}else{
				DEBUG_STREAM << "setImage Failed" << endl;
			}
		}
		closedir(dp);
//		out_file.close();
		return 0;
}

int test_features(int argc, char **argv){
	if(argc < 2){
		cout << "usage is : " << argv[0] << " path\\to\\image" << endl;
		return -1;
	}
//	cout << "Image name : " << argv[1] << endl;
	Mat img = imread(argv[1],CV_LOAD_IMAGE_GRAYSCALE);
//	cout << "image size : " << img.cols << "x" << img.rows << endl;
	feat::HOGEvaluator fe; //= new feat::HOGEvaluator();
	resize(img,img,Size(64,64));
	if(fe.setImage(img,Size(0,0))){
		fe.generate_features();
		fe.write_features("features.csv");
		cout << "Features : " << fe.features_.cols << endl;
	}else{
		cout << "setImage Failed" << endl;
	}
	cout << "exiting..." << endl;
	exit(0);
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
int testFile(){
	std::streambuf *psbuf, *backup;
	std::ofstream filestr;
	filestr.open("test.csv");

	backup = std::cout.rdbuf();     // back up cout's streambuf

	psbuf = filestr.rdbuf();        // get file's streambuf
	std::cout.rdbuf(psbuf);         // assign streambuf to cout

	std::cout << "This is written to the file";

	std::cout.rdbuf(backup);        // restore cout's original streambuf

	filestr.close();

	return 0;
}
