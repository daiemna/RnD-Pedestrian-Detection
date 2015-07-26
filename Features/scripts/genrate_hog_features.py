#! /usr/bin/python
# load mnist and train svm.


import pylab, numpy, skimage, csv, sys 
import os, scipy.misc
# from skimage.feature import hog
from HOG_features import computeHOG
from os import walk
from cv2 import imread , CV_LOAD_IMAGE_GRAYSCALE, resize
# from scipy.misc.pilutil import imresize

def main(argv):
	if len(argv) < 3:
		print(argv[0] + " [path/to/image] [feature_file.csv]");
		return;
	bins = 9;
	ppc = 6;
	cpb = 1;
	with open(argv[2], 'wb') as csvfile:
		test_writer = csv.writer(csvfile, delimiter=';');
		for (dirpath, dirnames, filenames) in walk(argv[1]):
# 			print(len(filenames));
			for ind,filename in enumerate(filenames):
				image_path = dirpath+"/"+filename;
				print("reading image "+str(ind)+" : ")
				print(image_path);
				img = imread(image_path, flags=CV_LOAD_IMAGE_GRAYSCALE);
				img = resize(img,(64,128));
				feats = computeHOG(img, orientations=bins, pixels_per_cell=(ppc, ppc),cells_per_block=(cpb, cpb), visualise=False, normalise=False);
				print("features  : " + str(len(feats)));
				test_writer.writerow(feats);
# 				print(len(feats))
# 				break;

if __name__ == "__main__":
    main(sys.argv)