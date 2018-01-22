#include "viewpointset.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <glm/gtc/matrix_transform.hpp>

ViewPointSet::ViewPointSet()
{
    refCameraPos.clear();
    sceneZ = -0.9;
    N_sample = 100;
    feaGeo = NULL;
    m_features = 18;
    geoFeatures.create(N_sample, m_features, CV_64FC1);
    imgFeatures.create(N_sample, 9 + 3 + 960 + 14, CV_64FC1);
}

ViewPointSet::ViewPointSet(std::vector<glm::vec3> &refCameraPos, std::vector<glm::vec3> &groundPlane)
{
    this->refCameraPos = refCameraPos;
    this->groundPlane = groundPlane;
    N_sample = 100;
    feaGeo = NULL;
    m_features = 18;
    geoFeatures.create(N_sample, m_features, CV_64FC1);
    imgFeatures.create(N_sample, 9 + 3 + 960 + 14, CV_64FC1);
}

ViewPointSet::~ViewPointSet()
{
//    feaGeo = NULL;
    geoFeatures.release();
    imgFeatures.release();
}

void ViewPointSet::setViewpoints()
{
    camPosSample();
    genMVMatrix(mvList);
}

void ViewPointSet::setFeatures(GLWidget *glWidget, int knowAxis)
{
    setViewpoints();
    rectifyMatrix = glWidget->getNowModelMatrix();
    rectifyMatrix = glm::inverse(rectifyMatrix);
    // rectify the camera positions
    if(!knowAxis)
    {
        for(int i=0;i<cameraPos.size();i++)
        {
            glm::vec4 pos = glm::vec4(cameraPos[i].x, cameraPos[i].y, cameraPos[i].z, 1.0);
            pos = rectifyMatrix * pos;
            cameraPos[i].x = pos.x;
            cameraPos[i].y = pos.y;
            cameraPos[i].z = pos.z;
        }
    }
    setGLWidget(glWidget);
    feaGeo->vpRecommendPipLine(mvList, geoFeatures, imgFeatures, knowAxis);
}

void ViewPointSet::copyImgFeatureTo(cv::Mat &imgFea)
{
    imgFeatures.copyTo(imgFea);
}

void ViewPointSet::copyGeoFeatureTo(cv::Mat &geoFea)
{
    geoFeatures.copyTo(geoFea);
}

std::vector<glm::vec3> &ViewPointSet::getCameraPos()
{
    return cameraPos;
}

void ViewPointSet::setGLWidget(GLWidget *glWidget)
{
    feaGeo = new FeaGeo(glWidget);
}

void ViewPointSet::genMVMatrix(std::vector<glm::mat4> &mvList)
{
//    std::cout << "vppoint sample sceneZ " << sceneZ << std::endl;
    for(int i=0;i<cameraPos.size();i++)
    {
        glm::mat4 mv = glm::lookAt(glm::vec3(cameraPos[i].x, cameraPos[i].y, cameraPos[i].z),
                                   glm::vec3(0.f,0.f,0.5f),
                                   glm::vec3(0.f,0.f,1.f));
        mvList.push_back(mv);
    }
}



