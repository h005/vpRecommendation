#ifndef PREDICTOR_H
#define PREDICTOR_H

#include <opencv.hpp>
#include "imgset.h"
#include "features/feageo.h"
#include "viewpointset.h"

class QString;

class Predictor
{
public:
    enum ViewID{ViewId_Img,ViewId_Geo,ViewId_ImgGeo};
public:
    Predictor();
    ~Predictor();
    Predictor(cv::Mat XTest1);
    Predictor(cv::Mat XTest1, cv::Mat XTest2);

    void setImgFeatures(ImgSet *imgset);
    void setGeoFeatures(ViewPointSet *vpset);
    void setImgGeoFeatures(ViewPointSet *vpset);

    void predictScoreWithViewId(cv::Mat &score, int viewId);
    void predictLabelWithViewId(cv::Mat &label, int viewId);

    void score2Label(cv::Mat &label, cv::Mat &score);


private:
    // viewId can be view1, view2 and view1 + view2
    void predictScore(cv::Mat &score, int viewId);
    // this this label should be given with the pairwised imgs
    // i. e. the the rows of the XTest should be even
    void predictLabel(cv::Mat &label, int viewId);

    void feaNormalization(cv::Mat &fea, cv::Mat &scalePara);

    void fillData();
    void fillMatrix(QString fileName, cv::Mat &Data);
    void predictCoreFunction(const cv::Mat &XTest,
                             const cv::Mat &XTrain,
                             const cv::Mat &g,
                             double &bm,
                             cv::Mat &score);

    void sumMultiply(const cv::Mat &input, cv::Mat &res);
    void expDK(cv::Mat &DK, double &sigma, cv::Mat &TK);

    void sigmod(cv::Mat &score);

    void cleanUp();

private:

    cv::Mat XTrain1;
    cv::Mat XTrain2;
//    cv::Mat YTrain;
    cv::Mat XTest1;
    cv::Mat XTest2;
    cv::Mat YTest;
    cv::Mat ga;
    cv::Mat gb;
    double bam;
    double bbm;

    cv::Mat imgFeaScale;
    cv::Mat geoFeaScale;

    // parameters
    double CA = 4.0;
    double CB = 4.0;
    double D = 0.1;
    double eps = 0.01;



};

#endif // PREDICTOR_H
