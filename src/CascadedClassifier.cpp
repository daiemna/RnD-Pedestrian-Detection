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
//	double minVal = -2, maxVal = -2;
//	minMaxIdx(responses, &minVal, &maxVal);
	Mat shifted_responses;
	shifted_responses = responses.clone();
//	if (minVal == 0) {
//		shifted_responses.setTo(Scalar(-1), responses == Mat::zeros(Size(responses.cols,responses.rows),responses.type()));
//	}
	double minVal  = -2.0,maxVal = -2.0;
//	DEBUG_LOG("Shifted Response type : %d\n",shifted_responses.type());
	minMaxIdx(shifted_responses, &minVal, &maxVal);
//	DEBUG_LOG("before min, max = (%f,%f)\n",minVal,maxVal);
	if (minVal == 0.0) {
//		DEBUG_LOG("Min Value is zero making it -1\n");
//		DEBUG_STREAM << "Response : " << shifted_responses << endl;
		Mat mask;
		inRange(shifted_responses,Scalar(0),Scalar(0),mask);
//		DEBUG_STREAM << "Make -1 Mask : " << mask << endl;
		shifted_responses.setTo(Scalar(-1), mask);
	}
	minMaxIdx(shifted_responses, &minVal, &maxVal);
//	DEBUG_LOG("after min, max = (%f,%f)\n",minVal,maxVal);
	DEBUG_STREAM << "shifted_responses : " << shifted_responses << endl;

	Mat layers = Mat(3, 1,CV_32SC1);
//	DEBUG_STREAM << "Sample Count : " << sample_count << endl;
//	DEBUG_LOG("layers size : (%d,%d)\n", layers.rows, layers.cols);
	layers.row(0) = Scalar(train_data.cols); //number of inputs
	layers.row(1) = Scalar(2); //number of hidden neuron
	layers.row(2) = Scalar(1); //number of output neuron
//	DEBUG_STREAM << "MLP layers : " << endl << layers << endl;
//	DEBUG_STREAM << "Training Data : " << train_data << endl;
//	DEBUG_STREAM << "Expected Response : " << responses << endl;
	for (int i = 0; i < cascade_list->size(); i++) {
//		DEBUG_LOG("Cascade number : %d\n",i);
		Mat pre_weight_vector = cur_weight_vector.clone();
		Mat weak_responses;
		cascade_list->at(i).create(layers);
		DEBUG_LOG("----------------------------------MLP Created!------------------------------\n");
		cascade_list->at(i).train(train_data, responses, cur_weight_vector);
		cascade_list->at(i).predict(train_data, weak_responses);
		weak_responses = (weak_responses < 0.5)/255;
		weak_responses.convertTo(weak_responses,CV_32F);
		Mat sumW(1, 1, CV_64F);
		reduce(pre_weight_vector, sumW, 0, CV_REDUCE_SUM);
		double sum_weights = sumW.at<float>(0);
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

//		DEBUG_LOG("Weak Response type : %d\n",weak_responses.type());
		minVal  = -2.0,maxVal = -2.0;
		minMaxIdx(weak_responses, &minVal, &maxVal);
//		DEBUG_LOG("before min, max = (%f,%f)\n",minVal,maxVal);
		if (minVal == 0.0) {
//			DEBUG_LOG("Min Value is zero making it -1\n");
//			DEBUG_STREAM << "Response : " << weak_responses << endl;
			Mat mask;
			inRange(weak_responses,Scalar(0),Scalar(0),mask);
//			DEBUG_STREAM << "Make -1 Mask : " << mask << endl;
			weak_responses.setTo(Scalar(-1.0), mask);
		}
		minMaxIdx(weak_responses, &minVal, &maxVal);
//		DEBUG_LOG("after min, max = (%f,%f)\n",minVal,maxVal);
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
		Mat total_err(pre_weight_vector.mul(conditional));
		DEBUG_STREAM << "total_err type : " << total_err.type() << endl;
		DEBUG_STREAM << "total_err : " << total_err.at<float>(0) << endl;
		float err = total_err.at<float>(0) / sum_weights;
		DEBUG_STREAM << "err : " << err << endl;
		double C = log((1 - err) / err);
		if(err == 1)
			C = 1.0;
		DEBUG_STREAM << "C : " << C << endl;

//		Mat weight_dist;
		cv::exp(conditional.mul(Scalar(C)),cur_weight_vector);
		DEBUG_STREAM << "cur_weight_vector : " << cur_weight_vector << endl;
//		Mat sumW_dist;
//		reduce(cur_weight_vector,sumW_dist,0,CV_REDUCE_SUM);
//		DEBUG_STREAM << "sumW_dist : " << sumW_dist.at<float>(0) << endl;

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
		sum_weights = sumW.at<float>(0);
		DEBUG_STREAM << "sum_weights : " << sum_weights << endl;
		cur_weight_vector = cur_weight_vector.mul(Scalar(1.0 / sum_weights));
		DEBUG_STREAM << "cur_weight_vector after norm : " << cur_weight_vector << endl;
//		break;
	}
	return true;
}
float CascadedClassifier::predict(const Mat& sample) const {
	return -1.0;
}
