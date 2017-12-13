#include "viewpointset.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <glm/gtc/matrix_transform.hpp>

ViewPointSet::ViewPointSet()
{
    sceneZ = -0.9;
    N_sample = 500;
    feaGeo = NULL;
    m_features = 18;
    geoFeatures.create(N_sample,m_features,CV_64FC1);
}

ViewPointSet::~ViewPointSet()
{

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
    feaGeo->vpRecommendPipLine(mvList, geoFeatures, knowAxis);
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
                                   glm::vec3(0.f,0.f,0.f),
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
    double theta = pi / 3.0 * 2.0;
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
//            std::cout << "[" << tmpx << ", "<< tmpy << "] tmpTheta " << tmpTheta << " [ " << biasTheta << ", " << biasTheta + theta << "]" << std::endl;
            if(tmpTheta >= biasTheta && tmpTheta <= biasTheta + theta)
                cameraPos.push_back(glm::vec3(tmpx, tmpy, -0.4));
        }
    std::cout << "total sample points "  << cameraPos.size() << std::endl;
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
    double sumRatio = 0.0;
//    double ratioVal[9] = {0.9,0.8,0.7,0.6,0.5,0.4,0.3,0.2,0.1};
    double ratioVal[3] = {0.3,0.2,0.1};
    std::vector<double> ratio(ratioVal, ratioVal + sizeof(ratioVal) / sizeof(double));
    for(int i=0;i < ratio.size();i++)
        sumRatio += ratio[i];
    cv::Mat indexScore;
    cv::sortIdx(score, indexScore, CV_SORT_EVERY_COLUMN + CV_SORT_DESCENDING);
    double topRate = 0.2;
    int numsCamera = topRate * score.rows;

    std::vector<int> index;
    for(int indexPart = 0; indexPart < ratio.size(); indexPart++)
    {
        int base = indexPart * score.rows / ratio.size();
//        std::cout << "base " << base << std::endl;
        for(int i = 0; i < numsCamera * ratio[indexPart] / sumRatio; i++)
        {
//            int tmpIdx = rand()
            int num = numsCamera * ratio[indexPart] / sumRatio;
            int tmpIdx = (float) rand() / (float) RAND_MAX * num;
            index.push_back(indexScore.at<int>(tmpIdx + base, 0));
        }
    }

    feaGeo->setRecommendationLocationsPos(cameraPos, index);
}

void ViewPointSet::printScore(cv::Mat &score)
{
    for(int i=0;i<score.rows;i++)
        std::cout << "vp " << i << " " << score.at<double>(i,0) << std::endl;
}
