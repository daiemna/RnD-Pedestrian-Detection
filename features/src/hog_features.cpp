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
#include "hog_features.h"

using namespace std;
using namespace feat;

// ------------------------------------------------------------
// ------------------------ HOGParams ------------------------
// ------------------------------------------------------------

HOGParams::HOGParams() :
		pixel_per_cell_(6, 6),
		cell_per_block_(1, 1),
		block_stride_(1, 1),
		image_norm_(true) {
	cell_count_x_ = 0;
	cell_count_y_ = 0;
	bin_count_ = 9;
	round_off_ = 1.0e-5;
}

HOGParams::HOGParams(Size cell_size, uint_t bin_count) :
		pixel_per_cell_(cell_size.width, cell_size.height), cell_per_block_(1,
				1) {
	cell_count_x_ = cell_count_y_ = 0;
	bin_count_ = bin_count;
	round_off_ = 0.01;
	image_norm_ = true;
}

HOGParams::HOGParams(uint_t block_count_x, uint_t block_count_y,
uint_t bin_count) {
	cell_count_x_ = block_count_x;
	cell_count_y_ = block_count_y;
	bin_count_ = bin_count;
	round_off_ = 0.01;
	image_norm_ = true;
}

HOGParams::~HOGParams() {
}

ostream& HOGParams::operator<<(ostream& os) const {
	os << "PARAMS!" << endl;
	return os;
}

void HOGParams::printToStream(ostream& stream) {
	stream << "Bin Count : " << this->bin_count_ << endl
			<< "Blocks along x-axis : " << this->cell_count_x_ << endl
			<< "Blocks along y-axis : " << this->cell_count_y_ << endl;
}

// ------------------------------------------------------------
// -------------------- FeatureEvaluator ----------------------
// ------------------------------------------------------------

bool HOGEvaluator::setImage(Mat img, Size origWinSize) {
	if (origWinSize.width <= 0 && origWinSize.height <= 0) {
		origWinSize.width = img.cols;
		origWinSize.height = img.rows;
	}
	if (origWinSize.width == 0 && origWinSize.height == 0) {
		win_size_->width = img.cols;
		win_size_->height = img.rows;
	} else {
		win_size_->width = origWinSize.width;
		win_size_->height = origWinSize.height;
	}
	params_->cell_count_x_ = win_size_->width / params_->pixel_per_cell_.width;
	params_->cell_count_y_ = win_size_->height
			/ params_->pixel_per_cell_.height;
	replaceImage(img);
	if (params_->image_norm_) {
		cv::sqrt(current_image_, current_image_);
	}
	resetFeatures();
	return true;
}

bool HOGEvaluator::replaceImage(Mat img) {
	if (current_image_.data) {
		current_image_.release();
	}
	img.assignTo(current_image_, CV_32F);
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
	if (p.x + win_size_->width
			< params_->cell_count_x_ * params_->pixel_per_cell_.width
			|| p.y + win_size_->height
					< params_->cell_count_y_
							* params_->pixel_per_cell_.height) {
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
}

HOGEvaluator::~HOGEvaluator() {
	params_.delete_obj();
}

bool HOGEvaluator::resetFeatures() {
	if (features_.data) {
		features_.release();
		return true;
	}
	return false;
}
void HOGEvaluator::generate_features() {
	if (current_image_.empty()) {
		cout << "Error : No Image Found!" << endl;
		return;
	}
	Mat H = Mat::zeros(
			params_->cell_count_x_ * params_->cell_count_y_
					* params_->bin_count_, 1, CV_32F);
	uint_t step_x = floor(
			(float) win_size_->width / ((float) params_->cell_count_x_));
	uint_t step_y = floor(
			(float) win_size_->height / ((float) params_->cell_count_y_));
	uint_t cont = 0;
	Mat hx = Mat::zeros(1, 2, CV_32F);
	hx.at<float>(0) = -1;
	hx.at<float>(1) = 1;
	Mat hy = (-hx.clone()).t();
	Point anchor = Point(-1, -1);
	double delta = 0.0;
	int ddepth = -1;
	Mat grad_xr;
	filter2D(current_image_, grad_xr, ddepth, hx, anchor, delta,
			BORDER_DEFAULT);
	Mat grad_yu;
	filter2D(current_image_, grad_yu, ddepth, hy, anchor, delta,
			BORDER_DEFAULT);
	Mat angles, magnit, mask, rect_angles;
	cartToPolar(grad_xr, grad_yu, magnit, angles, false);

	//correcting range b/w 0 to 180
	inRange(angles,M_PI+(params_->round_off_*M_PI/180), 2*M_PI,mask);
	add(angles, -M_PI,angles,mask);

	for (int n = 0; n < params_->cell_count_y_; n++) {
		for (int m = 0; m < params_->cell_count_x_; m++) {
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
			Mat part_H = H(Rect(Point(0, (cont - 1) * params_->bin_count_),
							Size(1, params_->bin_count_)));
			for (float ang_lim = M_PI / params_->bin_count_;
					ang_lim <= M_PI;
					ang_lim += M_PI / params_->bin_count_,
					bin = bin + 1) {
				Mat sum_these;
				double lower_lim = ang_lim-M_PI / params_->bin_count_;
				double upper_lim = ang_lim-(M_PI/180);
				if(bin == params_->bin_count_ - 1)
					upper_lim = ang_lim;

				inRange(v_angles,lower_lim, upper_lim,sum_these);

				multiply(v_magnit,(sum_these/255),sum_these,1,CV_32F);
				part_H.at<float>(bin) = sum(sum_these)[0];
			}
		}
	}
	H = H.reshape(H.channels(), params_->cell_count_y_);
	Mat H_norm = Mat::zeros(H.rows, H.cols, H.type());
	for (int y = 0; y < H.rows; y += params_->block_stride_.height) {
		for (int x = 0; x < H.cols;
				x += params_->block_stride_.width * params_->bin_count_) {
			int width = params_->bin_count_ * params_->cell_per_block_.width;
			int height = params_->cell_per_block_.height;
			if (y + height > H.rows) {
				height = H.rows - y;
			}
			if (x + width > H.cols) {
				width = H.cols - x;
			}
			Mat part_H = H(Rect(Point(x, y), Size(width, height))).clone();
//			H_norm(Rect(Point(x,y),Size(width, height))) = part_H / sqrt(pow(cv::sum(part_H)[0],2) + params_->round_off_);
			H_norm(Rect(Point(x, y), Size(width, height))) = part_H
					/ sqrt(pow(cv::norm(part_H, cv::NORM_L2), 2) + pow(params_->round_off_, 2));
		}
	}
	H_norm.reshape(H_norm.channels(),
			params_->cell_count_x_ * params_->cell_count_y_
					* params_->bin_count_).assignTo(features_, CV_32F);
}

void HOGEvaluator::write_features(string path) {
	std::ofstream out_file(path.c_str(), ofstream::out | ofstream::app);
	if (out_file.is_open()) {
		ostringstream trimer;
		trimer << features_;
		string the_mat = trimer.str();
		the_mat.erase(0, 1);
		the_mat.erase(the_mat.size() - 1, 1);
//		cout << "Features : " << the_mat <<"*"<< endl;
		the_mat.erase(std::remove(the_mat.begin(), the_mat.end(), ' '),
				the_mat.end());
		out_file << the_mat << endl;
	}
	out_file.close();
}
