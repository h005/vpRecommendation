#ifndef FEAGEO_H
#define FEAGEO_H

///
/// \brief The FeaGeo class
///
/// The geometry feaure handler
///
/// input:
/// 1. Model-View Projection matrix
/// 2. 3D model
///
/// output:
/// geometry feature
///
/// pipline:
/// 1. initial
/// 2. render
/// 3. extract features
///
///

#include <glm/glm.hpp>
#include <opencv.hpp>
#include "OpenGLWidget/glwidget.h"
//#include "viewpointset.h"

#define PI 3.1415926

class FeaGeo
{
public:
    FeaGeo(GLWidget *glWidget);

    void vpRecommendPipLine(std::vector<glm::mat4> &cameraList, cv::Mat &geoFeature, int knowAxis);

    // fill in the modelView and projection matrix
    void initial(glm::mat4 modelView, glm::mat4 projection);
    void setRecommendationLocationsPos(std::vector<glm::vec3> &vpRcameraLocations,
                                    std::vector<int> &index);
    void setRecommendationLocationsNeg(std::vector<glm::vec3> &vpRcameraLocations,
                                    std::vector<int> &index);

    // render the image
    void render();
    void setImgMask(cv::Mat &img, cv::Mat &mask);
    void setParameters();
    void extractFeatures();
    void extractFeaturesPipline();
    void fillInFeature(cv::Mat &features, int index);

private:
    // call these function one by one with th following orders
    void setProjectArea();
    // void setVisSurfaceArea();
    void setViewpointEntropy();
    void setSilhouetteLength();
    void setSilhouetteCE();
    void setBoundingBox3D();
    void setOutLierCount();
    void setBoundingBox3DAbs();
    void setBallCoord();
    void setTiltAngle();

    double getArea2D(CvPoint2D64f *a, CvPoint2D64f *b, CvPoint2D64f *c);
    double getDis2D(CvPoint2D64f *a,CvPoint2D64f *b);
    double getContourCurvature(const std::vector<cv::Point2d> &points, int target);
    float floatAbs(float num);

private:
    GLWidget *glWidget;
    glm::mat4 modelView;
    glm::mat4 projection;

    // three channels
    cv::Mat img;
    // single channel
    cv::Mat mask;
    // verticesMvp and visibleFaces
    std::vector<GLfloat> vertice;
    std::vector<GLuint> face;
    std::vector< std::vector<cv::Point> > contour;
    // model axis
    glm::vec4 model_x;
    glm::vec4 model_y;
    glm::vec4 model_z;
    // outer points
    int outsidePointsNum;

    std::vector<double> fea;

//    ViewPointSet *vpSample;
    std::vector< glm::mat4 > cameraList;
};

#endif // FEAGEO_H
