#include "predictor.h"

#include <QString>
#include <QDir>
#include <QTextStream>

#include <fstream>
#include <QPlainTextEdit>

extern QPlainTextEdit *messageWidget;

Predictor::Predictor()
{
    fillData();
}

Predictor::~Predictor()
{
    cleanUp();
}

Predictor::Predictor(cv::Mat XTest1)
{
    fillData();
}

Predictor::Predictor(cv::Mat XTest1, cv::Mat XTest2)
{
    fillData();
}

void Predictor::setImgFeatures(ImgSet *imgset)
{
    imgset->copyImgFeatureTo(XTest1);
    feaNormalization(XTest1, imgFeaScale);
}

void Predictor::setGeoFeatures(ViewPointSet *vpset)
{
    vpset->copyGeoFeatureTo(XTest2);
    feaNormalization(XTest2, geoFeaScale);
}

void Predictor::predictLabel(cv::Mat &label, int viewId)
{
    assert(!(XTest1.rows % 2));
    cv::Mat score;
    predictScoreWithViewId(score, viewId);

    label.create(score.rows,score.cols,CV_8SC1);
    for(int i=0;i<score.rows;i+=2)
        for(int j=0;j<score.cols;j++)
            if(score.at<double>(i,j) < score.at<double>(i+1,j))
            {
                label.at<char>(i,j) = -1;
                label.at<char>(i+1,j) = 1;
            }
            else
            {
                label.at<char>(i,j) = 1;
                label.at<char>(i+1,j) = -1;
            }
}

void Predictor::feaNormalization(cv::Mat &fea, cv::Mat& scalePara)
{
    for(int i=0;i<fea.rows;i++)
        for(int j=0;j<fea.cols;j++)
        {
            fea.at<double>(i,j) = (scalePara.at<double>(j,3) - scalePara.at<double>(j,2)) \
                    * (fea.at<double>(i,j) - scalePara.at<double>(j,0)) \
                    / (scalePara.at<double>(j,1) - scalePara.at<double>(j,0)) \
                    + scalePara.at<double>(j,2);
        }
}

void Predictor::predictScoreWithViewId(cv::Mat &score, int viewId)
{
    predictScore(score,viewId);
}

void Predictor::predictLabelWithViewId(cv::Mat &label, int viewId)
{
    cv::Mat score;
    predictScore(score, viewId);
//    for(int i=0;i<score.rows;i++)
//        std::cout << "score " << score.at<double>(i,0) << std::endl;
    // convert the score to label
    assert(!(XTest1.rows % 2));
    label.create(score.rows,score.cols,CV_8SC1);
    for(int i=0;i<score.rows;i+=2)
        for(int j=0;j<score.cols;j++)
        {
            if(score.at<double>(i,j) < score.at<double>(i+1,j))
            {
                label.at<char>(i,j) = -1;
                label.at<char>(i+1,j) = 1;
            }
            else
            {
                label.at<char>(i,j) = 1;
                label.at<char>(i+1,j) = -1;
            }
        }
    score.release();
}

void Predictor::predictScore(cv::Mat &score, int viewId)
{
    switch(viewId)
    {
    case ViewId_Img:
        predictCoreFunction(XTest1, XTrain1, ga, bam, score);
        break;
    case ViewId_Geo:
        predictCoreFunction(XTest2, XTrain2, gb, bbm, score);
        break;
    case ViewId_ImgGeo:
        // used for both Img and Geo features
        cv::Mat score1;
        cv::Mat score2;
        predictCoreFunction(XTest1, XTrain1, ga, bam, score1);
        predictCoreFunction(XTest2, XTrain2, gb, bbm, score2);
        // codes for merge the score1 and score2
        score.create(score1.rows, score1.cols, CV_64FC1);
        for(int i=0;i < score1.rows; i++)
            for(int j=0;j<score1.cols;j++)
            {
                score.at<double>(i,j) = score1.at<double>(i,j) + score2.at<double>(i,j);
                score.at<double>(i,j) /= 2.0;
            }
        score1.release();
        score2.release();
        break;
    }
    sigmod(score);



}

