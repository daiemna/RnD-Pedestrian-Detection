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
		cascade_list.push_back(simple_classifier());
	}

}
CascadedClassifier::~CascadedClassifier() {
	while (!cascade_list.empty()) {
		simple_classifier cascade = cascade_list.pop_back();
		delete cascade;
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
	double minVal = -2, maxVal = -2;
	minMaxIdx(responses, &minVal, &maxVal);
	Mat shifted_responses;
	shifted_responses = responses.clone();
	if (minVal == 0) {
		shifted_responses.setTo(Scalar(-1), (responses == Scalar(0)));
	}

	Mat layers(3, 1, CV_32S);
	layers[0] = sample_count; //number of inputs
	layers[1] = 1; //number of hidden neuron
	layers[2] = 1; //number of output neuron
	for (int i = 0; i < cascade_list.size(); i++) {
		Mat pre_weight_vector = cur_weight_vector.clone();
		Mat weak_responses;
		cascade_list[i].create(layers, CvANN_MLP::SIGMOID_SYM, 1, 1);
		cascade_list[i].train(train_data, responses, cur_weight_vector);
		cascade_list[i].predict(train_data, weak_responses);
		Mat sumW(1, 1, CV_64F);
		reduce(pre_weight_vector, sumW, 0, CV_REDUCE_SUM);
		double sum_weights = sumW.at<double>(0, 0);
		//updating current weight vector.
		//TODO: Calculate pseudo error
		//TODO: Update Weights
		/* NOTE: following comments are copied from opencv
		 * file[1] opencv/apps/traincscade/boost.cpp
		 */

		// Discrete AdaBoost:
		//   weak_eval[i] =(f(x_i)) is in {-1,1}
		//   err = sum(w_i*(f(x_i) != y_i))/sum(w_i)
		//   C = log((1-err)/err)
		//   w_i *= exp(C*(f(x_i) != y_i))
		minVal = maxVal = -2.0;
		minMaxIdx(weak_responses, &minVal, &maxVal);
		if (minVal == 0) {
			weak_responses.setTo(Scalar(-1), (weak_responses == Scalar(0)));
		}
		Mat total_err(
				pre_weight_vector.mul(weak_responses != shifted_responses));
		Mat sum_err(1, 1, CV_64F);
		reduce(total_err, sum_err, 0, CV_REDUCE_SUM);
		double err = sum_err.at<double>(0, 0) / sum_weights;
		double C = log((1 - err) / err);
		exp((weak_responses != shifted_responses) * Scalar(C),
				cur_weight_vector);

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

		// cur_weight_vector =
		// normalize weights
		reduce(cur_weight_vector, sumW, 0, CV_REDUCE_SUM);
		sum_weights = sumW.at<double>(0, 0);
		cur_weight_vector = cur_weight_vector * Scalar(1.0 / sum_weights);
	}
	return false;
}
float CascadedClassifier::predict(const Mat& sample) const {
	return -1.0;
}