void ViewPointSet::camPosSample()
{
    srand((int)time(0));
      // random sample with circle
/*
      int counter = N_sample;
//      double r1 = 1.5;
//      double r2 = 3.0;
      double r1 = 1.2;
      double r2 = 1.5;
      double pi = acos(-1.0);
      double theta = pi / 3.0 * 2;
      double biasTheta = pi + pi / 2.0 - theta / 2.0;
      while(counter--)
      {
          float r = (float)rand() / (float) RAND_MAX;
          r = (r2 - r1) * r + r1;
          float tmpTheta = (float) rand() / (float) RAND_MAX;
          tmpTheta = tmpTheta * theta + biasTheta;
          float tmpx = r * cos(tmpTheta);
          float tmpy = r * sin(tmpTheta);
//          cameraPos.push_back(glm::vec3(tmpx, tmpy, sceneZ));
          cameraPos.push_back(glm::vec3(tmpx, tmpy, -0.45));
      }
*/
    // uniform sample in the sector
//    int counter = N_sample;
    double r1 = 1.2;
    double r2 = 1.5;
    double pi = acos(-1.0);
    double theta = pi / 3.0 * 2.0 / 3.0;
    double cameraZ = -0.4;
    double shiftY = 0.0;
    camPosSampleParameter(r1, r2, theta, cameraZ, shiftY);
//    std::cout << "################## cameraZ ############ " << cameraZ << std::endl;
    double biasTheta = pi + pi / 2.0 - theta / 2.0;
    // area of the sector
    double areaSector = theta / 2.0 * (r2 * r2 - r1 * r1);
    // area of the retangle
    double retangle_w = 2.0 * r2 * sin(theta / 2.0);
    double retangle_h = r2 - r1 * cos(theta / 2.0);
    double areaRetangle =  2.0 * r2 * sin(theta / 2.0) * (r2 - r1 * cos(theta / 2.0));

    int counter = int(areaRetangle * N_sample / areaSector);

    // sample_ratio = W / W_step, H / H_step
    double sample_ratio = sqrt(double(counter));
    double w_step = retangle_w / sample_ratio;
    double h_step = retangle_h / sample_ratio;
    glm::vec2 startPoint(- r2 * sin(theta / 2.0), - r1 * cos(theta / 2.0));
    glm::vec2 endPoint(r2 * sin(theta / 2.0), - r2);

    double tmpx,tmpy;
    double tmpTheta, tmpR;
    for(int i = 0; i < sample_ratio; i++)
        for(int j = 0; j < sample_ratio; j++)
        {
            tmpx = startPoint.x + i * w_step;
            tmpy = startPoint.y - j * h_step;
            tmpR = sqrt(tmpx * tmpx + tmpy * tmpy);
            if(tmpR < r1 || tmpR > r2)
                continue;
            // convert to polar coordinates
            tmpTheta = pi + acos( - tmpx / tmpR);
            if(tmpTheta >= biasTheta && tmpTheta <= biasTheta + theta)
            {
                tmpy += shiftY;
                // plane.point(:,3) = p(3) - ( n(1) * ( plane.point(:,1) - p(1) ) + n(2) * ( plane.point(:,2) - p(2) ) ) / n(3);
                cameraZ = groundPlane[1].z - ( groundPlane[0].x * ( tmpx - groundPlane[1].x ) + groundPlane[0].y * ( tmpy - groundPlane[1].y ) ) / groundPlane[0].z;
                cameraPos.push_back(glm::vec3(tmpx, tmpy, cameraZ));
            }
        }
    std::cout << "total sample points "  << cameraPos.size() << std::endl;
}

void ViewPointSet::camPosSampleParameter(double &r1, double &r2, double &theta, double &cameraZ, double &shiftY)
{
    if(refCameraPos.size() == 0)
    {
        r1 = 1.2;
        r2 = 1.5;
        double pi = acos(-1.0);
        theta = pi / 3.0 * 2.0;
        cameraZ = -0.4;
        return;
    }
    else
    {
        r1 = 1e8;
        r2 = -1e8;
        theta /= 2.0;
        double r = 0;
        double tmpTheta;
        double maxTheta = 0;
        glm::vec3 tmpThetaPos = refCameraPos[0];
        cameraZ  = 0.0;
        for(int i = 0; i < refCameraPos.size(); i++)
        {
            cameraZ += refCameraPos[i].z;
            r = sqrt(refCameraPos[i].x * refCameraPos[i].x + refCameraPos[i].y * refCameraPos[i].y);
            tmpTheta  = acos(- refCameraPos[i].y / r);
            if( maxTheta < tmpTheta )
            {
                maxTheta = tmpTheta;
                tmpThetaPos = refCameraPos[i];
            }
        }
        double absX = tmpThetaPos.x > 0 ? tmpThetaPos.x : - tmpThetaPos.x;
        shiftY = absX / tan(theta) - absX / tan(maxTheta);
        if(shiftY < 0)
        {
            shiftY = 0;
            return;
        }
//        r1 = sqrt(tmpR1Pos.x * tmpR1Pos.x + (tmpR1Pos.y - shiftY) * (tmpR1Pos.y - shiftY));
//        r2 = sqrt(tmpR2Pos.x * tmpR2Pos.x + (tmpR2Pos.y - shiftY) * (tmpR2Pos.y - shiftY));
        // compute the r1 and r2
        for(int i=0; i < refCameraPos.size(); i++)
        {
            r = sqrt(refCameraPos[i].x * refCameraPos[i].x + (refCameraPos[i].y - shiftY) * (refCameraPos[i].y - shiftY));
            r1 = r1 < r ? r1 : r;
            r2 = r2 < r ? r : r2;
        }
        cameraZ /= refCameraPos.size();
        return;
    }
}

