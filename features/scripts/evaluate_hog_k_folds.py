#! /usr/bin/python
# load mnist and train svm.


import pylab, numpy, csv, sys, sklearn
from sklearn.svm import SVC;
from os import walk
from sklearn.metrics import roc_curve, auc
from docutils.nodes import legend
from sklearn.metrics.classification import accuracy_score
from evaluate_hog import readFeatureCSV
# from sklearn.svm.libsvm import cross_validation
from sklearn.cross_validation import KFold
from numpy import char, round_

def readDir(dir_path):
    train_feat=numpy.ndarray((0));
    train_labels=numpy.ndarray((0));
    test_feat=numpy.ndarray((0));
    test_labels=numpy.ndarray((0));
    for (dirpath, dirnames, filenames) in walk(dir_path):
        for ind,filename in enumerate(filenames):
            file_path = dirpath+"/"+filename;
            print("----------------reading file "+str(ind)+" : ")
            print(file_path);
            print("labels will be : "+ str(int('pos' in file_path)));
            if "train" in file_path and len(train_feat) <= 0:
                print("creating training data set");
                [train_feat,train_labels] = readFeatureCSV(file_path, int('pos' in file_path));
                print("data size : "+str(train_feat.shape))
                print("labels size : "+str(train_labels.shape))
            elif "train" in file_path and len(train_feat) > 0:
                print("adding to training data set")
                [feat,labels] = readFeatureCSV(file_path, int('pos' in file_path));
                print("data size : "+str(feat.shape))
                print("labels size : "+str(labels.shape))
                train_feat = numpy.vstack((train_feat,feat));
                train_labels = numpy.hstack((train_labels,labels));
            elif "test" in file_path and len(test_feat) <= 0:
                print("creating testing data set")
                [test_feat,test_labels] = readFeatureCSV(file_path, int('pos' in file_path));
                print("data size : "+str(test_feat.shape))
                print("labels size : "+str(test_labels.shape))
            else:
                print("adding to testing data set")
                [feat,labels] = readFeatureCSV(file_path, int('pos' in file_path));
                print("data size : "+str(feat.shape))
                print("labels size : "+str(labels.shape))
                test_feat = numpy.vstack((test_feat,feat));
                test_labels = numpy.hstack((test_labels,labels));
                
    return train_feat,train_labels,test_feat,test_labels;
    
def main(argv):
    if len(argv) < 2:
        print(argv[0] + " [path/to/hog]");
        return;
    
    
    legends= [];
    print("************* HOG form " + argv[1])
    train_feat,train_labels,test_feat,test_label = readDir(argv[1]);
    folds = 5;
    kf = KFold(len(train_feat), n_folds=folds,shuffle=True);
    fold_num = 0;
    best_classifier = [];
    max_acc = 0;
    min_acc = 100;
    for train_index, test_index in kf:
        print"fold number %d : "%(fold_num)
        t_data = train_feat[train_index];
        t_labels = train_labels[train_index];
        v_data = train_feat[test_index];
        v_labels = train_labels[test_index];
        
        classifier = SVC(probability=True,verbose=True);
        classifier.fit(t_data, t_labels);
        predictions =classifier.predict(v_data);
        
        acc = accuracy_score(v_labels, predictions)*100;
        print("Accuracy : "+str(acc) + " %");
        if acc > max_acc :
            max_acc = acc;
            best_classifier = classifier;
        if acc < min_acc :
            min_acc = acc;
    
    probs = best_classifier.predict_proba(test_feat);
    predictions =classifier.predict(test_feat);    
    acc = accuracy_score(test_label, predictions)*100;    
    tpr,fpr,th = roc_curve(test_label, probs[:,1],pos_label=0);
        
    pylab.plot(fpr,tpr,'r');
    legends.append("HOG Dalal and Trigs");
     
    pylab.plot(numpy.arange(0,1.1,0.1),numpy.arange(0,1.1,0.1),'k--')
    pylab.plot(numpy.arange(1,-0.1,-0.1),numpy.arange(0,1.1,0.1),'g--')
    pylab.legend(legends);
    
    mean_acc = round((max_acc + min_acc) / 2);
    var_acc = round_(max_acc - mean_acc,2);
    
    pylab.title(str(folds)+" folds verification ROC, accuracy : "+str(mean_acc) + " +- "+str(var_acc)+" %")
    pylab.xlabel('False Positive Rate');
    pylab.ylabel('True Positive Rate');
    pylab.grid('on')
    pylab.show()

if __name__ == "__main__":
    main(sys.argv)