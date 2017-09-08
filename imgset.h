#ifndef IMGSET_H
#define IMGSET_H

#include "imglabel.h"
#include <QDebug>

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

private:
    QStringList imgFiles;
    ImgLabel **imgLabels;
};

#endif // IMGSET_H
