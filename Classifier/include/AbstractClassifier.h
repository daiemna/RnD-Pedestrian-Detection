/*
 * AbstractClassifier.h
 *
 *  Created on: Jul 17, 2015
 *      Author: dna
 */

#ifndef CLASSIFIER_INCLUDE_ABSTRACTCLASSIFIER_H_
#define CLASSIFIER_INCLUDE_ABSTRACTCLASSIFIER_H_

//#include <opencv2/opencv.hpp>
#include <opencv2/ml/ml.hpp>

using namespace cv;

namespace machineLearning {

class AbstractClassifier: public CvStatModel{
public:
	AbstractClassifier();
	virtual ~AbstractClassifier();
	virtual bool train(const Mat& train_data, const Mat& responses, const Mat& var_idx =
				Mat(), const Mat& sample_idx = Mat()) = 0;
	virtual float predict(const Mat& samples,Mat& predictions,Mat& probabilities) const = 0;
};

} /* namespace feat */

#endif /* CLASSIFIER_INCLUDE_ABSTRACTCLASSIFIER_H_ */
