#ifndef DETECTOR_HPP_
#define DETECTOR_HPP_

#include <opencv2/opencv.hpp>
#include "debug.h"
#include "FeatureEvaluator.h"
#include "AbstractClassifier.h"

using namespace cv;
using namespace std;
namespace object_detection{

class ObjectDetector{

public:
	ObjectDetector(machineLearning::AbstractClassifier*,feat::FeatureEvaluator*,Size);
	~ObjectDetector();
	void train(vector<Mat>&,Mat&);
	void predict(vector<Mat>&,Mat&,Mat&);
	void detect();

private:
	void extract_features(vector<Mat>&,Mat&);
	Ptr<machineLearning::AbstractClassifier> learning_algorithm_;
	Ptr<feat::FeatureEvaluator> feature_extractor_;
	Size detection_window_size_;
	Mat features_;
//	float threshold_;
};

}


#endif // DETECTOR_HPP_
