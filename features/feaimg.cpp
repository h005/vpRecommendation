#include "feaimg.h"
#include "features/gistFeature/gist.h"
#include "features/gistFeature/standalone_image.h"
#include "features/lineSegmentFeature/linesegmentfea.h"
#include <QDebug>

using namespace saliency;

FeaImg::FeaImg()
{

}

void FeaImg::getRuleOfThirds(QString imgFile, float &res)
{
    res = 100000.0;
    double centroidRow = 0;
    double centroidCol = 0;
//    setCentroid();
    // abstract saliency mask
    std::string algorithm[2] = {"SPECTRAL_RESIDUAL","BING"};
    Saliency::create(algorithm[0]);
    Ptr<Saliency> saliencyAlgorithm = Saliency::create(algorithm[0]);
    if( saliencyAlgorithm == NULL)
    {
        cout << "***Error in the instantiation of the saliency algorithm...***\n";
        return ;
    }

    cv::Mat binaryMap;
    cv::Mat src = cv::imread(imgFile.toStdString());
    cv::resize(src,src,cv::Size(480,320));

    if(algorithm[0].find("SPECTRAL_RESIDUAL") == 0)
    {
        cv::Mat saliencyMap;
        if(saliencyAlgorithm->computeSaliency(src, saliencyMap))
        {
            StaticSaliencySpectralResidual spec;
            spec.computeBinaryMap(saliencyMap, binaryMap);
            // binaryMap is a mat with one channel and uchar type for element.
//            qDebug() << "channels " << binaryMap.channels() << endl;
//            channels = 1
//            cv::imshow( "Saliency Map", saliencyMap );
//            cv::imshow( "Original Image", src );
//            cv::imshow( "Binary Map", binaryMap );
//            cv::waitKey( 0 );
            setCentroid(centroidRow, centroidCol, binaryMap);
            double ruleOfThirdRow[2] = {1.0/3.0,2.0/3.0};
            double ruleOfThirdCol[2] = {1.0/3.0,2.0/3.0};

            for(int i=0;i<2;i++)
                for(int j=0;j<2;j++)
                {
                    double tmp = sqrt((centroidRow - ruleOfThirdRow[i])*(centroidRow - ruleOfThirdRow[i])
                                      +(centroidCol - ruleOfThirdCol[j])*(centroidCol - ruleOfThirdCol[j]));
                    res = res < tmp ? res : tmp;
                }

        }
        else
            res = 100000.0;
    }
    else
    {
        cout << "***Error in the instantiation of the saliency algorithm...***\n";
        return ;
    }
}

///
/// \brief FeaImg::setCentroid
/// \param centroidRow
/// \param centroidCol
///
/// this function was created to compute Geometric Centroid of an image
/// and was used in the function of setRuleOfThird()
/// and the setLineSegmentFeature()
///
void FeaImg::setCentroid(double &centroidRow, double &centroidCol, cv::Mat &mask)
{
    centroidRow = 0.0;
    centroidCol = 0.0;
    // ref http://mathworld.wolfram.com/GeometricCentroid.html
    double mess = 0;

//    qDebug()<<"getRuleOfThird .. "<<mask.rows<<" "<<mask.cols<<endl;

    for(int i=0;i<mask.rows;i++)
        for(int j=0;j<mask.cols;j++)
            if(mask.at<uchar>(i,j) != 255)
            {
                mess ++;
                centroidRow += i;
                centroidCol += j;
            }

    if(mess)
    {
        centroidRow /= mess;
        centroidCol /= mess;
    }

    //    scale to [0,1]
    if(mask.rows && mask.cols)
    {
        centroidRow /= mask.rows;
        centroidCol /= mask.cols;
    }
    else
    {
        centroidRow = 0.0;
        centroidCol = 0.0;
    }
}


void FeaImg::getHogFeature(QString imgFile, std::vector<float> &hog)
{
    hog.clear();
    cv::Mat src = cv::imread(imgFile.toStdString());
    cv::cvtColor(src,src,CV_BGR2GRAY);
    int NUMbins = 9;
    hog.resize(9,0);
    cv::resize(src,src,cv::Size(16,16));

    // widnowsSize,blockSize,blockStride,cellSize,numBins
    cv::HOGDescriptor d(cv::Size(16,16),cv::Size(8,8),cv::Size(4,4),cv::Size(4,4),NUMbins);

    std::vector<float> descriptorsValues;
    std::vector<cv::Point> locations;

    d.compute(src,descriptorsValues,cv::Size(0,0),cv::Size(0,0),locations);

    for(int i=0;i<descriptorsValues.size();i++)
        hog[i % NUMbins] += descriptorsValues[i];

}

