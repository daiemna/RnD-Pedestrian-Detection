#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <vector>
//#include <algorithm>

#include "Detector.hpp"
#include "debug.h"
#include "hog_lite_features.h"
#include "CascadedClassifier.h"

using namespace std;
using namespace feat;
using namespace object_detection;
using namespace machineLearning;

int read_image_dir(string,vector<Mat>&,int count=-1);
int classifiy_images(int argc, char** argv);

int main(int argc, char** argv){

	return classifiy_images(argc,argv);
}

int classifiy_images(int argc, char** argv){
	if(argc < 5){
		ERROR_LOG("Usage : %s path/to/train/positive path/to/train/negative path/to/test/positive path/to/test/negative",argv[0]);
	}
	int image_count;
	int count = 20;
	std::vector<Mat> train_pos_images;
	image_count = read_image_dir(argv[1],train_pos_images,count);
	DEBUG_STREAM << "train_pos_images count " << train_pos_images.size() << endl;
	Mat train_labels = Mat::ones(image_count,1,CV_32F);

	std::vector<Mat> train_neg_images;
	image_count = read_image_dir(argv[2],train_neg_images,count);
	DEBUG_STREAM << "train_neg_images count " << train_neg_images.size() << endl;
	Mat train_neg_labels = Mat::zeros(image_count,1,CV_32F);

	std::vector<Mat> train_images;
//	train_images.reserve((train_pos_images.size() + train_neg_images.size()));
	train_images.insert(train_images.end(), train_pos_images.begin(), train_pos_images.end());
	train_images.insert(train_images.end(), train_neg_images.begin(), train_neg_images.end());

	if(train_images.size() != train_neg_images.size() + train_pos_images.size()){
		DEBUG_STREAM<<"train_images.size() != train_neg_images.size() + train_pos_images.size()" << endl;
		return -1;
	}
	vconcat(train_labels,train_neg_labels,train_labels);
	DEBUG_STREAM << "***********************************************************" << endl << endl;
	std::vector<Mat> test_pos_images;
	image_count = read_image_dir(argv[3],test_pos_images,count);
	DEBUG_STREAM << "test_pos_images count " << test_pos_images.size() << endl;
	Mat test_labels = Mat::ones(image_count,1,CV_32F);

	std::vector<Mat> test_neg_images;
	image_count = read_image_dir(argv[4],test_neg_images,count);
	DEBUG_STREAM << "test_neg_images count " << test_neg_images.size() << endl;
	Mat test_neg_labels = Mat::zeros(image_count,1,CV_32F);

	std::vector<Mat> test_images;
	test_images.reserve((test_pos_images.size() + test_neg_images.size()));
	test_images.insert(test_images.end(), test_pos_images.begin(), test_pos_images.end());
	test_images.insert(test_images.end(), test_neg_images.begin(), test_neg_images.end());
	if(test_images.size() != test_neg_images.size() + test_pos_images.size()){
		DEBUG_STREAM<<"test_images.size() != test_neg_images.size() + test_pos_images.size()" << endl;
		return -1;
	}
	vconcat(test_labels,test_neg_labels,test_labels);
	DEBUG_STREAM << "***********************************************************" << endl << endl;
	HOGLiteEvaluator * hog_lite_evaluator = new HOGLiteEvaluator();
	machineLearning::CascadedClassifier * cascade = new CascadedClassifier(100);
	ObjectDetector * pedestrian_detector = new ObjectDetector(cascade,hog_lite_evaluator,Size(64,128));
	DEBUG_STREAM << "Constructed Pedestrian Detection!" << endl;
	pedestrian_detector->train(train_images,train_labels);
	DEBUG_STREAM << "Pedestrian Detection training done!" << endl;
	Mat predictions,probablities;
	pedestrian_detector->predict(test_images,predictions, probablities);
	DEBUG_STREAM << "Pedestrian Detection prediction done!" << endl;
	DEBUG_STREAM << "Probs : " << probablities << endl;
	DEBUG_STREAM << "Predictions : " << predictions<< endl;
	return 0;
}

int read_image_dir(string path,vector<Mat>& images,int count){
	local_dbg::redirect_outstream_file();
	string  delm = " ";
	DIR *dp,*odp;
	struct dirent *dirp;
	struct stat filestat;
	ofstream out_file;

	dp = opendir(path.c_str());
	if (dp == NULL){
		ERROR_LOG("Error(%d): opening %s\n",errno,path.c_str());
		return errno;
	}
	int image_count = 0;

	while((dirp = readdir( dp ))){
		string filepath = path + "/" + dirp->d_name;

		// If the file is a directory (or is in some way invalid) we'll skip it
		if (stat(filepath.c_str(), &filestat)) continue;
		if (S_ISDIR(filestat.st_mode))         continue;

//		DEBUG_STREAM << "reading file " << filepath.c_str() << endl;
		Mat frame;
		frame = imread(filepath.c_str(),CV_LOAD_IMAGE_GRAYSCALE);
		resize(frame,frame,Size(64,128));
		images.push_back(frame);
		image_count++;
		if(image_count == count){
			break;
		}
	}
	closedir(dp);
	local_dbg::redirect_outstream_stdout();
	return images.size();
}
