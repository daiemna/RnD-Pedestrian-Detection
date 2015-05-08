//============================================================================
// Name        : main.cpp
// Author      : Thomas Werner
// Version     :
// Copyright   : Your copyright notice
// Description :
//============================================================================

#include <iostream>
#include <cstring>

// imagelib
#include "image.h"
#include "png_io.h"

using namespace std;
using namespace image;

int main(int argc, char* argv[]) {
	IMAGE* img = LoadPNG("a.png");
	
	cout << img->xdim << " " << img->ydim << endl;
	
	return 0;
}

