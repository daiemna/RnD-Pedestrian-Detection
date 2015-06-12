//============================================================================
// Name        : features.cpp
// Author      : Thomas Werner
// Version     :
// Copyright   : Your copyright notice
// Description :
//============================================================================

#ifndef FEATURES_H_
#define FEATURES_H_

#include <string>

#include "image.h"
#include "cv_utils.h"

using namespace image;
using namespace std;

namespace feat_discript{

class Params{
public:
	Params();
	virtual ~Params();

	virtual bool write(string path) const;
	virtual bool read(string path);
	
	string name;
};
	
class FeatureParams : public Params{
public:

	enum FeatureType{
		HAAR,
		// LBP,
		RHOG,
		// RHOG_LITE,
		// RHOG_LITE_PLUS,
		// CHOG,
		// EHOG, // entropy HOG
		// HIST_FEAT,
		// REGION_COVAR,
		// EDGELETS,
		// FDF, // four direction feature
		FEATURE_TYPE_COUNT
	};

	FeatureParams();
	virtual ~FeatureParams();

	FeatureType type;

	// dimensionality of the feature; e.g. 1 for HAAR
	int dim;
};

class FeatureEvaluator{
public:
	FeatureEvaluator();
	virtual ~FeatureEvaluator();

	virtual void init(FeatureParams* params, int win_w, int win_h);
	virtual void set_image(NIIMAGE& img);
	virtual float operator()(int feature_idx) const = 0;
	int get_num_features() const;

	virtual void generate_features() = 0;
	virtual void write_features(string path) = 0;

protected:
	int win_w, win_h;
	int n_features;
	NIIMAGE img;

	FeatureParams* params;
};

}// end feat_discript
#endif 
