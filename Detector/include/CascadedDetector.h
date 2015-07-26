/*
 * CascadedDetector.h
 *
 *  Created on: Jul 24, 2015
 *      Author: dna
 */

#include <opencv2/opencv.hpp>
//#include <opencv2../apps/traincascade/cascadeclassifier.h>
#include <string.h>


#ifndef DETECTOR_SRC_CASCADEDDETECTOR_H_
#define DETECTOR_SRC_CASCADEDDETECTOR_H_

namespace object_detection {

class CascadedDetector {
public:
	CascadedDetector();
	virtual ~CascadedDetector();
//	CvCascadeClassifier classifier;
};

} /* namespace object_detection */

#endif /* DETECTOR_SRC_CASCADEDDETECTOR_H_ */
