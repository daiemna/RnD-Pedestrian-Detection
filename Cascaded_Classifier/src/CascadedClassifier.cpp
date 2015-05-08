/*
 * CascadedClassifier.cpp
 *
 *  Created on: Mar 1, 2015
 *      Author: Daiem
 */

#include <CascadedClassifier.hpp>

using namespace ml;

CascadedClassifier::CascadedClassifier(unsigned int cascadeCount) {
	cascade_list = new vector<simple_classifier>();
	for (int i = 0; i < cascadeCount; i++) {
		cascade_list->push_back(simple_classifier());
	}

}
CascadedClassifier::~CascadedClassifier() {
	while (!cascade_list->empty()) {
		cascade_list->pop_back();
	}
	delete cascade_list;
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

	Mat layers = Mat(3, 1,CV_32SC1);
	layers.row(0) = Scalar(train_data.cols); //number of inputs
	layers.row(1) = Scalar(2); //number of hidden neuron
	layers.row(2) = Scalar(1); //number of output neuron
	for (int i = 0; i < cascade_list->size(); i++) {
		Mat pre_weight_vector = cur_weight_vector.clone();
		Mat weak_responses;
		cascade_list->at(i).create(layers);
		DEBUG_LOG("----------------------------------MLP Created!------------------------------\n");
		cascade_list->at(i).train(train_data, responses, cur_weight_vector);
		cascade_list->at(i).predict(train_data, weak_responses);
		weak_responses = (weak_responses < 0.5)/255;
		weak_responses.convertTo(weak_responses,CV_32F);
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
		DEBUG_STREAM << "C : " << C << endl;

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
	}
	return true;
}
float CascadedClassifier::predict(const Mat& samples,Mat& predictions) const {
	Mat examples = samples.clone();
	Mat cur_predections, pre_predections,indexes;
	for (int i = 0; i < cascade_list->size(); i++) {
		DEBUG_LOG("------------------Cascade number %d----------------\n",i);
		cascade_list->at(i).predict(examples,cur_predections);
		cur_predections = (cur_predections < 0.5)/255;
		cur_predections.convertTo(cur_predections,CV_32F);
		DEBUG_STREAM << " cur_predections : "<< cur_predections << endl;
		if(i == 0){
			pre_predections = cur_predections.clone();
		}else{
			pre_predections = pre_predections.mul(cur_predections);
		}
		DEBUG_STREAM << " pre_predections : "<< pre_predections << endl;
	}
	predictions = pre_predections.clone();
	return 0.0;
}
