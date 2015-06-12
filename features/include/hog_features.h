//============================================================================
// Name        : hog_features.cpp
// Author      : Daiem Ali
// Version     :
// Copyright   : Your copyright notice
// Description :
//============================================================================

#ifndef HOGFEATURES_H_
#define HOGFEATURES_H_

#include <string>
#include <stdint.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
#define uint_t uint16_t

#ifndef NDEBUG
#define NDEBUG
#endif

#define MIN_BLOCK_SIZE 3

class HOGParams{
public:

	HOGParams();
	HOGParams(uint_t block_count, uint_t bin_count);
	HOGParams(uint_t block_count_x,uint_t block_count_y, uint_t bin_count);
	virtual ~HOGParams();
// 	Size getBlockSize(uint_t,uint_t);
// 	uint_t getBinCount();
// private:
	uint_t block_count_x_;
	uint_t block_count_y_;
	uint_t bin_count_;
};

class HOGEvaluator : public FeatureEvaluator{
public:
	enum{HOG=2};
	HOGEvaluator();
	virtual ~HOGEvaluator();
	
	//Methods From FeatureEvaluator
	bool set_image(Mat,Size);
	bool setWindow(Point);
	int getFeatureType();

	float operator()(int feature_idx) const;
	void generate_features();
	void write_features(string path);

protected:
	bool resetFeatures();
	Ptr<Mat> features_;
	Ptr<HOGParams> params_;
	Mat current_image_;
	Ptr<Size> win_size_;
};

// ------------------------------------------------------------
// -------------------- HOGEvaluator -------------------------
// ------------------------------------------------------------
inline float HOGEvaluator::operator()(int feature_idx) const{
	if(features_.empty()){
		return -1.0;
	}
	return features_->at<float>(feature_idx);
}

#endif 
