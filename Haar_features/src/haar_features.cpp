//============================================================================
// Name        : haar_features.cpp
// Author      : Thomas Werner
// Version     :
// Copyright   : Your copyright notice
// Description :
//============================================================================

#include <iostream>

#include "haar_features.h"

using namespace std;
using namespace image;

// ------------------------------------------------------------
// ------------------------ HaarParams ------------------------
// ------------------------------------------------------------

HaarParams::HaarParams(){
	name = "HAAR PARAMETERS";
	type = FeatureParams::HAAR;
	dim = 1;

	mode = BASIC;
	weighting = STD;
}

HaarParams::HaarParams(Mode mode, Weighting weighting){
	name = "HAAR PARAMETERS";
	type = FeatureParams::HAAR;
	dim = 1;

	this->mode = mode;
	this->weighting = weighting;
}

HaarParams::~HaarParams(){}

// ------------------------------------------------------------
// ---------------- HaarEvaluator/Feature ---------------------
// ------------------------------------------------------------
HaarEvaluator::Feature::Feature(){
	tilted = false;
	channel = 0;

	for(int idx = 0; idx < MAX_RECTS; idx++){
		rects[idx].r = 0;
		rects[idx].c = 0;
		rects[idx].w = 0;
		rects[idx].h = 0;
		rects[idx].weight = 0.f;
	}	
}

HaarEvaluator::Feature::Feature(int channel, bool tilted,
								int r0, int c0, int w0, int h0, float wt0,
								int r1, int c1, int w1, int h1, float wt1,
								int r2, int c2, int w2, int h2, float wt2){
	this->tilted = tilted;
	this-> channel = channel;

	rects[0].r = r0;
	rects[0].c = c0;
	rects[0].w = w0;
	rects[0].h = h0;
 	rects[0].weight = wt0;

	rects[1].r = r1;
	rects[1].c = c1;
	rects[1].w = w1;
	rects[1].h = h1;
 	rects[1].weight = wt1;

	rects[2].r = r2;
	rects[2].c = c2;
	rects[2].w = w2;
	rects[2].h = h2;
 	rects[2].weight = wt2;	
}
								

HaarEvaluator::Feature::~Feature(){}

// ------------------------------------------------------------
// -------------------- HaarEvaluator -------------------------
// ------------------------------------------------------------
HaarEvaluator::HaarEvaluator(){}

HaarEvaluator::~HaarEvaluator(){}

void HaarEvaluator::set_image(NIIMAGE& img){
	FeatureEvaluator::set_image(img);
	
	SAT = NIIMAGE(img.ydim, img.xdim, img.cdim);
	compute_integral_image(img, SAT);
}

void HaarEvaluator::generate_features(){

	HaarParams::Mode mode = ((HaarParams*)params)->mode;

	for(int r = 0; r < win_h; r++){
		for(int c = 0; c < win_w; c++){
			for(int dr = 1; dr <= win_h; dr++){
				for(int dc = 1; dc <= win_w; dc++){					
					// typ 1a
					if((r+2*dr <= win_h) && (c+dc <= win_w)){
						for(int ch = 0; ch < img.cdim; ch++){
							features.emplace_back(ch, false,
												  r, c, 2*dr, dc, -1.f,
												  r+dr, c, dr, dc, 2.f);
						}
					}

					// typ 1b
					if((r+dr <= win_h) && (c+2*dc <= win_w)){
						for(int ch = 0; ch < img.cdim; ch++){
							features.emplace_back(ch, false,
												  r, c, dr, 2*dc, -1.f,
												  r, c+dc, dr, dc, 2.f);
						}
					}						

					// typ 2a
					if((r+3*dr <= win_h) && (c+dc <= win_w)){
						for(int ch = 0; ch < img.cdim; ch++){
							features.emplace_back(ch, false,
												  r, c, 3*dr, dc, -1.f,
												  r+dr, c, dr, dc, 3.f);
						}
					}

					// typ 2c
					if((r+dr <= win_h) && (c+3*dc <= win_w)){
						for(int ch = 0; ch < img.cdim; ch++){
							features.emplace_back(ch, false,
												  r, c, dr, 3*dc, -1.f,
												  r, c+dc, dr, dc, 3.f);
						}
					}

					// typ 2e
					if((r+2*dr <= win_h) && (c+2*dc <= win_w)){
						for(int ch = 0; ch < img.cdim; ch++){
							features.emplace_back(ch, false,
												  r, c, 2*dr, 2*dc, -1.f,
												  r, c, dr, dc, 2.f,
												  r+dr, c+dc, dr, dc, 2.f);
						}
					}

					if(mode != HaarParams::BASIC){
						// typ 2b
						if((r+4*dr <= win_h) && (c+dc <= win_w)){
							for(int ch = 0; ch < img.cdim; ch++){
								features.emplace_back(ch, false,
													  r, c, 4*dr, dc, -1.f,
													  r+dr, c, 2*dr, dc, 2.f);
							}
						}

						// typ 2d
						if((r+dr <= win_h) && (c+4*dc <= win_w)){
							for(int ch = 0; ch < img.cdim; ch++){
								features.emplace_back(ch, false,
													  r, c, dr, 4*dc, -1.f,
													  r, c+dc, dr, 2*dc, 2.f);
							}
						}

						// tye 3a, center-surround
						if((r+3*dr <= win_h) && (c+3*dc <= win_w)){
							for(int ch = 0; ch < img.cdim; ch++){
								features.emplace_back(ch, false,
													  r, c, 3*dr, 3*dc, -1.f,
													  r+dr, c+dc, dr, dc, 9.f);
							}
						}
					}

					if(mode == HaarParams::ALL){		
						// typ 1c
						if((r+2*dr <= win_h) && (c+2*dr+dc <= win_w) && (r-dc >= 0)){

						}

						// typ 1d
						if((r+dr <= win_h) && (c+dr+2*dc <= win_w) && (r-2*dc >= 0)){

						}

						// type 2f
						if((r+3*dr <= win_h) && (c+3*dr+dc <= win_w) && (r-dc >= 0)){

						}

						// typ 2g
						if((r+dr <= win_h) && (c+dr+3*dc <= win_w) && (r-3*dc >= 0)){

						}

						// typ 2f
						if((r+4*dr <= win_h) && (c+4*dr+dc <= win_w) && (r-dc >= 0)){

						}
						
						// typ 2h
						if((r+dr <= win_h) && (c+dr+4*dc <= win_w) && (r-4*dc >= 0)){

						}

						// typ 3b
						if((r+dr <= win_h) && (c+dr+4*dc <= win_w) && (r-4*dc >= 0)){

						}
					}					
				}
			}
		}
	}
	n_features = (int)features.size();

#ifndef NDEBUG
	std::cout << "generating features..."  << std::endl;
	std::cout << "win_h: "  << win_h << std::endl;
	std::cout << "win_w: "  << win_w << std::endl;
	std::cout << "cdim: "  << img.cdim << std::endl;
	std::cout << "mode: " << mode << std::endl;
	std::cout << "n_features: " << n_features << endl;
#endif
}

void HaarEvaluator::write_features(string path){
	
}
