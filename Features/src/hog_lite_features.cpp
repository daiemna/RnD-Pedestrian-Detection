//============================================================================
// Name        : hog_features.cpp
// Author      : Daiem Ali
// Version     :
// Copyright   : Your copyright notice
// Description :
//============================================================================

#include <iostream>
#include <tgmath.h>
#include <fstream>
#include "hog_lite_features.h"

using namespace std;
using namespace feat;

// ------------------------------------------------------------
// ------------------------ HOGLiteParams ------------------------
// ------------------------------------------------------------

HOGLiteParams::HOGLiteParams(): HOGParams(),
		pixel_per_cell_(4,4),
		magnitude_threshold_(0.25){
	bin_count_ = BIN_COUNT;

}

HOGLiteParams::~HOGLiteParams() {
}

Size HOGLiteParams::getPixelPerCell(){
	return this->pixel_per_cell_;
}

void HOGLiteParams::printToStream(ostream& stream) {
	stream << "Bin Count : " << bin_count_ << endl
		   << "Blocks along x-axis : " << cell_count_x_ << endl
		   << "Blocks along y-axis : " << cell_count_y_ << endl;
}

// ------------------------------------------------------------
// -------------------- FeatureEvaluator ----------------------
// ------------------------------------------------------------

int HOGLiteEvaluator::getFeatureType() {
	return HOGLiteEvaluator::HOGLite;
}

// ------------------------------------------------------------
// -------------------- HOGLiteEvaluator -------------------------
// ------------------------------------------------------------
HOGLiteEvaluator::HOGLiteEvaluator(){
	params_ = new HOGLiteParams();
	win_size_ = new Size(0, 0);
	win_pos_ = new Point(0, 0);
	imageDecompressed_ = false;
}

HOGLiteEvaluator::~HOGLiteEvaluator() {
	params_.delete_obj();
}

bool HOGLiteEvaluator::resetFeatures(){
	if (histogram_image_.data) {
		histogram_image_.release();
		return HOGEvaluator::resetFeatures();
	}
	return false;
}

bool HOGLiteEvaluator::setImage(Mat image,Size origWinSize){
	return (HOGEvaluator::setImage(image,origWinSize) &&
			genrateHistogramImage());
}

bool HOGLiteEvaluator::genrateHistogramImage(){
	if (current_image_.empty()) {
		 cout<< "Error : No Image Found!" << endl;
		return false;
	}
	Ptr<HOGLiteParams> params = params_;
	params_->cell_count_x_ = current_image_.rows/params->getPixelPerCell().height;
	params_->cell_count_y_ = current_image_.cols/params->getPixelPerCell().width;
	resetHistogramImage(Mat::zeros(1,params_->cell_count_y_*params_->cell_count_x_,CV_32S));
//	cout << "size histogram image : " << histogram_image_.rows <<" , " << histogram_image_.cols << endl;
	uint_t step_x = floor((float) win_size_->width / ((float) params_->cell_count_x_));
	uint_t step_y = floor((float) win_size_->height / ((float) params_->cell_count_y_));
	uint_t cont = 0;
	Mat hx = Mat::zeros(1, 2, CV_32F);
	hx.at<float>(0) = -1;
	hx.at<float>(1) = 1;
	Mat hy = (-hx.clone()).t();
	Point anchor = Point(-1, -1);
	double delta = 0.0;
	int ddepth = -1;
	Mat grad_x;
	filter2D(current_image_, grad_x, ddepth, hx, anchor, delta,
			BORDER_DEFAULT);
	Mat grad_y;
	filter2D(current_image_, grad_y, ddepth, hy, anchor, delta,
			BORDER_DEFAULT);
	Mat rect_angles;
	Mat orientations = (grad_y < 0)/255 *4 + (grad_x < 0)/255 *2 + (abs(grad_y) > abs(grad_x))/255 *1;
	Mat magnitudes = abs(grad_x) + abs(grad_y);
	double min = 0, max = 0;
	cv::minMaxLoc(magnitudes,&min,&max);
	double threshold = min + (max-min)*params->magnitude_threshold_;
//	cout << "Magnitude Mat  min, max: " << min <<" , " << max << endl;
//	cout << "Threshold : " << threshold << endl;
	magnitudes = magnitudes >= threshold;

//		namedWindow( "Display window", WINDOW_AUTOSIZE );
//		imshow( "Display window", magnitudes);
//		waitKey(0);

//		cv::minMaxLoc(magnitudes,&min,&max);
//		cout << "Magnitudes : " << magnitudes << endl;
	int histimage_index = 0;
	for(int x = 0; x + params->getPixelPerCell().width <= magnitudes.cols; x += params->getPixelPerCell().width){
		for(int y = 0; y + params->getPixelPerCell().height<= magnitudes.rows ; y += params->getPixelPerCell().height){

			Mat part_ori = orientations(Range(y,y+params->getPixelPerCell().height),Range(x,x+ params->getPixelPerCell().width))
					.clone()
					.reshape(1,1);
			Mat part_mag = magnitudes(Range(y,y+params->getPixelPerCell().height),Range(x,x+ params->getPixelPerCell().width))
					.clone()
					.reshape(1,1);
			SIZE32_MEMORY hist[BIN_COUNT] = {0,0,0,0,0,0,0,0};
//				cout << "part ori type : " << (part_ori.type() != CV_8U) << endl;
//				cout << "sizeof unsigned int : " << sizeof(SIZE32_MEMORY) * 8<<endl;
//			cout << "Mag : " << part_mag << endl;
//			cout << "Ori : " << part_ori << endl;
			for(int i = 0; i < part_mag.cols ; i++){
//					cout << "part_mag " << i << " : " << part_mag.at<char>(i) << endl;
				if(part_mag.at<char>(i) == char(255)){
//						cout << "i : " << part_ori.at<unsigned char>(i) << endl;
					hist[part_ori.at<unsigned char>(i)] += 1;
					if(hist[part_ori.at<unsigned char>(i)] == MAX_BIN_VALUE)
					{
						hist[part_ori.at<unsigned char>(i)] = MAX_BIN_VALUE -1;
						cout << "Over Flows Occurred!" << endl;
					}
				}
			}
			SIZE32_MEMORY hist_pixel = 0x00;
			for(int i = 0; i < BIN_COUNT;i++){
//					cout << "hist " << i << " : " << hist[i] << endl;
				hist_pixel = hist_pixel | (hist[i] << (i*BIN_BIT_COUNT));
			}
			histogram_image_.at<SIZE32_MEMORY>(histimage_index) = hist_pixel;
			++histimage_index;
//			printf("hex value for hist pix : %x\n",hist_pixel);
		}
	}
//	cout << "cell_count_Y : " << params_->cell_count_y_ << endl;
	histogram_image_ = histogram_image_.reshape(1,params_->cell_count_y_);
//	cout << "histogram_image_ : " << histogram_image_.rows << " , " << histogram_image_.cols << endl;
//	cout << "histogram_image_ : " << histogram_image_ << endl;

//	namedWindow( "Display window", WINDOW_AUTOSIZE );
//	imshow( "Display window", histogram_image_);
//	waitKey(0);
	imageDecompressed_ = false;
	return true;
}

