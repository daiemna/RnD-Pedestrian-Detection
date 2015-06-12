//============================================================================
// Name        : cv_utils.cpp
// Author      : Thomas Werner
// Version     :
// Copyright   : Your copyright notice
// Description :
//============================================================================

#ifndef CV_UTILS_H_
#define CV_UTILS_H_

#include "image.h"

#include <cstring>

using namespace image;
using namespace std;

template <class PIXTYPE>
class NIMAGE : public Image<PIXTYPE>{
public:

	NIMAGE(){
		this->InitDefaults();
	}
	
	NIMAGE(int rows, int cols, int channels){
		
		this->InitDefaults();
		
		if((rows <= 0) || (cols <= 0)){
			return;
		}
		
		int n;
		PIXTYPE *h, **hh;
		
		this->cdim = channels;	
		this->xdim = cols;
		this->ydim = rows;
		
		this->pixmap = new PIXTYPE[this->cdim*this->xdim*this->ydim];
		
		this->gray = new PIXTYPE*[this->cdim*this->ydim];
		for(n = 0, h = this->pixmap; n < this->cdim*this->ydim; ++n, h += this->xdim){
			this->gray[n] = h;
		}

		this->color = new PIXTYPE**[this->cdim+1];
		for(n = 0, hh = this->gray; n < this->cdim; ++n, hh += this->ydim){
			this->color[n] = hh;
		}		
	};

	void set_channel_at_once(PIXTYPE* data, int ch){
		PIXTYPE *dst = this->color[ch][0];
		size_t n_bytes = this->ydim*this->xdim*sizeof(PIXTYPE);
		memcpy(dst, data, n_bytes);
	}


	void set_channel_rowwise(PIXTYPE* data, int ch){
		for(int r = 0; r < this->ydim; r++){
			PIXTYPE* dst = this->color[ch][r];
			
			memcpy(dst, data+r*this->xdim, this->xdim*sizeof(PIXTYPE));
		}
	}

	virtual ~NIMAGE(){};
};

typedef NIMAGE<int> NIIMAGE;
typedef NIMAGE<float> NFIMAGE;

template <class PIXTYPE>
void compute_integral_image(Image<PIXTYPE>& src, Image<PIXTYPE>& dst){

	if(src.colorspace != GRAYSCALE){
		ErrorImg("Src must be grayscale");
	}

	if(dst.xdim != src.xdim || dst.ydim != src.ydim){
		ErrorImg("Src and dst must have the same size");
	}

	if(src.cdim != dst.cdim){
		ErrorImg("Src and dst must have the same number of channels");
	}		
	
	// copy upper left corner
	dst.gray[0][0] = src.gray[0][0];

	// compute first row of dst image
	for(int c = 1; c < src.xdim; c++){
		dst.gray[0][c] = src.gray[0][c] + dst.gray[0][c-1];
	}

	// compute rest
	for(int r = 1; r < src.ydim; r++){
		PIXTYPE* src_row = src.gray[r];
		PIXTYPE* dst_row_prev = dst.gray[r-1];
		PIXTYPE* dst_row = dst.gray[r];

		// compute first column
		dst_row[0] = dst_row_prev[0] + src_row[0];

		for(int c = 1; c < src.xdim; c++){
			dst_row[c] = dst_row[c-1] + dst_row_prev[c] - dst_row_prev[c-1] + src_row[c];
		}
	}
}

template <class PIXTYPE>
void compute_integral_image(NIMAGE<PIXTYPE>& src, NIMAGE<PIXTYPE>& dst){

	if(src.colorspace != GRAYSCALE){
		ErrorImg("Src must be grayscale");
	}

	if(dst.xdim != src.xdim || dst.ydim != src.ydim){
		ErrorImg("Src and dst must have the same size");
	}
	
	// copy upper left corner
	for(int ch = 0; ch < src.cdim; ch++){
		dst.color[ch][0][0] = src.color[ch][0][0];
	}

	// compute first row of dst image
	for(int ch = 0; ch < src.cdim; ch++){
		for(int c = 1; c < src.xdim; c++){
			dst.color[ch][0][c] = src.color[ch][0][c] + dst.color[ch][0][c-1];
		}
	}

	// compute rest
	for(int ch = 0; ch < src.cdim; ch++){	
		for(int r = 1; r < src.ydim; r++){
			PIXTYPE* src_row = src.color[ch][r];
			PIXTYPE* dst_row_prev = dst.color[ch][r-1];
			PIXTYPE* dst_row = dst.color[ch][r];

			// compute first column
			dst_row[0] = dst_row_prev[0] + src_row[0];

			for(int c = 1; c < src.xdim; c++){
				dst_row[c] = dst_row[c-1] + dst_row_prev[c] - dst_row_prev[c-1] + src_row[c];
			}
		}
	}
}


#endif
