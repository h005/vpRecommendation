#include "imgset.h"
#include "features/feaimg.h"

ImgSet::ImgSet()
{
    imgFiles.clear();
    imgLabels = NULL;
}

void ImgSet::setImgFiles(QStringList imgFiles)
{
    this->imgFiles = imgFiles;
}

int ImgSet::size()
{
    return imgFiles.size();
}

void ImgSet::initialImgLabels()
{
    imgLabels = new ImgLabel*[imgFiles.size()];
    for(int i=0;i<imgFiles.size();i++)
    {
        imgLabels[i] = new ImgLabel();
        imgLabels[i]->loadPixmap(imgFiles[i]);
    }
}

void ImgSet::setBackGround()
{

    for(int i=0; i< imgFiles.size();i++)
    {
        imgLabels[i]->setAutoFillBackground(true);
        imgLabels[i]->setPixmap(QPixmap(imgFiles[i]).scaled(
                                    imgLabels[i]->size(),
                                    Qt::KeepAspectRatio,
//                                    Qt::IgnoreAspectRatio,
                                    Qt::SmoothTransformation));
        qDebug() << "imgLabel size " << endl;
        qDebug() << imgLabels[i]->size() << endl;
//        QPalette palette = imgLabels[i]->palette();
//        palette.setBrush(QPalette::Window,
//                         QBrush(QPixmap(imgFiles[i]).scaled(
//                                    imgLabels[i]->size(),
//                                    Qt::IgnoreAspectRatio,
//                                    Qt::SmoothTransformation)));
//        imgLabels[i]->setPalette(palette);
//        imgLabels[i]->repaint();
    }
}

ImgLabel *ImgSet::getImgLabel(int index)
{
    return imgLabels[index];
}

QSize ImgSet::imgLabelSize()
{
    return imgLabels[0]->size();
}

void ImgSet::setFeatures()
{
    FeaImg *feaImg = new FeaImg();
    for(int i=0;i<imgFiles.size();i++)
    {
//        float res;
//        feaImg->getRuleOfThirds(imgFiles[i],res);
        std::vector<float> vanishFea;
        feaImg->getVanishLine(imgFiles[i], vanishFea);
        break;
    }
}
