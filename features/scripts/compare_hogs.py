#! /usr/bin/python
# load mnist and train svm.


import pylab, numpy, csv, sys, sklearn
from sklearn.svm import SVC;
from os import walk
from sklearn.metrics import roc_curve, auc
from docutils.nodes import legend
from evaluate_hog import readFeatureCSV, readDir_trainSVM

def main(argv):
    if len(argv) < 3:
        print(argv[0] + " [path/to/hog1] [path/to/hog2]");
        return;
    
    
    legends= [];
    print("************* HOG form " + argv[1])
    clf_1,test_1,tl_1,pro_1 = readDir_trainSVM(argv[1]);
    predict_1 =clf_1.predict(test_1);
#     print("probs :");
#     print(pro_1);
    tpr_1,fpr_1,th_1 = roc_curve(tl_1, pro_1[:,1],pos_label=0);
    pylab.plot(fpr_1,tpr_1,'r');
    legends.append("form " + argv[1]);
    print("**************HOG form " + argv[2])
    clf_2,test_2,tl_2,pro_2 = readDir_trainSVM(argv[2]);
    predict_2 =clf_2.predict(test_2);
#     print("probs :");
#     print(pro_2);
    tpr_2,fpr_2,th_2 = roc_curve(tl_2, pro_2[:,1],pos_label=0);
    pylab.plot(fpr_2,tpr_2,'g');
    legends.append("form " + argv[2]);
    
    pylab.plot(numpy.arange(0,1,0.1),numpy.arange(0,1,0.1),'k--')
    pylab.plot(numpy.arange(1,-0.1,-0.1),numpy.arange(0,1.1,0.1),'g--')
    pylab.legend(legends);

    pylab.title('ROC');
    pylab.xlabel('False Positive Rate');
    pylab.ylabel('True Positive Rate');
    pylab.grid('on');
    pylab.show()

if __name__ == "__main__":
    main(sys.argv)