///
/// \brief Predictor::predictCoreFunction
/// \param XTest ncases * mfeatures
/// \param XTrain ncases * mfeatures
/// \param score
///
void Predictor::predictCoreFunction(const cv::Mat &XTest,
                                    const cv::Mat &XTrain,
                                    const cv::Mat &g,
                                    double &bm,
                                    cv::Mat &score)
{
    const double xsigma = 0.01935;
    cv::Mat XTrain_transpose;
    cv::Mat d1;
    cv::Mat d2;
    cv::transpose(XTrain, XTrain_transpose);

    double dim = (double)XTrain.cols;
//    std::cout << "XTest size " << XTest.rows << " " << XTest.cols << std::endl;
//    std::cout << "XTrain size " << XTrain.rows << " " << XTrain.cols << std::endl;
    cv::Mat TK = XTest * XTrain_transpose;
//    std::cout << "TK " << TK.rows << " " << TK.cols << std::endl;
    double sigma = xsigma * dim;
//    std::cout << "sigma " << sigma << std::endl;
    sumMultiply(XTest, d1);
    sumMultiply(XTrain, d2);
//    std::cout << "d1 " << std::endl;
//    for(int i=0;i<d1.rows;i++)
//        for(int j=0;j<d1.cols;j++)
//            std::cout << d1.at<double>(i,j) << " ";
//    std::cout << std::endl;
//    std::cout << "d2 " << std::endl;
//    for(int i=0;i<d2.rows;i++)
//        for(int j=0;j<d2.cols;j++)
//            std::cout << d2.at<double>(i,j) << " ";
//    std::cout << std::endl;
//    std::cout << "Multiply done" << std::endl;
    cv::Mat onesTrain(1, XTrain.rows, CV_64FC1);
    for(int i=0;i<onesTrain.cols;i++)
        onesTrain.at<double>(0,i) = 1.0;
    cv::Mat onesTest(XTest.rows, 1, CV_64FC1);
    for(int i=0;i<onesTest.rows;i++)
        onesTest.at<double>(i,0) = 1.0;
    cv::transpose(d2, d2);
//    std::cout << "d2 transpose "<< std::endl;

    cv::Mat DK = d1 * onesTrain + onesTest * d2 - 2.0 * TK;
//    cv::Mat DK = d1 * onesTrain;
//    std::cout << "DK size "<< DK.rows << " " << DK.cols << std::endl;
//    for(int i=0;i<DK.cols;i++)
//        std::cout << DK.at<double>(0,i) << " ";
//    std::cout << std::endl;
//    std::cout << "DK "<< std::endl;
//    for(int i=0;i<DK.cols;i++)
//        std::cout << DK.at<double>(1,i) << " ";
//    std::cout << std::endl;
    expDK(DK, sigma, TK);
//    std::cout << "expDK done" << std::endl;
    // g 1 * 1000
//    std::cout << "g matrix " << g.rows << " " << g.cols << std::endl;
//    std::cout << "TK matrix " << TK.rows << " " << TK.cols << std::endl;
//    std::cout << "bm matrix " << bm.rows << " " << bm.cols << std::endl;
    score = g * TK + bm;
    cv::transpose(score, score);
//    std::cout << "score transpose done" << std::endl;
    XTrain_transpose.release();
    d1.release();
    d2.release();
    TK.release();
    onesTrain.release();
    onesTest.release();
    DK.release();
}

///
/// \brief Predictor::sumMultiply
/// \param input
/// \param res
/// this function performs the multiplates of sum(input.^2,2)
///
void Predictor::sumMultiply(const cv::Mat &input, cv::Mat &res)
{
    cv::Mat tmp = input.mul(input);
    res.create(input.rows,1,CV_64FC1);
    for(int i = 0; i < res.rows; i++)
        res.at<double>(i,0) = 0.0;
    for(int i = 0; i < tmp.rows; i++)
        for(int j = 0; j < tmp.cols; j++)
            res.at<double>(i,0) += tmp.at<double>(i,j);
    tmp.release();
}

