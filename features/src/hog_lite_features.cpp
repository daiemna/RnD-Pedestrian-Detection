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
}

HOGLiteEvaluator::~HOGLiteEvaluator() {
	params_.delete_obj();
}

bool HOGLiteEvaluator::setImage(Mat image,Size origWinSize){
	return (HOGEvaluator::setImage(image,origWinSize) &&
			compressImage());
}

bool HOGLiteEvaluator::compressImage(){
	if (current_image_.empty()) {
		cout << "Error : No Image Found!" << endl;
		return false;
	}
	params_->cell_count_x_ = current_image_.rows/params_->getPixelPerCell().height;
	params_->cell_count_y_ = current_image_.cols/params_->getPixelPerCell().width;
	histogram_image_ = Mat::zeros(1,params_->cell_count_y_*params_->cell_count_x_,CV_32S);
	cout << "size histogram image : " << histogram_image_.rows <<" , " << histogram_image_.cols << endl;
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
	double threshold = min + (max-min)*params_->magnitude_threshold_;
	cout << "Magnitude Mat  min, max: " << min <<" , " << max << endl;
	cout << "Threshold : " << threshold << endl;
	magnitudes = magnitudes >= threshold;

//		namedWindow( "Display window", WINDOW_AUTOSIZE );
//		imshow( "Display window", magnitudes);
//		waitKey(0);

//		cv::minMaxLoc(magnitudes,&min,&max);
//		cout << "Magnitudes : " << magnitudes << endl;
	int histimage_index = 0;
	for(int x = 0; x + params_->getPixelPerCell().width <= magnitudes.cols; x += params_->getPixelPerCell().width){
		for(int y = 0; y + params_->getPixelPerCell().height<= magnitudes.rows ; y += params_->getPixelPerCell().height){

			Mat part_ori = orientations(Range(y,y+params_->getPixelPerCell().height),Range(x,x+ params_->getPixelPerCell().width))
					.clone()
					.reshape(1,1);
			Mat part_mag = magnitudes(Range(y,y+params_->getPixelPerCell().height),Range(x,x+ params_->getPixelPerCell().width))
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
	histogram_image_ = histogram_image_.reshape(1,params_->cell_count_y_);
//	cout << "histogram_image_ : " << histogram_image_.rows << " , " << histogram_image_.cols << endl;
//	cout << "histogram_image_ : " << histogram_image_ << endl;

//	namedWindow( "Display window", WINDOW_AUTOSIZE );
//	imshow( "Display window", histogram_image_);
//	waitKey(0);
	return true;
}

void HOGLiteEvaluator::generate_features() {
// 	if (current_image_.empty()) {
// 		cout << "Error : No Image Found!" << endl;
// 		return;
// 	}
// 	Mat H = Mat::zeros(
// 			params_->cell_count_x_ * params_->cell_count_y_
// 					* params_->bin_count_, 1, CV_32F);
// 	uint_t step_x = floor(
// 			(float) win_size_->width / ((float) params_->cell_count_x_));
// 	uint_t step_y = floor(
// 			(float) win_size_->height / ((float) params_->cell_count_y_));
// 	uint_t cont = 0;
// 	Mat hx = Mat::zeros(1, 2, CV_32F);
// 	hx.at<float>(0) = -1;
// 	hx.at<float>(1) = 1;
// 	Mat hy = (-hx.clone()).t();
// 	Point anchor = Point(-1, -1);
// 	double delta = 0.0;
// 	int ddepth = -1;
// 	Mat grad_xr;
// 	filter2D(current_image_, grad_xr, ddepth, hx, anchor, delta,
// 			BORDER_DEFAULT);
// 	Mat grad_yu;
// 	filter2D(current_image_, grad_yu, ddepth, hy, anchor, delta,
// 			BORDER_DEFAULT);
// 	Mat angles, magnit, mask, rect_angles;
// 	cartToPolar(grad_xr, grad_yu, magnit, angles, false);

// 	//correcting range b/w 0 to 180
// 	inRange(angles,M_PI+(params_->round_off_*M_PI/180), 2*M_PI,mask);
// 	add(angles, -M_PI,angles,mask);

// 	for (int n = 0; n < params_->cell_count_y_; n++) {
// 		for (int m = 0; m < params_->cell_count_x_; m++) {
// 			cont += 1;
// 			Mat angles2, magnit2;
// 			angles2 = angles(Range(n * step_y, (n + 1) * step_y),
// 					Range(m * step_x, (m + 1) * step_x));
// 			magnit2 = magnit(Range(n * step_y, (n + 1) * step_y),
// 					Range(m * step_x, (m + 1) * step_x));
// 			Mat v_angles = angles2.clone().reshape(angles2.channels(), 1);
// 			Mat v_magnit = magnit2.clone().reshape(magnit2.channels(), 1);

// 			int K = v_angles.cols;
// 			int bin = 0;
// 			Mat part_H = H(Rect(Point(0, (cont - 1) * params_->bin_count_),
// 							Size(1, params_->bin_count_)));
// 			for (float ang_lim = M_PI / params_->bin_count_;
// 					ang_lim <= M_PI;
// 					ang_lim += M_PI / params_->bin_count_,
// 					bin = bin + 1) {
// 				Mat sum_these;
// 				double lower_lim = ang_lim-M_PI / params_->bin_count_;
// 				double upper_lim = ang_lim-(M_PI/180);
// 				if(bin == params_->bin_count_ - 1)
// 					upper_lim = ang_lim;

// 				inRange(v_angles,lower_lim, upper_lim,sum_these);

// 				multiply(v_magnit,(sum_these/255),sum_these,1,CV_32F);
// 				part_H.at<float>(bin) = sum(sum_these)[0];
// 			}
// 		}
// 	}
// 	H = H.reshape(H.channels(), params_->cell_count_y_);
// 	Mat H_norm = Mat::zeros(H.rows, H.cols, H.type());
// 	for (int y = 0; y < H.rows; y += params_->block_stride_.height) {
// 		for (int x = 0; x < H.cols;
// 				x += params_->block_stride_.width * params_->bin_count_) {
// 			int width = params_->bin_count_ * params_->cell_per_block_.width;
// 			int height = params_->cell_per_block_.height;
// 			if (y + height > H.rows) {
// 				height = H.rows - y;
// 			}
// 			if (x + width > H.cols) {
// 				width = H.cols - x;
// 			}
// 			Mat part_H = H(Rect(Point(x, y), Size(width, height))).clone();
// //			H_norm(Rect(Point(x,y),Size(width, height))) = part_H / sqrt(pow(cv::sum(part_H)[0],2) + params_->round_off_);
// 			H_norm(Rect(Point(x, y), Size(width, height))) = part_H
// 					/ sqrt(pow(cv::norm(part_H, cv::NORM_L2), 2) + pow(params_->round_off_, 2));
// 		}
// 	}
// 	H_norm.reshape(H_norm.channels(),
// 			params_->cell_count_x_ * params_->cell_count_y_
// 					* params_->bin_count_).assignTo(features_, CV_32F);
}
