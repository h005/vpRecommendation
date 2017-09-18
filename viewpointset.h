#ifndef VIEWPOINTSAMPLE_H
#define VIEWPOINTSAMPLE_H

///
/// \brief The ViewPointSample class
///
/// pipline:
///
/// 1. get the shift and scale parameter of the given model
///
/// 2. sample the camera positions with N points (such as N = 500, 1000)
///
/// 3. the region of the sample viewpoints
///     z = scene.z  x = [-3.0, 3.0] y = [-3.0, -1.5];
///     camera looks at (0,0,0)
///
/// 4. generate the model-view and projection matrix
///
/// 5. assessment the scores and show the position with top 20 % scores
///
///
///

#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include "features/feageo.h"
#include "OpenGLWidget/glwidget.h"


class ViewPointSet
{
public:
    ViewPointSet();

    void setViewpoints();

    void setFeatures(GLWidget *glWidget);

    void copyGeoFeatureTo(cv::Mat &geoFea);

    std::vector<glm::vec2>& getCameraPos();

    float getSceneZ();

    void setRecommendationLocations(cv::Mat &score);

    void printScore(cv::Mat &score);

private:
    void setGLWidget(GLWidget *glWidget);

    void genMVMatrix(std::vector<glm::mat4> &mvList);

    void camPosSample();

    void fillInFeature(int index);

private:

    int N_sample;
    int m_features;
    float sceneZ;

    FeaGeo *feaGeo;

    cv::Mat geoFeatures;

    std::vector<glm::vec2> cameraPos;
    std::vector<glm::mat4> mvList;

};

#endif // VIEWPOINTSAMPLE_H
