#include "imgset.h"
#include "features/feaimg.h"
#include <QFileInfo>
#include <QPlainTextEdit>

extern QPlainTextEdit *messageWidget;

ImgSet::ImgSet()
{
    imgFiles.clear();
    imgLabels = NULL;
}

ImgSet::~ImgSet()
{
    clean();
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
    int numRow = imgFiles.size();
    int numCol = 9 + 3 + 960 + 14;
    imgFeature.create(numRow,numCol, CV_64FC1);
    for(int i=0;i<imgFiles.size();i++)
    {
        FeaImg *feaImg = new FeaImg(imgFiles[i]);
        feaImg->setFeatures();
        feaImg->fillInMat(imgFeature, i);
        delete feaImg;
    }
}

void ImgSet::copyImgFeatureTo(cv::Mat &imgFea)
{
    imgFeature.copyTo(imgFea);
}

void ImgSet::printLabel(cv::Mat &label)
{
//    for(int i=0;i<imgFiles.size();i++)
//        std::cout << imgFiles[i].toStdString() << " " << (int)label.at<char>(i,0) << std::endl;
    QFileInfo *file = new QFileInfo();
    for(int i=0;i<imgFiles.size();i+=2)
    {
        file->setFile(imgFiles[i]);
        messageWidget->appendPlainText(file->fileName() + "    " + QString::number((int)label.at<char>(i,0)));
        file->setFile(imgFiles[i+1]);
        messageWidget->appendPlainText(file->fileName() + "    " + QString::number((int)label.at<char>(i+1,0)));
        messageWidget->appendPlainText("");
    }
}

void ImgSet::clean()
{

    imgFeature.release();

    for(int i=0;i<imgFiles.size();i++)
        delete imgLabels[i];
    delete[] imgLabels;
    imgLabels = NULL;

    imgFiles.clear();
}