void ViewPointSet::fillInFeature(int index)
{
    feaGeo->fillInFeature(geoFeatures, index);
}

float ViewPointSet::getSceneZ()
{
    return sceneZ;
}

void ViewPointSet::setRecommendationLocations(cv::Mat &score)
{
    cv::Mat indexScore;
    cv::sortIdx(score, indexScore, CV_SORT_EVERY_COLUMN + CV_SORT_DESCENDING);
    double topRate = 0.2;
    int numsCamera = topRate * score.rows;
//    for(int i=0;i<score.rows;i++)
//        std::cout << score.at<double>(i,0) << std::endl;
    std::vector<int> index;
    std::vector<int> indexNeg;
    for(int i=0; i < numsCamera; i++)
        index.push_back(indexScore.at<int>(i,0));
    for(int i=0; i < numsCamera; i++)
        indexNeg.push_back(indexScore.at<int>(score.rows - i - 1, 0));
//    for(int i=0;i<numsCamera;i++)
//        std::cout << "cameraIndex " << index[i] << " " << indexNeg[i] << std::endl;
    feaGeo->setRecommendationLocationsPos(cameraPos, index);
    feaGeo->setRecommendationLocationsNeg(cameraPos, indexNeg);

//    for(int i=0;i<score.rows;i++)
    //        std::cout << score.at<double>(i,0) << " " << indexScore.at<int>(i,0) << std::endl;
}

void ViewPointSet::setRecommendationLocationsWithRatio(cv::Mat &score)
{
    srand(12134);
    double sumRatio = 0.0;
//    double ratioVal[9] = {0.9,0.8,0.7,0.6,0.5,0.4,0.3,0.2,0.1};
//    double ratioVal[3] = {0.3,0.2,0.1};
    double ratioVal[1] = {0.05};
    std::vector<double> ratio(ratioVal, ratioVal + sizeof(ratioVal) / sizeof(double));
    for(int i=0;i < ratio.size();i++)
        sumRatio += ratio[i];
    cv::Mat indexScore;
    cv::sortIdx(score, indexScore, CV_SORT_EVERY_COLUMN + CV_SORT_DESCENDING);
    double topRate = 0.05;
    int numsCamera = topRate * score.rows;

    std::vector<int> index;
    for(int indexPart = 0; indexPart < ratio.size(); indexPart++)
    {
        int base = indexPart * score.rows / ratio.size();
//        std::cout << "base " << base << std::endl;
        std::set<int> indexSet;
        indexSet.clear();
        std::set<int>::iterator it;
        int num = numsCamera * ratio[indexPart] / sumRatio;
        for(int i = 0; i < numsCamera * ratio[indexPart] / sumRatio; i++)
        {
            int tmpIdx = (float) rand() / (float) RAND_MAX * num;
            it = indexSet.find(tmpIdx);
            if(it == indexSet.end())
            {
                indexSet.insert(tmpIdx);
                index.push_back(indexScore.at<int>(tmpIdx + base, 0));
            }
            else
                i--;
        }
    }

    feaGeo->setRecommendationLocationsPos(cameraPos, index);
}

void ViewPointSet::printScore(cv::Mat &score)
{
    for(int i=0;i<score.rows;i++)
        std::cout << "vp " << i << " " << score.at<double>(i,0) << std::endl;
}
