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
    m_features = 27;
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
//    feaGeo->
    feaGeo->vpRecommendPipLine(mvList, cameraPos, sceneZ);
    feaGeo->extractFeatures();
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
    std::cout << "vppoint sample sceneZ " << sceneZ << std::endl;
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

    glm::vec2 xRange(-3.0, 3.0);
    glm::vec2 yRange(-3.0, -1.5);

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
