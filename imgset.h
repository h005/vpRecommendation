#ifndef IMGSET_H
#define IMGSET_H

#include "imglabel.h"
#include <QDebug>
#include <opencv.hpp>

class ImgSet
{
public:
    ImgSet();

    void setImgFiles(QStringList imgFiles);

    int size();

    void initialImgLabels();

    void setBackGround();

    ImgLabel* getImgLabel(int index);

    QSize imgLabelSize();

    void setFeatures();

    void copyImgFeatureTo(cv::Mat &imgFea);

    void printLabel(cv::Mat &label);

private:
    QStringList imgFiles;
    ImgLabel **imgLabels;
    cv::Mat imgFeature;
};

#endif // IMGSET_H
