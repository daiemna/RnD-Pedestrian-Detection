#! /usr/bin/python
# load mnist and train svm.


import pylab, numpy, csv, sys, sklearn
from sklearn.svm import SVC;
from os import walk
from sklearn.metrics import roc_curve, auc
from docutils.nodes import legend
from sklearn.metrics.classification import accuracy_score

def readFeatureCSV(path,label):
    with open(path, 'rb') as csvfile:
        reader = csv.reader(csvfile, delimiter=';');
        feat_vec = [];
        labels = [];
        row_num = 0
        for row in reader:
            try:
#                 feats = [float(feat) if feat != '' else 0 for feat in row]
                feats = [float(feat) for feat in row]
                if len(feats) <= 0 :
                    print("length Error in row "+str(row_num) + " file : " + path);
                    return feat_vec,labels;
                feat_vec.append(feats);
                labels.append(label);
                row_num = row_num + 1;
            except ValueError:
                print("Value Error in row "+str(row_num) + " file : " + path);
                print(row);
                
#                 return feat_vec,labels;
             
        print("Converting to arrays");       
        feat_vec = numpy.asarray(feat_vec);
        labels = numpy.asarray(labels);
        print("Features Read from files"+ str(feat_vec.shape));
        return feat_vec,labels
def readDir_trainSVM(dir_path):
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
    
    classifier = SVC(probability=True,verbose=True);
    probs = classifier.fit(train_feat, train_labels).predict_proba(test_feat);
    return classifier,test_feat,test_labels,probs;
    
def main(argv):
    if len(argv) < 2:
        print(argv[0] + " [path/to/hog]");
        return;
    
    
    legends= [];
    print("************* HOG form " + argv[1])
    clf_1,test_1,tl_1,pro_1 = readDir_trainSVM(argv[1]);
    predict_1 =clf_1.predict(test_1);
    acc = accuracy_score(tl_1, predict_1)*100;
    print("Accuracy : "+str(acc) + " %");

    tpr_1,fpr_1,th_1 = roc_curve(tl_1, pro_1[:,1],pos_label=0);
    pylab.plot(fpr_1,tpr_1,'r');
    legends.append("form " + argv[1]);
     
    pylab.plot(numpy.arange(0,1.1,0.1),numpy.arange(0,1.1,0.1),'k--')
    pylab.plot(numpy.arange(1,-0.1,-0.1),numpy.arange(0,1.1,0.1),'g--')
    pylab.legend(legends);
 
    pylab.title('ROC'+" Accuracy : "+str(acc) + " %");
    pylab.xlabel('False Positive Rate');
    pylab.ylabel('True Positive Rate');
    pylab.grid('on')
    pylab.show()

if __name__ == "__main__":
    main(sys.argv)