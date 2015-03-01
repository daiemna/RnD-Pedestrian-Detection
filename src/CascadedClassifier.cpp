/*
 * CascadedClassifier.cpp
 *
 *  Created on: Mar 1, 2015
 *      Author: Daiem
 */

#include <CascadedClassifier.hpp>

using namespace ml;

CascadedClassifier::CascadedClassifier(unsigned int cascadeCount,
		CvStatModel classifier) {
	cascade_list = new vector(cascadeCount);
	//TODO: build the constructor
//	for(CvStatModel)

}
CascadedClassifier::~CascadedClassifier() {
	while (!cascade_list.empty()) {
		cascade_list.pop_back();
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
		int tflag, const Mat& var_idx, const Mat& sample_idx) {
	return false;
}
float CascadedClassifier::predict(const Mat& sample) const {
	return -1.0;
}
