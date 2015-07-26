#include "FeatureEvaluator.h"

using namespace feat;

FeatureEvaluator::FeatureEvaluator(){

}
FeatureEvaluator::~FeatureEvaluator(){

}

void FeatureEvaluator::write_features(string path) {
	std::ofstream out_file(path.c_str(), ofstream::out | ofstream::app);
	if (out_file.is_open()) {
		ostringstream trimer;
		trimer << features_;
		string the_mat = trimer.str();
		the_mat.erase(0, 1);
		the_mat.erase(the_mat.size() - 1, 1);
		the_mat.erase(std::remove(the_mat.begin(), the_mat.end(), ' '),
				the_mat.end());
		out_file << the_mat << endl;
	}
	out_file.close();
}
