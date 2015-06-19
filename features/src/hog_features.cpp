//============================================================================
// Name        : hog_features.cpp
// Author      : Daiem Ali
// Version     :
// Copyright   : Your copyright notice
// Description :
//============================================================================

#include <iostream>
#include <tgmath.h>

#include "hog_features.h"

//#define NDEBUG true

using namespace std;
using namespace feat;

// ------------------------------------------------------------
// ------------------------ HOGParams ------------------------
// ------------------------------------------------------------

HOGParams::HOGParams() {
	block_count_x_ = 3;
	block_count_y_ = 3;
	bin_count_ = 9;
}

HOGParams::HOGParams(uint_t block_count, uint_t bin_count) {
	block_count_x_ = block_count_y_ = block_count;
	bin_count_ = bin_count;
}

HOGParams::HOGParams(uint_t block_count_x, uint_t block_count_y,
uint_t bin_count) {
	block_count_x_ = block_count_x;
	block_count_y_ = block_count_y;
	bin_count_ = bin_count;
}

HOGParams::~HOGParams() {
}

ostream& HOGParams::operator<<(ostream& os) const {
	os << "PARAMS!" << endl;
//	os << "Bin Count : " << this->bin_count_ << endl
//			<< "Blocks along x-axis : " << this->block_count_x_<< endl
//			<< "Blocks along y-axis : " << this->block_count_y_ << endl;
	return os;
}

void HOGParams::printToStream(ostream& stream) {
	stream << "Bin Count : " << this->bin_count_ << endl
			<< "Blocks along x-axis : " << this->block_count_x_ << endl
			<< "Blocks along y-axis : " << this->block_count_y_ << endl;
}

// ------------------------------------------------------------
// -------------------- FeatureEvaluator ----------------------
// ------------------------------------------------------------

bool HOGEvaluator::setImage(Mat img, Size origWinSize) {

	cout << "inside set image!" << endl;
	printf("Image size:(%d,%d)\n", img.cols, img.rows);
	printf("origWinSize :(%d,%d)\n", origWinSize.width, origWinSize.height);
	cout << "HOG params :" << endl;
	params_->printToStream(cout);
	if (origWinSize.width <= 0 && origWinSize.height <= 0) {
		origWinSize.width = img.cols;
		origWinSize.height = img.rows;
	}
	if (img.rows <= params_->block_count_y_ * MIN_BLOCK_SIZE
			|| img.cols <= params_->block_count_x_ * MIN_BLOCK_SIZE
			|| origWinSize.height <= params_->block_count_y_ * MIN_BLOCK_SIZE
			|| origWinSize.width <= params_->block_count_x_ * MIN_BLOCK_SIZE) {
		return false;
	}
	if (origWinSize.width == 0 && origWinSize.height == 0) {
		win_size_->width = img.cols;
		win_size_->height = img.rows;
	} else {
		win_size_->width = origWinSize.width;
		win_size_->height = origWinSize.height;
	}
	if (!current_image_.empty()) {
		current_image_.deallocate();
	}
	current_image_ = img.clone();
	resetFeatures();
	return true;
}

int HOGEvaluator::getFeatureType() {
	return HOGEvaluator::HOG;
}

bool HOGEvaluator::setWindow(Point p) {
	if (p.x >= current_image_.cols || p.y >= current_image_.rows) {
		return false;
	}
	if (p.x + win_size_->width > current_image_.cols
			|| p.y + win_size_->height > current_image_.rows) {
		return false;
	}
	if (p.x + win_size_->width < params_->block_count_x_ * MIN_BLOCK_SIZE
			|| p.y + win_size_->height
					< params_->block_count_y_ * MIN_BLOCK_SIZE) {
		return false;
	}
	resetFeatures();
	win_pos_->x = p.x;
	win_pos_->y = p.y;
	return true;
}

// ------------------------------------------------------------
// -------------------- HOGEvaluator -------------------------
// ------------------------------------------------------------
HOGEvaluator::HOGEvaluator() :
		params_() {
	params_ = new HOGParams();
	win_size_ = new Size(0, 0);
	win_pos_ = new Point(0, 0);
//	features_ = Mat::zeros(params_->block_count_x_ * params_->block_count_y_ * params_->bin_count_, 1, CV_32F);
}

