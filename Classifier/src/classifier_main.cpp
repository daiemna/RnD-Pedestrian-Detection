#include <opencv2/opencv.hpp>
#include <iostream>

#include "debug.h"
#include "CascadedClassifier.h"

using namespace std;
using namespace cv;
using namespace machineLearning;

int captureStream(int argc, char* argv[]);
int matrixTest(int argc, char* argv[]);
int cascadeTest(int argc, char* argv[]);
int mlp_test(int argc, char* argv[]);
int mat_test_sclice(int argc, char* argv[]);

int main(int argc, char* argv[]) {
//	matrixTest(argc,argv);
	return cascadeTest(argc, argv);
//	return mat_test_sclice(argc, argv);

}



int cascadeTest(int argc, char* argv[]){
	CascadedClassifier cascade_classifier(100);
	uint samples = 40;
	uint features = 4096;
	Mat train_data = Mat::zeros(samples,features,CV_32FC1);
	RNG rng = RNG();
//	Mat part = train_data(Rect(1,0,features-1,samples));
	rng.fill(train_data,RNG::UNIFORM,0,10);
	Mat res = Mat::zeros(Size(1,samples),CV_32FC1);
	rng.fill(res,RNG::UNIFORM,0,1);
	res = (res <= 0.5)/255;
	res.convertTo(res,CV_32F);
	DEBUG_STREAM << "train_data : " << train_data.rows <<", " << train_data.cols << endl;
	DEBUG_STREAM << "res : " << res.rows <<", " << res.cols << endl;
	if(cascade_classifier.train(train_data,res)){
		DEBUG_STREAM << "Training Complete!" << endl;
		rng.fill(train_data,RNG::UNIFORM,0,10);
		res = (res <= 0.5)/255;
		res.convertTo(res,CV_32F);

		Mat predictions,probs;
		cascade_classifier.predict(train_data,predictions,probs);
		DEBUG_STREAM << "predictions : " << predictions << endl;
		DEBUG_STREAM << "probs : " << probs<< endl;
	}
	return 0;
}

int mat_test_sclice(int argc, char* argv[]){
	Mat A(1,10,CV_8U),B;
	uchar data[10] = {1,2,3,4,5,6,7,8,9,10};
	A.data = data;
	int ex_ind = 0;
	hconcat(A.colRange(0,ex_ind),A.colRange(ex_ind+1,A.cols),B);
	cout << "A is : " << A << endl;
	cout << "A is : " << B << endl;
	return 0;
}



int mlp_test(int argc, char* argv[]){
	Mat train_data = Mat(5,10,CV_32FC1);
	RNG rng = RNG();
	rng.fill(train_data,RNG::NORMAL,rng.uniform(1,10),rng.uniform(1,10));
	Mat training_classes = Mat::zeros(Size(1,5),CV_32FC1);
	training_classes.row(0) = Scalar(1);
	training_classes.row(3) = Scalar(1);
	training_classes.row(2) = Scalar(1);

	Mat layers = Mat(3, 1, CV_32SC1);
	layers.row(0) = Scalar(train_data.cols);
	layers.row(1) = Scalar(3);
	layers.row(2) = Scalar(1);

	CvANN_MLP mlp;

	CvANN_MLP_TrainParams params;
	CvTermCriteria criteria;
	criteria.max_iter = 10000;
	criteria.epsilon  = 0.001;
	criteria.type     = CV_TERMCRIT_ITER + CV_TERMCRIT_EPS;
	params.train_method = CvANN_MLP_TrainParams::BACKPROP;
	params.bp_dw_scale  = 0.1;
	params.bp_moment_scale = 0.1;
	params.term_crit  = criteria;


	mlp.create(layers);
	DEBUG_STREAM << "MLP layers : " << endl << layers << endl;
	DEBUG_STREAM << "Training Data : " << train_data << endl;
	DEBUG_STREAM << "Expected Response : " << training_classes << endl;
	mlp.train(train_data,training_classes,Mat(),Mat(),params);
	return 0;
}

int matrixTest(int argc, char* argv[]){
	DEBUG_LOG("-------------------------- matrixTest() --------------------------\n");
	Mat weak_responses = Mat::zeros(Size(1,5),CV_32FC1);
//	res.row(0) = Scalar(1);
	weak_responses.row(4) = Scalar(1);
	weak_responses.row(1) = Scalar(1);
	DEBUG_LOG("Weak Response type : %d\n",weak_responses.type());
	double minVal  = -2.0,maxVal = -2.0;
	minMaxIdx(weak_responses, &minVal, &maxVal);
	DEBUG_LOG("before min, max = (%f,%f)\n",minVal,maxVal);
	if (minVal == 0.0) {
		DEBUG_LOG("Min Value is zero making it -1\n");
		DEBUG_STREAM << "Response : " << weak_responses << endl;
		Mat mask;
		inRange(weak_responses,Scalar(0),Scalar(0),mask);
		DEBUG_STREAM << "Make -1 Mask : " << mask << endl;
		weak_responses.setTo(Scalar(-1), mask);
	}
	minMaxIdx(weak_responses, &minVal, &maxVal);
	DEBUG_LOG("after min, max = (%f,%f)\n",minVal,maxVal);
	DEBUG_STREAM << "Response : " << weak_responses << endl;
	return 0;
}

int captureStream(int argc, char* argv[]) {
	VideoCapture cap(0); // open the default camera
	if (!cap.isOpened())  // check if we succeeded
		return -1;

	Mat edges;
	namedWindow("edges", 1);
	for (;;) {
		Mat frame;
		cap >> frame; // get a new frame from camera
		// cvtColor(frame, edges, CV_BGR2GRAY);
		GaussianBlur(frame, edges, Size(7, 7), 1.5, 1.5);
		// Canny(edges, edges, 0, 30, 3);
		imshow("edges", edges);
		if (waitKey(30) >= 0)
			break;
	}
	return 0;
}
