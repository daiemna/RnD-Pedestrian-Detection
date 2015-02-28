//CascadedClassifier.hpp

#ifndef CASCADED_CLASSIFIER_HPP
#define CASCADED_CLASSIFIER_HPP

#include <opencv2/opencv.hpp>

using namespace cv;

namespace ml{

	class BoostedCascade : public CvStatModel {
	public:
		BoostedCascade(CvStatModel classifier);
	    ~BoostedCascade();
	    void clear();
	    void save( const char* filename, const char* name=0 );
	    void load( const char* filename, const char* name=0 );
	    void write( CvFileStorage* storage, const char* name );
	    void read( CvFileStorage* storage, CvFileNode* node );
	    bool train(const Mat& train_data, const Mat& responses, int tflag = CV_COL_SAMPLE, const Mat& var_idx =0, const Mat& sample_idx = 0);
	    float predict(const Mat& sample) const;
	private:
		unsigned int cascadeCount;
		CvStatModel cascadeClassifier;
		Mat featureIndices;
		Mat sampleIndices;

	};

}


#endif // CASCADED_CLASSIFIER_HPP
