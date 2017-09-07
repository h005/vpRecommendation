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

    void getRuleOfThirds(QString imgFile, float &res);

    void getHogFeature(QString imgFile, std::vector<float> &hog);

    void getGistFeature(QString imgFile, std::vector<float> &gist);

    void getDirectionsOfLines(QString imgFile, std::vector<float> &lsd);

    void getVanishLine(QString imgFile, std::vector<float> &vanish);

private:

    void setCentroid(double &centroidRow, double &centroidCol, cv::Mat &mask);

    double getAngle(cv::Point2d u, cv::Point2d v);

    double absDouble(double val);

};

#endif // FEAIMG_H
