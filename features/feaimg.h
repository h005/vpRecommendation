#ifndef FEAIMG_H
#define FEAIMG_H

#include <QString>
#include <vector>
#include <opencv.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/saliency.hpp>
#include <algorithm>

class FeaImg
{
public:
    FeaImg();

    FeaImg(QString imgFile);

    void setFeatures();

    void fillInMat(cv::Mat &featureMat, int index);

private:

    void setRuleOfThirds();

    void setHogFeature();

    void setGistFeature();

    void setLSD_VanishLine();

    void setDirectionsOfLines();

    void setVanishLine();

    void setCentroid(double &centroidRow, double &centroidCol, cv::Mat &mask);

    double getAngle(cv::Point2d u, cv::Point2d v);

    double absDouble(double val);

    cv::Mat img;

    std::vector<double> fea;

};

#endif // FEAIMG_H
