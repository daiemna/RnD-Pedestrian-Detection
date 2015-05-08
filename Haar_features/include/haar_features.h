//============================================================================
// Name        : haar_features.cpp
// Author      : Thomas Werner
// Version     :
// Copyright   : Your copyright notice
// Description :
//============================================================================

#ifndef HAARFEATURES_H_
#define HAARFEATURES_H_

#include <string>

#include "image.h"
#include "features.h"
#include "cv_utils.h"

#define MAX_RECTS 3

using namespace image;
using namespace std;

class HaarParams : public FeatureParams{
public:

	enum Mode{
		BASIC,		// classical features by Viola & Jones
		UPRIGHT,	// all upright features
		ALL			// all upright + all rotated
	};

	enum Weighting{
		STD,		// classical weights (+1,-1)
		FLDA,		// determine optimal weights using Fisher's LDA
		GA			// determine optimal weights using Genetic Algorithms
	};
	
	HaarParams();
	HaarParams(Mode mode, Weighting weighting);
	virtual ~HaarParams();

	Mode mode;
	Weighting weighting;
};

class HaarEvaluator : public FeatureEvaluator{
public:
	HaarEvaluator();
	virtual ~HaarEvaluator();

	float operator()(int feature_idx) const;

	void set_image(NIIMAGE& img);
	void generate_features();
	void write_features(string path);

protected:
	class Feature{
	public:
		Feature();
		Feature(int channel, bool tilted,
				int r0, int c0, int w0, int h0, float wt0,
				int r1, int c1, int w1, int h1, float wt1,
				int r2 = 0, int c2 = 0, int w2 = 0, int h2 = 0, float wt2 = 0.f);

		virtual ~Feature();

		float calc(const NIIMAGE& SAT, const NIIMAGE& SAT_tilted) const;
		void write(string path) const;

		bool tilted;
		int channel;
		
		struct Rect{
			int r, c, w, h;
			int channel;
			float weight;
			
		} rects[MAX_RECTS];
	};

	vector<Feature> features;
	
	NIIMAGE SAT, SAT_tilted;
};

// ------------------------------------------------------------
// ---------------- HaarEvaluator/Feature ---------------------
// ------------------------------------------------------------
inline float HaarEvaluator::Feature::calc(const NIIMAGE& SAT, const NIIMAGE& SAT_tilted) const{
	// const IMAGE& src = (tilted ? SAT_tilted : SAT);

	// TODO: tilted!!!
	const NIIMAGE& src = SAT;

	float ret = 0.f;

	for(int idx = 0; idx < MAX_RECTS; idx++){
		if(rects[idx].weight == 0.f) continue;
		// upper left
		int sum = src.color[channel][rects[idx].r][rects[idx].c];
		// lower right
		sum += src.color[channel][rects[idx].r+rects[idx].h][rects[idx].c+rects[idx].w];
		
		sum -= src.color[channel][rects[idx].r][rects[idx].c+rects[idx].w];
		sum -= src.color[channel][rects[idx].r+rects[idx].h][rects[idx].c];
			
		ret += rects[idx].weight*sum;
	}
	
	return ret;
}

// ------------------------------------------------------------
// -------------------- HaarEvaluator -------------------------
// ------------------------------------------------------------
inline float HaarEvaluator::operator()(int feature_idx) const{
	std::cout << "computing feature " << feature_idx << std::endl;
	return features[feature_idx].calc(SAT, SAT_tilted);
}

#endif 
