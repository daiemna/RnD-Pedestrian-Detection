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
#define NDEBUG true
#endif

namespace feat{

#define MIN_BLOCK_SIZE 3

class HOGParams{
public:

	HOGParams();
	HOGParams(uint_t block_count, uint_t bin_count);
	HOGParams(uint_t block_count_x,uint_t block_count_y, uint_t bin_count);
	virtual ~HOGParams();

	ostream& operator<<(ostream& os)const;
	void printToStream(ostream&);

// 	Size getBlockSize(uint_t,uint_t);
// 	uint_t getBinCount();
// private:
	uint_t block_count_x_;
	uint_t block_count_y_;
	uint_t bin_count_;
};

class HOGEvaluator : public cv::FeatureEvaluator{
public:
	enum{HOG=2};
	HOGEvaluator();
	virtual ~HOGEvaluator();
	
	//Methods From FeatureEvaluator
	bool setImage(Mat,Size);
	bool setWindow(Point);
	int getFeatureType();

	float operator()(int feature_idx) const;
	void generate_features();
	void write_features(string path);

	Mat features_;
protected:

	bool resetFeatures();
	Ptr<HOGParams> params_;
	Mat current_image_;
	Ptr<Size> win_size_;
	Ptr<Point> win_pos_;
};
}// end namespace feat
// ------------------------------------------------------------
// -------------------- HOGEvaluator -------------------------
// ------------------------------------------------------------
inline float feat::HOGEvaluator::operator()(int feature_idx) const{
	if(features_.empty()){
		return -1.0;
	}
	return features_.at<float>(feature_idx);
}

#endif 
