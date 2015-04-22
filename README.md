# RnD-Pedestrian-Detection

## Cascadded Classifier
 This is an open implimentation of Cascaded Classifier using [1].
### Functions:
1. BoostedCascade(CvStatModel classifier) **Done**;
2. ~BoostedCascade()**Done**;
void clear();
3. void save( const char* filename, const char* name=0 );
4. void load( const char* filename, const char* name=0 );
5. void write( CvFileStorage* storage, const char* name );
6. void read( CvFileStorage* storage, CvFileNode* node );
7. bool train(const Mat& train_data, const Mat& responses, int tflag = CV_COL_SAMPLE, const Mat& var_idx =0, const Mat& sample_idx = 0)**Done**;
8. float predict(const Mat& sample) const;


#References
[1] Lienhart, Rainer, Alexander Kuranov, and Vadim Pisarevsky. "Empirical analysis of detection cascades of boosted classifiers for rapid object detection." In Pattern Recognition, pp. 297-304. Springer Berlin Heidelberg, 2003.