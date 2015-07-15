//============================================================================
// Name        : hog_features.cpp
// Author      : Daiem Ali
// Version     :
// Copyright   : Your copyright notice
// Description :
//============================================================================

#ifndef HOGLITEFEATURES_H_
#define HOGLITEFEATURES_H_

#include <string>
#include <algorithm>
#include <stdint.h>
#include <opencv2/opencv.hpp>
#include "hog_features.h"

using namespace std;
using namespace cv;
#ifndef uint_t
	#define uint_t int
#endif

#ifndef SIZE32_MEMORY
	#define SIZE32_MEMORY unsigned int
#endif

#ifndef MAX_BIN_VALUE
	#define MAX_BIN_VALUE 16
#endif

#ifndef BIN_COUNT
	#define BIN_COUNT 8
#endif

#ifndef BIN_BIT_COUNT
	#define BIN_BIT_COUNT 4
#endif

#ifndef NDEBUG
	#define NDEBUG true
#endif

namespace feat{

	class HOGLiteParams : public feat::HOGParams{
	public:

		HOGLiteParams();
		virtual ~HOGLiteParams();

		void printToStream(ostream&);
		Size getPixelPerCell();
		float magnitude_threshold_;
	protected:
		Size pixel_per_cell_;
	};
	
	class HOGLiteEvaluator : public HOGEvaluator{
	public:
		enum{HOGLite=3};
		HOGLiteEvaluator();
		virtual ~HOGLiteEvaluator();

		//Methods From FeatureEvaluator
		int getFeatureType();
		bool setImage(Mat,Size);
		void generate_features();
	protected:
		bool genrateHistogramImage();
		void decompressImage();
		bool resetFeatures();

	private:
//		Ptr<HOGLiteParams> params_;
		bool imageDecompressed_;
	};

}// end namespace feat
#endif 
