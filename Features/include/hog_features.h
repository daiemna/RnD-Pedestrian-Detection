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
#include <algorithm>
#include <stdint.h>
#include <opencv2/opencv.hpp>
//#include <bitset>

#include "FeatureEvaluator.h"

using namespace std;
using namespace cv;
#ifndef uint_t
	#define uint_t int
#endif

#ifndef NDEBUG
	#define NDEBUG true
#endif
namespace feat{

	#define MIN_BLOCK_SIZE 3

	class HOGParams{
	public:

		HOGParams();
		HOGParams(Size, uint_t);
		HOGParams(uint_t,uint_t, uint_t);
		virtual ~HOGParams();

		ostream& operator<<(ostream& os)const;
		void printToStream(ostream&);

		Size pixel_per_cell_;
		Size cell_per_block_;
		Size block_stride_;
		uint_t cell_count_x_;
		uint_t cell_count_y_;
		uint_t bin_count_;
		float round_off_;
		bool image_norm_;
	};
	
	class HOGEvaluator : public feat::FeatureEvaluator{
	public:
		enum{HOG=2};
		HOGEvaluator();
		virtual ~HOGEvaluator();

		//Methods From FeatureEvaluator
		bool setImage(const Mat&,Size);
		bool setWindow(Point);
		int getFeatureType();
		int getFeatureCount() const;
		float operator()(int feature_idx) const;
//		void write_features(string path);
		Mat features_;

	protected:
		Mat histogram_image_;
		bool resetFeatures();
		Mat current_image_;
		Ptr<Size> win_size_;
		Ptr<Point> win_pos_;
		bool replaceImage(Mat img);
		void resetHistogramImage(Mat);
		bool virtual genrateHistogramImage();
		void virtual genrateFeatures();
		Ptr<HOGParams> params_;
//	private:
//		Ptr<HOGParams> params_;
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

inline int feat::HOGEvaluator::getFeatureCount()const{
	return params_->bin_count_ * params_->cell_count_x_ * params_->cell_count_y_;
}

#endif 
