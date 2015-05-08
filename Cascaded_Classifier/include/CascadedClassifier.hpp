//CascadedClassifier.hpp

#ifndef CASCADED_CLASSIFIER_HPP
#define CASCADED_CLASSIFIER_HPP

#include <opencv2/opencv.hpp>
#include <math.h>
#include "debug.hpp"


using namespace cv;
using namespace std;


namespace ml {

#define simple_classifier CvANN_MLP

/******************************************************************************\
*                          Cascaded Classifier                                *
 \******************************************************************************/

// Cascaded Classifier
class CascadedClassifier: public CvStatModel {
public:
	CascadedClassifier(unsigned int cascadeCount);
	~CascadedClassifier();
	void clear();
	void save(const char* filename, const char* name = 0);
	void load(const char* filename, const char* name = 0);
	void write(CvFileStorage* storage, const char* name);
	void read(CvFileStorage* storage, CvFileNode* node);
	bool train(const Mat& train_data, const Mat& responses, const Mat& var_idx =
			Mat(), const Mat& sample_idx = Mat());
	float predict(const Mat& samples,Mat& predictions) const;
private:
//		unsigned int cascadeCount;
	vector<simple_classifier> * cascade_list;
	Mat featureIndices;
	Mat sampleIndices;

};

}

#endif // CASCADED_CLASSIFIER_HPP