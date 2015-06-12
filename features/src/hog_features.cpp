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

using namespace std;
// using namespace image;

// ------------------------------------------------------------
// ------------------------ HOGParams ------------------------
// ------------------------------------------------------------

HOGParams::HOGParams(){
	block_count_x_ = 3;
	block_count_y_ = 3;
	bin_count_ = 9;
}

HOGParams::HOGParams(uint_t block_count, uint_t bin_count){
	block_count_x_ = block_count_y_ = block_count;
	bin_count_ = bin_count;
}

HOGParams::HOGParams(uint_t block_count_x,uint_t block_count_y, uint_t bin_count){
	block_count_x_ = block_count_x;
	block_count_y_ = block_count_y;
	bin_count_ = bin_count;
}

HOGParams::~HOGParams(){
}

// Size HOGParams::getBlockSize(uint_t imageWidth,uint_t imageHeight){
	
// 	return s;
// }

// uint_t HOGParams::getBinCount(){
// 	return -1;
// }

// ------------------------------------------------------------
// -------------------- FeatureEvaluator ----------------------
// ------------------------------------------------------------


bool HOGEvaluator::set_image(Mat img,Size origWinSize = Size(0,0)){
	if(img.rows <= params_->block_count_y_*MIN_BLOCK_SIZE || img.cols <= params_->block_count_x_*MIN_BLOCK_SIZE ||
	   origWinSize.height <= params_->block_count_y_*MIN_BLOCK_SIZE || origWinSize.width <= params_->block_count_x_*MIN_BLOCK_SIZE){
		return false;
	}
	if(origWinSize.width == 0 && origWinSize.height == 0){
		win_size_->width = img.cols;
		win_size_->height = img.rows;
	}else{
		win_size_->width = origWinSize.width;
		win_size_->height = origWinSize.height;
	}
	current_image_ = img;
	resetFeatures();
	return true;
}

int HOGEvaluator::getFeatureType(){
	return HOGEvaluator::HOG;
}

bool HOGEvaluator::setWindow(Point p){
	if(p.x >= current_image_.cols || p.y >= current_image_.rows){
		return false;
	}
	if(p.x + win_size_->width > current_image_.cols ||
	   p.y + win_size_->height > current_image_.rows){
		return false;
	}
	if(p.x + win_size_->width < params_->block_count_x_ * MIN_BLOCK_SIZE ||
	   p.y + win_size_->height < params_->block_count_y_ * MIN_BLOCK_SIZE){
		return false;
	}
	resetFeatures();
	return true;
}

// ------------------------------------------------------------
// -------------------- HOGEvaluator -------------------------
// ------------------------------------------------------------
HOGEvaluator::HOGEvaluator():params_(){
	params_ = new HOGParams();
	win_size_ = new Size(0,0);
}

HOGEvaluator::~HOGEvaluator(){
	params_.delete_obj();
}

bool HOGEvaluator::resetFeatures(){
	if(!features_.empty()){
			features_.delete_obj();
			return true;
	}
	return false;
}
void HOGEvaluator::generate_features(){

	uint_t nwin_x=params_->block_count_x_;
	uint_t nwin_y=params_->block_count_y_;
	uint_t bins=params_->bin_count_;
	// [L,C]=size(Im);
	uint_t L = win_size_->height;
	uint_t C  = win_size_->width;
	// H=zeros(nwin_x*nwin_y*bins,1); % column vector with zeros
	Mat H = Mat::zeros(nwin_x*nwin_y*bins,1,CV_32F);
	// m=sqrt(L/2);
	float m = sqrt((float)L/2);
	// if C==1 % if num of columns==1
	//     Im=im_recover(Im,m,2*m);%verify the size of image, e.g. 25x50
	//     L=2*m;
	//     C=m;
	// end
	// Im=double(Im);
	// img_f.set_channel_at_once(SAT.pixmap,0); // Not sure about this
	// step_x=floor(C/(nwin_x+1));
	// TODO: Enable uint_t setp_x = floor((float)C/((float)nwin_x+1));
	// step_y=floor(L/(nwin_y+1));
	// TODO: Enable uint_t setp_y = floor((float)L/((float)nwin_y+1));
	// count = 0;
	uint_t cont=0;
	// hx = [-1,0,1];
	// Mat hx(1,3,1);
	// hx.set_channel_at_once({-1,0,1},0);
	// hy = -hx.T();
	// grad_xr = imfilter(double(Im),hx);
	// grad_yu = imfilter(double(Im),hy);
	// angles=atan2(grad_yu,grad_xr);
	// magnit=((grad_yu.^2)+(grad_xr.^2)).^.5;
	// for n=0:nwin_y-1
	//     for m=0:nwin_x-1
	//         cont=cont+1;
	//         angles2=angles(n*step_y+1:(n+2)*step_y,m*step_x+1:(m+2)*step_x); 
	//         magnit2=magnit(n*step_y+1:(n+2)*step_y,m*step_x+1:(m+2)*step_x);
	//         v_angles=angles2(:);    
	//         v_magnit=magnit2(:);
	//         K=max(size(v_angles));
	//         %assembling the histogram with 9 bins (range of 20 degrees per bin)
	//         bin=0;
	//         H2=zeros(bins,1);
	//         for ang_lim=-pi+2*pi/bins:2*pi/bins:pi
	//             bin=bin+1;
	//             for k=1:K
	//                 if v_angles(k)<ang_lim
	//                     v_angles(k)=100;
	//                     H2(bin)=H2(bin)+v_magnit(k);
	//                 end
	//             end
	//         end
	                
	//         H2=H2/(norm(H2)+0.01);        
	//         H((cont-1)*bins+1:cont*bins,1)=H2;
	//     end
	// end

#ifndef NDEBUG
	std::cout << "generating features..."  << std::endl;
	std::cout << "HOGParams : "  << std::endl;
	std::cout << "Row wise block count  : "<< block_count_y_ << std::endl;
	std::cout << "Coloum wise block count : "<< block_count_x_  << std::endl;
	std::cout << "Bins per block : "  << bin_count_ << std::endl;
#endif
}

void HOGEvaluator::write_features(string path){
	
}
