//CascadedClassifier.hpp

#ifndef CASCADED_CLASSIFIER_H
#define CASCADED_CLASSIFIER_H

#include <opencv2/opencv.hpp>
#include <math.h>
#include "debug.h"
#include "AbstractClassifier.h"


using namespace cv;
using namespace std;

#define simple_classifier CvDTree

namespace machineLearning {


/******************************************************************************\
*                          Cascaded Classifier                                *
 \******************************************************************************/

// Cascaded Classifier
class CascadedClassifier: public machineLearning::AbstractClassifier {
public:
	CascadedClassifier(int cascadeCount);
	~CascadedClassifier();
	void clear();
	void save(const char* filename, const char* name = 0);
	void load(const char* filename, const char* name = 0);
	void write(CvFileStorage* storage, const char* name);
	void read(CvFileStorage* storage, CvFileNode* node);
	bool train(const Mat& train_data, const Mat& responses, const Mat& var_idx =
			Mat(), const Mat& sample_idx = Mat());
	float predict(const Mat& samples,Mat& predictions,Mat& probabilities) const;
private:
//		unsigned int cascadeCount;
	vector<simple_classifier*> cascade_list_;
	int cascade_count_;
	Mat feature_indices_;
	Mat sample_indices_;
	float threshold_;

};

}

namespace cv {
template<typename T>
bool isInsideMat(T number,Mat mat){
	if(mat.channels() != 1){
		throw "ERROR : Mat is a multiple channel";
	}
	Mat row_mat = mat.reshape(1,1).clone();
	for(int i = 0;i < row_mat.cols; i++ ){
		if(row_mat.at<T>(i) == number){
			return true;
		}
	}
	return false;
}

}  // namespace cv

#endif // CASCADED_CLASSIFIER_H
