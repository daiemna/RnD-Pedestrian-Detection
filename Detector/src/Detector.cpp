#include "Detector.hpp"

using namespace object_detection;

ObjectDetector::ObjectDetector(machineLearning::AbstractClassifier* learner,feat::FeatureEvaluator* featureExtractor,Size detection_window_size){
	learning_algorithm_ = learner;
	feature_extractor_ = featureExtractor;
	detection_window_size_.width = detection_window_size.width;
	detection_window_size_.height = detection_window_size.height;
//	threshold_ = 0.4;
}

void ObjectDetector::train(vector<Mat>& images, Mat& images_class){
	if(images.size() != images_class.rows){
		ERROR_LOG("images.size == images_class.rows failed in train!");
		return;
	}
	Mat features;
	extract_features(images,features);
	DEBUG_STREAM << "images : " << features.rows << "," << features.cols << endl;
	DEBUG_STREAM << "images class: " << images_class.rows << "," << images_class.cols << endl;
	learning_algorithm_->train(features,images_class);
}

void ObjectDetector::predict(vector<Mat>& images,Mat& predictions, Mat& probablities){
	Mat features;
	extract_features(images,features);
	learning_algorithm_->predict(features,predictions,probablities);
}

void ObjectDetector::extract_features(vector<Mat>& images,Mat& features){
	DEBUG_STREAM<<"inside extract_feature"<< endl;
	feature_extractor_->setImage(images[0],detection_window_size_);
	features = Mat::zeros(images.size(), feature_extractor_->getFeatureCount(), CV_32F);

	for(int i = 0; i < images.size(); i++){
		feature_extractor_->setImage(images[i],detection_window_size_);
		feature_extractor_->setWindow(Point(0,0));
		Mat feats = features(Range(i,i),Range(0,feature_extractor_->features_.rows));
		feature_extractor_->features_.assignTo(feats,CV_32F);
	}
	DEBUG_STREAM << "extraction done!" << endl;
}
