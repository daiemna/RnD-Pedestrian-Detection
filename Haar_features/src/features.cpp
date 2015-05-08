//============================================================================
// Name        : features.cpp
// Author      : Thomas Werner
// Version     :
// Copyright   : Your copyright notice
// Description :
//============================================================================

#include <iostream>
#include <string>

#include "features.h"

using namespace std;
using namespace image;

// ------------------------------------------------------------
// -------------------------- Params --------------------------
// ------------------------------------------------------------

Params::Params(){
	name = "PARAMETER PROTOTYPE";
}

Params::~Params(){}

bool Params::write(string path) const{
	cout << "Writing " << name << " to " << path << endl;
	return true;
}

bool Params::read(string path){
	cout << "Reading " << name << " from " << path << endl;
	return true;
}

// ------------------------------------------------------------
// ----------------------- FeatureParams ----------------------
// ------------------------------------------------------------

FeatureParams::FeatureParams(){
	name = "FEATURE PARAMETERS";
}

FeatureParams::~FeatureParams(){}

// ------------------------------------------------------------
// -------------------- FeatureEvaluator ----------------------
// ------------------------------------------------------------

FeatureEvaluator::FeatureEvaluator(){
	n_features = 0;
}

FeatureEvaluator::~FeatureEvaluator(){}

void FeatureEvaluator::init(FeatureParams* params, int win_w, int win_h){
	this->params = params;
	this->win_w = win_w;
	this->win_h = win_h;

	// generate_features(); // bullshit?
}

int FeatureEvaluator::get_num_features() const{
	return n_features;
}

void FeatureEvaluator::set_image(NIIMAGE& img){
	this->img = img;
}