void FeaImg::getGistFeature(QString imgFile, std::vector<float> &gist)
{
    const cls::GISTParams DEFAULT_PARAMS {true, 32, 32, 4, 3, {8, 8, 4}};
    cv::Mat src = cv::imread(imgFile.toStdString());
    if (src.empty())
    {
        std::cerr << "No input image!" << std::endl;
        exit(1);
    }
    vector<float> result;
    cls::GIST gist_ext(DEFAULT_PARAMS);
    gist_ext.extract(src, result);
    src.release();
}

void FeaImg::getDirectionsOfLines(QString imgFile, std::vector<float> &lsd)
{
    lsd.clear();
    cv::Mat src = imread(imgFile.toStdString());
    int NUM_Hist = 9;
    // line segment detection
    LineSegmentFea *lsf = new LineSegmentFea();

    double thLength = 30;

    lsf->initial(src, thLength);

    std::vector<double> angleHist(NUM_Hist, 0);
    double variance = 0.0;
    double entropy = 0.0;
    std::vector<double> clusterSize;

    // direction with x axis
    lsf->setHist_v_e(angleHist, variance, entropy);
    lsf->setClusterSize(clusterSize);
    for(int i=0;i<angleHist.size();i++)
        lsd.push_back(angleHist[i]);
    lsd.push_back(variance);
    lsd.push_back(entropy);
    for(int i=0;i<clusterSize.size();i++)
        lsd.push_back(clusterSize[i]);

/*
    // direction with the diagonal
    lsd.clear();
    double val_lb2ru = 0.0;
    double val_lu2rb = 0.0;
    lsd.push_back(val_lb2ru);
    lsd.push_back(val_lu2rb);
    lsf->setMinDiagonalAngle(val_lb2ru, val_lu2rb);
    lsf->setHist_diagonal(angleHist,"lb2ru");
    for(int i=0;i<angleHist.size();i++)
        lsd.push_back(angleHist[i]);
    lsf->setHist_diagonal(angleHist,"lu2rb");
    for(int i=0;i<angleHist.size();i++)
        lsd.push_back(angleHist[i]);
*/
    src.release();
}

void FeaImg::getVanishLine(QString imgFile,
                           std::vector<float> &vanish)
{
    vanish.clear();
    cv::Mat src = imread(imgFile.toStdString());
    // line segment detection
    LineSegmentFea *lsf = new LineSegmentFea();

    double thLength = 30;

    lsf->initial(src, thLength);
    std::vector<cv::Point2d> vps;
    vps.clear();
    lsf->setVanishPoints(vps);

    std::vector<cv::Point2d> lines;
    lines.clear();
    lines.push_back(cv::Point2d(vps[1] - vps[0]));
    lines.push_back(cv::Point2d(vps[2] - vps[1]));
    lines.push_back(cv::Point2d(vps[0] - vps[2]));
    cv::Point2d horizon(1.0,0.0);
    double minVal = absDouble(getAngle(lines[0],horizon));
    minVal = minVal < absDouble(getAngle(lines[1],horizon)) ? minVal : absDouble(getAngle(lines[1],horizon));
    minVal = minVal < absDouble(getAngle(lines[2],horizon)) ? minVal : absDouble(getangle(lines[2],horizon));
    // the first element is the min angle with the horizontal line
    vanish.push_back(minVal);

    std::vector<double> angles;
    angles.push_back(getAngle(cv::Point2d(-lines[0].x, -lines[0].y), lines[1]));
    angles.push_back(getAngle(lines[0], cv::Point2d(-lines[2].x, -lines[2].y)));
    angles.push_back(getAngle(cv::Point2d(-lines[1].x, -lines[1].y), lines[2]));
    std::sort(angles.begin(),angles.end());

    for(int i=0;i<angles.size();i++)
        vanish.push_back(angles[i]);

}

double FeaImg::getAngle(cv::Point2d u, cv::Point2d v)
{
    double angle = 0.0;
    if(u.dot(u) == 0 || v.dot(v) == 0)
        angle = 0.0;
    else
    {
        double cosTheta = u.ddot(v) / (sqrt(u.ddot(u) * v.ddot(v)));
        angle = acos(cosTheta);
    }
}

double FeaImg::absDouble(double val)
{
    return val < 0 ? -val : val;
}