HOGEvaluator::~HOGEvaluator() {
	params_.delete_obj();
}

bool HOGEvaluator::resetFeatures() {
	if (!features_.empty()) {
		features_.deallocate();
		return true;
	}
	return false;
}
void HOGEvaluator::generate_features() {
	if (current_image_.empty()) {
		cout << "Error : No Image Found!" << endl;
	}
	uint_t nwin_x = params_->block_count_x_;
	uint_t nwin_y = params_->block_count_y_;
	uint_t bins = params_->bin_count_;
	uint_t L = win_size_->height;
	uint_t C = win_size_->width;
	Mat H = Mat::zeros(nwin_x * nwin_y * bins, 1, CV_32F);
	float m = sqrt((float) L / 2);
	Mat double_im;
	current_image_.assignTo(double_im, CV_32F);
	uint_t step_x = floor((float) C / ((float) nwin_x));
	uint_t step_y = floor((float) L / ((float) nwin_y));
	uint_t cont = 0;
	Mat hx = Mat::zeros(1, 3, CV_32F);
	hx.at<float>(0) = -1;
	hx.at<float>(2) = 1;
	Mat hy = (-hx.clone()).t();
	if (NDEBUG) {
		cout << "1D filters : " << endl;
		cout << "hx : " << hx << endl;
		cout << "hy : " << hy << endl;
		printf("Image size:(%d,%d)\n", current_image_.cols,
				current_image_.rows);
		cout << "step x : " << step_x << endl;
		cout << "step y : " << step_y << endl;
	}
	Point anchor = Point(-1, -1);
	double delta = 0.0;
	int ddepth = -1;
	Mat grad_xr;
	filter2D(double_im, grad_xr, ddepth, hx, anchor, delta, BORDER_DEFAULT);
	Mat grad_yu;
	filter2D(double_im, grad_yu, ddepth, hy, anchor, delta, BORDER_DEFAULT);
	Mat angles, magnit;
	cartToPolar(grad_xr, grad_yu, magnit, angles);
	imshow("Current Image", current_image_);
	for (int n = 0; n < nwin_y; n++) {
		for (int m = 0; m < nwin_x; m++) {
			cont += 1;
			Mat angles2, magnit2;
			angles2 = angles(Range(n * step_y, (n + 1) * step_y),
					Range(m * step_x, (m + 1) * step_x));
			magnit2 = magnit(Range(n * step_y, (n + 1) * step_y),
					Range(m * step_x, (m + 1) * step_x));
			Mat v_angles = angles2.clone().reshape(angles2.channels(), 1);
			Mat v_magnit = magnit2.clone().reshape(magnit2.channels(), 1);
			int K = v_angles.cols;
			int bin = 0;
			Mat part_H = H(Rect(Point(0, (cont - 1) * bins), Size(1, bins)));
			for (float ang_lim = -M_PI + 2 * M_PI / bins; ang_lim <= M_PI;
					ang_lim += 2 * M_PI / bins) {
				bin = bin + 1;
				for (int k = 0; k <= K; k++) {
					if (v_angles.at<float>(k) < ang_lim) {
						v_angles.at<float>(k) = 100;
						part_H.at<float>(bin) = part_H.at<float>(bin)
								+ v_magnit.at<float>(k);
					}
				}
				part_H = part_H / (norm(part_H, NORM_L1) + 0.01);
//				cout << "computed bins in part_H: " << part_H <<endl;
			}
		}
	}
//	cout << "computed features : " << H << endl;
	H.assignTo(features_, CV_32F);
#ifndef NDEBUG
	std::cout << "generating features..." << std::endl;
	std::cout << "HOGParams : " << std::endl;
	std::cout << "Row wise block count  : "<< block_count_y_ << std::endl;
	std::cout << "Coloum wise block count : "<< block_count_x_ << std::endl;
	std::cout << "Bins per block : " << bin_count_ << std::endl;
#endif
}

void HOGEvaluator::write_features(string path) {

}
