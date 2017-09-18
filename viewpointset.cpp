#include "viewpointset.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <glm/gtc/matrix_transform.hpp>

ViewPointSet::ViewPointSet()
{
    sceneZ = -1.0;
    N_sample = 500;
    feaGeo = NULL;
    m_features = 18;
    geoFeatures.create(N_sample,m_features,CV_64FC1);
}

void ViewPointSet::setViewpoints()
{
    camPosSample();
    genMVMatrix(mvList);
}

void ViewPointSet::setFeatures(GLWidget *glWidget)
{
    setViewpoints();
    setGLWidget(glWidget);
    feaGeo->vpRecommendPipLine(mvList, geoFeatures);
}

void ViewPointSet::copyGeoFeatureTo(cv::Mat &geoFea)
{
    geoFeatures.copyTo(geoFea);
}

std::vector<glm::vec2> &ViewPointSet::getCameraPos()
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
        glm::mat4 mv = glm::lookAt(glm::vec3(cameraPos[i].x, cameraPos[i].y, sceneZ),
                                   glm::vec3(0.f,0.f,0.f),
                                   glm::vec3(0.f,0.f,1.f));
        mvList.push_back(mv);
    }
}

void ViewPointSet::camPosSample()
{
    srand((int)time(0));

    glm::vec2 xRange(-1.5, 1.5);
    glm::vec2 yRange(-3.0, -0.8);

    int counter = N_sample;
    while(counter--)
    {
        float tmpx = (float)rand() / (float)RAND_MAX;
        tmpx = tmpx * (xRange.y - xRange.x) + xRange.x;
        float tmpy = (float)rand() / (float)RAND_MAX;
        tmpy = tmpy * (yRange.y - yRange.x) + yRange.x;
        cameraPos.push_back(glm::vec2(tmpx,tmpy));
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
    std::vector<int> index;
    for(int i=0; i < numsCamera; i++)
        index.push_back(indexScore.at<int>(i,0));
    feaGeo->setRecommendationLocations(cameraPos, sceneZ, index);
//    for(int i=0;i<score.rows;i++)
//        std::cout << score.at<double>(i,0) << " " << indexScore.at<int>(i,0) << std::endl;
}

void ViewPointSet::printScore(cv::Mat &score)
{
    for(int i=0;i<score.rows;i++)
        std::cout << "vp " << i << " " << score.at<double>(i,0) << std::endl;
}
