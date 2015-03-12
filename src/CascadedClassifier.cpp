/*
 * CascadedClassifier.cpp
 *
 *  Created on: Mar 1, 2015
 *      Author: Daiem
 */

#include <CascadedClassifier.hpp>

using namespace ml;

CascadedClassifier::CascadedClassifier(unsigned int cascadeCount) {
	cascade_list = new vector<simple_classifier>();
	for (int i = 0; i < cascadeCount; i++) {
		cascade_list.push_back(simple_classifier());
	}

}
CascadedClassifier::~CascadedClassifier() {
	while (!cascade_list.empty()) {
		simple_classifier cascade = cascade_list.pop_back();
		delete cascade;
	}
	delete cascade_list;
}
void CascadedClassifier::clear() {
}
void CascadedClassifier::save(const char* filename, const char* name) {
}
void CascadedClassifier::load(const char* filename, const char* name) {
}
void CascadedClassifier::write(CvFileStorage* storage, const char* name) {
}
void CascadedClassifier::read(CvFileStorage* storage, CvFileNode* node) {
}
bool CascadedClassifier::train(const Mat& train_data, const Mat& responses,
		const Mat& var_idx, const Mat& sample_idx) {
	int sample_count = train_data.rows;
	Mat cur_weight_vector(sample_count, 1, CV_32F, Scalar(1 / sample_count));
	Mat pre_weight_vector = cur_weight_vector.clone();
	double minVal = -2, maxVal = -2;
	minMaxIdx(responses, &minVal, &maxVal);
	Mat response_vector;
	if (minVal == 0) {
		response_vector = responses - 1;
	} else if (minVal == -1) {
		response_vector = responses;
	} else {
		throw "Illegal response values";
	}

	Mat layers(3, 1, CV_32S);
	layers[0] = sample_count; //number of inputs
	layers[1] = 1; //number of hidden neuron
	layers[2] = 1; //number of output neuron
	for (int i = 0; i < cascade_list.size(); i++) {
		Mat pre_weight_vector = cur_weight_vector.clone();
		cascade_list[i].create(layers, CvANN_MLP::SIGMOID_SYM, 1, 1);
		cascade_list[i].train(train_data, response_vector, cur_weight_vector);
		//updating current weight vector.
		//TODO: Calculate pseudo error
		//TODO: Update Weights
//		cur_weight_vector =
	}
	return false;
}
float CascadedClassifier::predict(const Mat& sample) const {
	return -1.0;
}
