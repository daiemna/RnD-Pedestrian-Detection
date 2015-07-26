/*
 * FeatureEvaluator.h
 *
 *  Created on: Jul 17, 2015
 *      Author: dna
 */

#ifndef FEATUREEVALUATOR_H_
#define FEATUREEVALUATOR_H_

#include <opencv2/opencv.hpp>
#include <string.h>
#include <fstream>

namespace feat {

using namespace std;

class FeatureEvaluator {
public:
	FeatureEvaluator();
	virtual ~FeatureEvaluator();
	virtual bool setImage (const cv::Mat &, cv::Size origWinSize) = 0;
	virtual bool setWindow (cv::Point p) = 0;
	virtual int getFeatureType () = 0;
	virtual int getFeatureCount() const = 0;
	virtual void write_features(std::string path);
	cv::Mat features_;
protected:
	virtual void genrateFeatures() = 0;
};

} /* namespace feat */

#endif /* FEATUREEVALUATOR_H_ */
