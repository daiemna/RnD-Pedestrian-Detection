/*
 * CascadedClassifier.cpp
 *
 *  Created on: Mar 1, 2015
 *      Author: Daiem
 */

#include <CascadedClassifier.h>
#include "debug.h"

using namespace machineLearning;

CascadedClassifier::CascadedClassifier(int cascadeCount) {
	cascade_count_ = cascadeCount;
	threshold_ = 0.4;
}
CascadedClassifier::~CascadedClassifier() {
	while (!cascade_list_.empty()) {
		cascade_list_[cascade_list_.size()-1]->~simple_classifier();
		cascade_list_.pop_back();
	}
}
void CascadedClassifier::clear() {
}
void CascadedClassifier::save(const char* filename, const char* name) {
}
void CascadedClassifier::load(const char* filename, const char* name) {
}
void CascadedClassifier::write(CvFileStorage* storage, const char* name) {
}
void CascadedClassifier::read(CvFileStorage* storage, CvFileNode* node) {
}
bool CascadedClassifier::train(const Mat& train_data, const Mat& responses,
		const Mat& var_idx, const Mat& sample_idx) {
	if(train_data.cols < cascade_count_){
		ERROR_LOG("Error: there are more cascades than features.");
		return false;
	}
//	local_dbg::redirect_outstream_file();
	int sample_count = train_data.rows;
	Mat cur_weight_vector(sample_count, 1, CV_32F, Scalar(1.0 / sample_count));
	Mat pre_weight_vector = cur_weight_vector.clone();
	Mat shifted_responses;
	shifted_responses = responses.clone();
	double minVal  = -2.0,maxVal = -2.0;
	minMaxIdx(shifted_responses, &minVal, &maxVal);
	if (minVal == 0.0) {
		Mat mask;
		inRange(shifted_responses,Scalar(0),Scalar(0),mask);
		shifted_responses.setTo(Scalar(-1), mask);
	}
	minMaxIdx(shifted_responses, &minVal, &maxVal);
	DEBUG_STREAM << "shifted_responses : " << shifted_responses << endl;

	vector<simple_classifier*> classifier_list(train_data.cols);
	DEBUG_STREAM << "Cascade List size: " << classifier_list.size() << endl;

	for(int i = 0; i < train_data.cols; i++) {
		Mat layers = Mat(3, 1,CV_32SC1);
		layers.row(0) = Scalar(1); //number of inputs
		layers.row(1) = Scalar(2); //number of hidden neuron
		layers.row(2) = Scalar(1); //number of output neuron
		classifier_list[i] = new simple_classifier();
//		classifier_list[i]->create(layers);
	}
//	cascade_list_.resize(0);
	int feat_red = round((double)train_data.cols/(double)cascade_count_);
	Mat err_array(1,train_data.cols,CV_32F);
	for(int j = 0; j < train_data.cols; j++){
		Mat baged_data;
//		if(j == 0){
//			baged_data = train_data.colRange(1,train_data.cols);
//		}else{
//			hconcat(train_data.colRange(0,j),train_data.colRange(j+1,train_data.cols),baged_data);
//		}
		baged_data = train_data.colRange(j,j+1);
//		for (int i = 0; i < classifier_list.size(); i++) {
		Mat pre_weight_vector = cur_weight_vector.clone();
		Mat weak_responses = Mat::zeros(cur_weight_vector.rows,cur_weight_vector.cols,CV_64F);
		DEBUG_STREAM << "----------------------------------MLP Created " << j <<"!------------------------------" << endl;
		DEBUG_STREAM << "baged data : " << baged_data.rows << "," << baged_data.cols << endl;
		DEBUG_STREAM << "responses : " << responses.rows << "," << responses.cols << endl;
		DEBUG_STREAM << "cur_weight_vector : " << cur_weight_vector << endl;
		CvDTreeParams params = CvDTreeParams(20, // max depth
			    1,// min sample count
			    0.5, // regression accuracy: N/A here
			    true, // compute surrogate split, no missing data
			    2, // max number of categories (use sub-optimal algorithm for larger numbers)
			    0, // the number of cross-validation folds
			    false, // use 1SE rule => smaller tree
			    false, // throw away the pruned tree branches
				(float*)cur_weight_vector.data // the array of priors
		);

		local_dbg::print_float_((float*)cur_weight_vector.data,cur_weight_vector.rows);
		DEBUG_STREAM << "Params ready" << endl;
		bool train = classifier_list.at(j)->train(baged_data,CV_ROW_SAMPLE, responses,Mat(),Mat(),Mat(),Mat(), params);
		DEBUG_STREAM << "training done! baged data type : " << (baged_data.type() == CV_32F) << endl;
		DEBUG_STREAM << "training returned : " << train << endl;
		if(!train){
			throw "ERROR: Training Failed!";
		}

		for(int i= 0; i < baged_data.rows; i++){
			DEBUG_STREAM << "Prediction Class : " << classifier_list.at(j)->predict(baged_data.rowRange(i,i+1),Mat(),false)->value << endl;
			weak_responses.at<double>(i) = classifier_list.at(j)->predict(baged_data.rowRange(i,i+1),Mat(),false)->value;
		}

//		Mat nd_risk;
//		= Mat::zeros(baged_data.rows,1,CV_64F);
//		nd_risk.data = (uchar *)&(node->value);
//		weak_responses = (weak_responses < threshold_)/255;
		weak_responses.convertTo(weak_responses,CV_32F);
		DEBUG_STREAM << "weak responses: " << weak_responses <<endl;
		DEBUG_STREAM << "previous responses: " << pre_weight_vector<<endl;
		Mat sumW(1, 1, CV_64F);
		double sum_weights = sum(pre_weight_vector).val[0];
		DEBUG_STREAM << "sum_weights :" << sum_weights << endl;

		//updating current weight vector.
		/* NOTE: following comments are copied from opencv
		 * file: opencv/apps/traincscade/boost.cpp
		 */

		// Discrete AdaBoost:
		//   weak_eval[i] =(f(x_i)) is in {-1,1}
		//   err = sum(w_i*(f(x_i) != y_i))/sum(w_i)
		//   C = log((1-err)/err)
		//   w_i *= exp(C*(f(x_i) != y_i))

		minVal  = -2.0,maxVal = -2.0;
		minMaxIdx(weak_responses, &minVal, &maxVal);
		if (minVal == 0.0) {
			Mat mask;
			inRange(weak_responses,Scalar(0),Scalar(0),mask);
			weak_responses.setTo(Scalar(-1.0), mask);
		}
		minMaxIdx(weak_responses, &minVal, &maxVal);
		DEBUG_STREAM << "weak_responses : " << weak_responses << endl;

		DEBUG_STREAM << "weak_responses != shifted_responses : " << (weak_responses != shifted_responses)/255 << endl;
		DEBUG_STREAM << "pre_weight_vector : " << pre_weight_vector << endl;
		Mat conditional((weak_responses != shifted_responses)/255);
		DEBUG_STREAM << "conditional : " << conditional << endl;
		DEBUG_STREAM << "conditional type : " << conditional.type() << endl;
		conditional.convertTo(conditional,CV_32F);
		DEBUG_STREAM << "after conversion conditional type : " << conditional.type() << endl;
		DEBUG_STREAM << "pre_weight_vector type : " << pre_weight_vector.type() << endl;
		DEBUG_STREAM << "conditional size : (" << conditional.rows << "," << conditional.cols <<
				") and pre_weight_vector size : ("<< pre_weight_vector.rows << "," << pre_weight_vector.cols << ")"<< endl;
		double total_err = sum(pre_weight_vector.mul(conditional)).val[0];
		DEBUG_STREAM << "total_err : " << total_err << endl;
		float err = total_err / sum_weights;
		DEBUG_STREAM << "err : " << err << endl;
		double C = log((1 - err) / err);
		if(err == 1)
			C = 0.0;
		if(err == 0)
			C = 999999;
		DEBUG_STREAM << "C : " << C << endl;
		err_array.at<float>(j) = err;
		cv::exp(conditional.mul(Scalar(C)),cur_weight_vector);
		DEBUG_STREAM << "cur_weight_vector : " << cur_weight_vector << endl;
		// Real AdaBoost:
		//   weak_eval[i] = f(x_i) = 0.5*log(p(x_i)/(1-p(x_i))), p(x_i)=P(y=1|x_i)
		//   w_i *= exp(-y_i*f(x_i))

		// LogitBoost:
		//   weak_eval[i] = f(x_i) in [-z_max,z_max]
		//   sum_response = F(x_i).
		//   F(x_i) += 0.5*f(x_i)
		//   p(x_i) = exp(F(x_i))/(exp(F(x_i)) + exp(-F(x_i))=1/(1+exp(-2*F(x_i)))
		//   reuse weak_eval: weak_eval[i] <- p(x_i)
		//   w_i = p(x_i)*1(1 - p(x_i))
		//   z_i = ((y_i+1)/2 - p(x_i))/(p(x_i)*(1 - p(x_i)))
		//   store z_i to the data->data_root as the new target responses

		// Gentle AdaBoost:
		//   weak_eval[i] = f(x_i) in [-1,1]
		//   w_i *= exp(-y_i*f(x_i))

		// normalize weights

		sum_weights = sum(cur_weight_vector).val[0];
		DEBUG_STREAM << "sum_weights : " << sum_weights << endl;
		cur_weight_vector = cur_weight_vector.mul(Scalar(1.0 / sum_weights));
		DEBUG_STREAM << "cur_weight_vector after norm : " << cur_weight_vector << endl;
//		}
//		DEBUG_STREAM << "error  " << min_err << " index is :" << min_ind << endl;
	}
	DEBUG_STREAM << "Error array : " << err_array << endl;
	Mat feat_ind,sequence_ind;
	sortIdx(err_array,feat_ind,CV_SORT_EVERY_ROW + CV_SORT_ASCENDING);
	DEBUG_STREAM << "feature index : " << feat_ind << endl;
	feature_indices_ = feat_ind.colRange(0,cascade_count_).clone();
	DEBUG_STREAM << "after clipping feature index : " << feature_indices_ << endl;
//	sortIdx(feat_ind,sequence_ind,CV_SORT_EVERY_ROW + CV_SORT_ASCENDING);
//	DEBUG_STREAM << " Feat_ind : " << sequence_ind << endl;
	cascade_list_.resize(cascade_count_);
	for(int i = 0; i < cascade_count_; i++){
		DEBUG_STREAM << "feature indexes : " << feature_indices_.at<int>(i) << endl;
		cascade_list_[i] = classifier_list[feature_indices_.at<int>(i)];
//		DEBUG_STREAM << "removing index from classifier list : " << feature_indices_.at<int>(i)-i <<endl;
//		classifier_list.erase(classifier_list.begin() + feature_indices_.at<int>(i)-i);
	}
//	DEBUG_STREAM << "removing the indixes">
//	for(int i = 0; i < cascade_count_; i++){
//		DEBUG_STREAM << "feature indexes : " << feature_indices_.at<int>(i)-i << endl;
//		classifier_list.erase(classifier_list.begin() + feature_indices_.at<int>(i)-i);
//	}

	DEBUG_STREAM <<"classifier_list size : " << classifier_list.size() << endl;
	int i = 0;
	while(classifier_list.size()!= 0){
		if(cv::isInsideMat<int>(i,feature_indices_)){
			DEBUG_STREAM << "found " << i <<" not removing!"<< endl;
			classifier_list.erase(classifier_list.begin());
			i++;
			continue;
		}
		classifier_list[0]->~simple_classifier();
		classifier_list.erase(classifier_list.begin());
		i++;
	}
	DEBUG_STREAM <<"classifier_list size : " << classifier_list.size() << endl;
//	local_dbg::redirect_outstream_stdout();
	return true;
}
float CascadedClassifier::predict(const Mat& samples,Mat& predictions,Mat& probabilities) const {
//	local_dbg::redirect_outstream_file();
	DEBUG_STREAM<<"****PREDICTING******" << endl;
//	Mat examples = samples.clone();
	Mat pre_predections,probs;
	for (int i = 0; i < cascade_list_.size(); i++) {
		Mat cur_predections(samples.rows,1,CV_64F);
		DEBUG_STREAM << "------------------Cascade number " <<  i <<" ----------------\n" << endl;
//		cascade_list_.at(i)->predict(samples.colRange(feature_indices_.at<int>(i),feature_indices_.at<int>(i) + 1),cur_predections);
		DEBUG_STREAM << "features are : " << samples.colRange(feature_indices_.at<int>(i),feature_indices_.at<int>(i) + 1) << endl;
		for(int j = 0; j < samples.rows; j++){
			cur_predections.at<double>(j) = cascade_list_[i]->predict(samples.colRange(feature_indices_.at<int>(i),feature_indices_.at<int>(i) + 1).rowRange(j,j+1),Mat(),false)->value;
		}
//		cascade_list_[i]->predict(samples.colRange(feature_indices_.at<int>(i),feature_indices_.at<int>(i) + 1),cur_predections);
		DEBUG_STREAM << "Prediction done"<<endl;
//		probabilities = cur_predections.clone();
		probs = cur_predections;
		cur_predections = (cur_predections < threshold_)/255;
		cur_predections.convertTo(cur_predections,CV_32F);
		DEBUG_STREAM << " cur_predections : "<< cur_predections << endl;
		if(i == 0){
			pre_predections = cur_predections.clone();
		}else{
			pre_predections = pre_predections.mul(cur_predections);
		}
		DEBUG_STREAM << " pre_predections : "<< pre_predections << endl;
	}
	probabilities = probs.clone();
	predictions = pre_predections.clone();
//	local_dbg::redirect_outstream_stdout();
	return 0.0;
}