///
/// \brief Predictor::expDK
/// \param DK
/// \param sigma
///
/// TK' = exp(-DK/sigma)
///
void Predictor::expDK(cv::Mat &DK, double &sigma, cv::Mat &TK)
{
    TK.create(DK.rows, DK.cols, CV_64FC1);
    for(int i=0; i < DK.rows; i++)
        for(int j=0; j < DK.cols; j++)
            TK.at<double>(i,j) = exp( - DK.at<double>(i,j) / sigma);
    cv::transpose(TK, TK);
}

void Predictor::sigmod(cv::Mat &score)
{
    for(int i=0;i<score.rows;i++)
        for(int j=0;j<score.cols;j++)
            score.at<double>(i,j) = 1.0 / (1.0 + exp( -score.at<double>(i,j) ));
}

void Predictor::cleanUp()
{
    XTrain1.release();
    XTrain2.release();
    XTest1.release();
    XTest2.release();
    YTest.release();
    ga.release();
    gb.release();
    imgFeaScale.release();
    geoFeaScale.release();
}


void Predictor::fillData()
{
    messageWidget->appendPlainText("predictor data loading...");
    messageWidget->repaint();
    // fill in the ga
    fillMatrix(":/svm2kData/svm2k/data/ga.txt", ga);
    cv::transpose(ga,ga);
//    std::cout << "load ga matrix done " << std::endl;
    // fill in the gb
    fillMatrix(":/svm2kData/svm2k/data/gb.txt", gb);
    cv::transpose(gb,gb);
//    std::cout << "load gb matrix done " << std::endl;
    // fill in the XTrain1
    fillMatrix(":/svm2kData/svm2k/data/XTrain1.txt", XTrain1);
//    std::cout << "load XTrain1 matrix done" << std::endl;
    // fill in the XTrain2
    fillMatrix(":/svm2kData/svm2k/data/XTrain2.txt", XTrain2);
//    std::cout << "load XTrain1 matrix done" << std::endl;
    fillMatrix(":/svm2kData/svm2k/data/imgPsParameter.txt", imgFeaScale);
//    std::cout << "load imgFeaScale matrix done" << std::endl;
    fillMatrix(":/svm2kData/svm2k/data/geoPsParameter.txt", geoFeaScale);
//    std::cout << "load geoFeaScale matrix done" << std::endl;

//    bam = -0.2068;
//    bbm = -1.3079;

    // sep 18
    bam = -0.2450;
    bbm = -1.1504;

    messageWidget->appendPlainText("predictor data load done");
    messageWidget->repaint();
//    std::cout << "XTrain " << XTrain2.cols << std::endl;
}

void Predictor::fillMatrix(QString fileName, cv::Mat &Data)
{
    QFile gaFile(fileName);
    if(!gaFile.open(QIODevice::ReadOnly | QIODevice::Text))
        std::cout << "open failed " << std::endl;
    QTextStream fin(&gaFile);
    QString line = fin.readLine();
    if(line.isNull())
    {
        std::cerr << "falied to read the parameter file" << std::endl;
        exit(-1);
    }
    line = line.trimmed();
    QStringList paraSize = line.split(' ');
    int rows = paraSize[0].toInt();
    int cols = paraSize[1].toInt();
    Data.create(rows,cols,CV_64FC1);

    line = fin.readLine();
    int rowIndex = 0;
    while(!line.isNull())
    {
        line = line.trimmed();
        QStringList feature = line.split(' ');
        for(int i=0;i<cols;i++)
            Data.at<double>(rowIndex,i) = feature[i].toDouble();
        rowIndex++;
        line = fin.readLine();
    }
}