void HOGLiteEvaluator::decompressImage(){
	if (histogram_image_.empty()) {
		cout << "Error : No histogram Image Found!" << endl;
		return;
	}
	if(imageDecompressed_){
		return;
	}
//	cout << "decompressImage" << endl;
	Mat reformed_image = Mat::zeros(params_->cell_count_y_,params_->cell_count_x_*BIN_COUNT,CV_32F);
	for(int row = 0 ; row < params_->cell_count_y_; row++){
		int reformed_cols = 0;
		for(int col = 0; col < params_->cell_count_x_; col++){
			SIZE32_MEMORY hist_pix = histogram_image_.at<SIZE32_MEMORY>(Point(col,row));
			Mat part_hist = reformed_image(Rect(reformed_cols,row,BIN_COUNT,1));
			bool firstNZ = false;
			bool secondNZ = false;
			int bin_ind_15 = -1;
			for(int bin_ind = 0; bin_ind < BIN_COUNT; bin_ind++){
				SIZE32_MEMORY mask = 0x0000000f;
				mask = mask << bin_ind*BIN_BIT_COUNT;
				part_hist.at<float>(bin_ind) = float((hist_pix & mask) >> bin_ind*BIN_BIT_COUNT);
				if(firstNZ && part_hist.at<float>(bin_ind) > 0){
					secondNZ = true;
				}
				if(part_hist.at<float>(bin_ind) > 0){
					firstNZ = true;
				}
				if(part_hist.at<float>(bin_ind) == MAX_BIN_VALUE -1){
					bin_ind_15 = bin_ind;
				}
			}
//			cout << "Bins are : " << part_hist << endl;
			if(secondNZ && bin_ind_15 > -1){
				part_hist.at<float>(bin_ind_15) = MAX_BIN_VALUE;
				cout << "Over Flows Resolved!" << endl;
			}
			reformed_cols += BIN_COUNT;
		}
	}
	resetHistogramImage(reformed_image.clone());
	imageDecompressed_ = true;
//	cout << "End decompressImage" << endl;
}

void HOGLiteEvaluator::generate_features() {
 	if (current_image_.empty()) {
 		cout << "Error : No Image Found!" << endl;
 		return;
 	}
 	decompressImage();
 	HOGEvaluator::genrateFeatures();
}
