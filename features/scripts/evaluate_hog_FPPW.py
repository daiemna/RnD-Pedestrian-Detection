#! /usr/bin/python
# load mnist and train svm.


import pylab, numpy, csv, sys, sklearn
from sklearn.svm import SVC;
from os import walk
from sklearn.metrics import roc_curve, auc, confusion_matrix, classification_report
from docutils.nodes import legend
from sympy.stats.rv import probability
from numpy import arange, logspace
from evaluate_hog import readFeatureCSV, readDir_trainSVM

def DET_curve(true_labels,probabilities,pos_label=1):
    fars=[]
    frrs=[]
    number_th = 10;
    
    min_prob = min(probabilities);
    max_prob = max(probabilities);
#     step = (max_prob - min_prob) / number_th;
    thresholds = logspace(-2,0,10);
    neg_index = [i for (i, val) in enumerate(true_labels) if val == pos_label]
    pos_index = [i for (i, val) in enumerate(true_labels) if val != pos_label]
    negatives = probabilities[neg_index];
    positives = probabilities[pos_index];
    
    for threshold in thresholds:
        far = len(negatives[negatives >= threshold]) / float(len(negatives))
        fars.append(far);
        frr = len(positives[positives <  threshold]) / float(len(positives))
        frrs.append(frr);
    
    return fars,frrs,thresholds;

def main(argv):
    if len(argv) < 2:
        print(argv[0] + " [path/to/hog]");
        return;
    
    
    legends= [];
    print("************* HOG form " + argv[1])
    clf_1,test_1,tl_1,pro_1 = readDir_trainSVM(argv[1]);
    predict_1 =clf_1.predict(test_1);
    fars,frrs,ths = DET_curve(tl_1, pro_1[:,1], pos_label=1)
    ths = [round(th,2) for th in ths]
    print("thresholds : ")
    print(ths)
    
    pylab.plot(frrs,fars,'ro--');
    legends.append("form " + argv[1]);
     
#     pylab.plot(numpy.arange(0,1.1,0.1),numpy.arange(0,1.1,0.1),'k--')
    pylab.legend(legends);
 
    pylab.title('DET');
    pylab.xticks(ths);
    pylab.yticks(ths);
    pylab.xlabel('False Positive Rate');
    pylab.ylabel('Miss Rate');
    pylab.grid(True)
    pylab.show()
    

if __name__ == "__main__":
    main(sys.argv)