#include "feageo.h"
#include <glm/gtx/string_cast.hpp>

#include <QPlainTextEdit>
extern QPlainTextEdit *messageWidget;

FeaGeo::FeaGeo(GLWidget *glWidget)
{
    this->glWidget = glWidget;
}

void FeaGeo::vpRecommendPipLine(std::vector<glm::mat4> &cameraList,
                                cv::Mat &geoFeature,
                                int knowAxis)
{
    glm::mat4 originalMV;
    if(knowAxis)
        originalMV = glWidget->getOriginalMatrix();
    else
        originalMV = glWidget->getNowMatrix();
    for(int i=0;i<cameraList.size();i++)
    {
        initial(cameraList[i] * originalMV, glWidget->getProjMatrix());
        if(!((i + 1) % 50))
        {
            messageWidget->appendPlainText("vpRecommendation sample points " + QString::number(i+1));
            messageWidget->repaint();
        }
//            std::cout << "vpRecommendation .... " << i << std::endl;
//        std::cout << glm::to_string(cameraList[i]) << std::endl;
        render();
        setImgMask(img,mask);
        setParameters();
        extractFeatures();
//        std::cout << "geofea size " << fea.size() << std::endl;
//        std::cout << "vpRecommendation " << i << std::endl;
        fillInFeature(geoFeature,i);
    }
}

void FeaGeo::initial(glm::mat4 modelView, glm::mat4 projection)
{
    this->modelView = modelView;
    this->projection = projection;
    glWidget->setVpRecommendationMatrix(modelView);
    //    std::cout << modelview.
}

void FeaGeo::setRecommendationLocationsPos(std::vector<glm::vec3> &vpRcameraLocations,
                                           std::vector<int> &index)
{
    glWidget->setRecommendationLocationsPos(vpRcameraLocations, index);
}

void FeaGeo::setRecommendationLocationsNeg(std::vector<glm::vec3> &vpRcameraLocations,
                                           std::vector<int> &index)
{
    glWidget->setRecommendationLocationsNeg(vpRcameraLocations, index);
}

void FeaGeo::render()
{
    glWidget->render();
}

void FeaGeo::setImgMask(cv::Mat &img, cv::Mat &mask)
{
    glWidget->setImgMask(img, mask);
}

///
/// \brief FeaGeo::setParameters
/// set the parameters below:
///
/// std::vector<GLfloat> &vertice,
/// std::vector<GLuint> &face,
/// glm::vec4 &model_x,
/// glm::vec4 &model_y,
/// glm::vec4 &model_z,
/// int &outsidePointsNum
///
///
void FeaGeo::setParameters()
{
    glWidget->setParameters(vertice, face, model_x, model_y, model_z, outsidePointsNum);
}

void FeaGeo::extractFeatures()
{
    fea.clear();
    setProjectArea();
    setViewpointEntropy();
    setSilhouetteLength();
    setSilhouetteCE();
    setTiltAngle();
//    setBoundingBox3D();
    setOutLierCount();
    setBoundingBox3DAbs();
    setBallCoord();

}

void FeaGeo::extractFeaturesPipline()
{
    initial(glWidget->getMVMatrix(),glWidget->getProjMatrix());
    render();
    setImgMask(img, mask);
    setParameters();
    extractFeatures();

    std::cout << "render done " << std::endl;
}

void FeaGeo::setProjectArea()
{
    double res = 0.0;
    for(int i=0;i<mask.rows;i++)
        for(int j=0;j<mask.cols;j++)
            if(mask.at<uchar>(i,j)!=255)
                res++;
    res /= mask.cols * mask.rows;
    fea.push_back(res);
//    std::cout << "project area " << res << std::endl;
}

void FeaGeo::setViewpointEntropy()
{
    double res = 0.0;
    double area = 0.0;
    double totalArea = img.cols * img.rows;
    for(int i=0;i<face.size();i+=3)
    {
        if(vertice.size() <= face[i] * 3)
            continue;
        if(vertice.size() <= face[i+1] * 3)
            continue;
        if(vertice.size() <= face[i+2] * 3)
            continue;
        CvPoint2D64f a = cvPoint2D64f(vertice[face[i]*3],vertice[face[i]*3+1]);
        CvPoint2D64f b = cvPoint2D64f(vertice[face[i+1]*3],vertice[face[i+1]*3+1]);
        CvPoint2D64f c = cvPoint2D64f(vertice[face[i+2]*3],vertice[face[i+2]*3+1]);
        area = getArea2D(&a, &b, &c);
        if(area > 0)
            res += area/totalArea * log2(area/totalArea);
    }
    // background
    if((1.0 - fea[0]) > 0)
        res += (1.0 - fea[0]) * log2(1.0 - fea[0]);
    res = -res;
    fea.push_back(res);
//    std::cout << "viewpoint entropy done " << res << std::endl;
}

double FeaGeo::getArea2D(CvPoint2D64f *a, CvPoint2D64f *b, CvPoint2D64f *c)
{
    CvPoint2D64f ab = cvPoint2D64f(b->x - a->x, b->y - a->y);
    CvPoint2D64f ac = cvPoint2D64f(c->x - a->x, c->y - a->y);
    double area = ab.x * ac.y - ab.y * ac.x;
    area = area > 0 ? area : -area;
    area /= 2.0;
    return area;
}

void FeaGeo::setSilhouetteLength()
{
    double res = 0.0;
    // ref http://docs.opencv.org/2.4/doc/tutorials/imgproc/shapedescriptors/find_contours/find_contours.html
//    cv::Mat gray;

    // ref http://docs.opencv.org/2.4/modules/imgproc/doc/miscellaneous_transformations.html?highlight=threshold#threshold
    // 这个一定要二值化，图像本身就基本都是白色，直接提取轮廓是拿不到结果的
//    cv::threshold( image, gray, 254, 255.0,cv::THRESH_BINARY_INV );

    std::vector<cv::Vec4i> hierarchy;
//    cv::findContours(gray,contour,hierarchy,CV_RETR_LIST  ,CV_CHAIN_APPROX_NONE );
    cv::findContours(mask,contour,hierarchy,CV_RETR_LIST  ,CV_CHAIN_APPROX_NONE );

    if(contour.size())
        for(int i=0;i<contour.size();i++)
        {
            res += cv::arcLength(contour[i],true);
        }
    else
        res = 0.0;
    // scale
    res = res / mask.cols / mask.rows;

    fea.push_back(res);
    std::vector<cv::Vec4i>().swap(hierarchy);
//    std::cout << "silhouette length " << res << std::endl;
}

void FeaGeo::setSilhouetteCE()
{
    double res0 = 0.0;
    double res1 = 0.0;
    double dis = 0.0;

    //    example
    //    abcdefghabcde
    //     ^  ->  ^
    //    abc -> bcd -> def

    for(int k = 0;k<contour.size();k++)
    {
        for(int i=0;i<contour[k].size();i++)
        {
            cv::Point a0 = contour[k][i];
            cv::Point b0 = contour[k][(i+1)%contour[k].size()];
            cv::Point c0 = contour[k][(i+2)%contour[k].size()];
            CvPoint2D64f a = cvPoint2D64f((double)a0.x,(double)a0.y);
            CvPoint2D64f b = cvPoint2D64f((double)b0.x,(double)b0.y);
            CvPoint2D64f c = cvPoint2D64f((double)c0.x,(double)c0.y);

            std::vector<cv::Point2d> points;
            points.push_back(cv::Point2d(a.x, a.y));
            points.push_back(cv::Point2d(b.x, b.y));
            points.push_back(cv::Point2d(c.x, c.y));


                dis = getDis2D(&a,&b) + getDis2D(&b,&c);

            double curvab = getContourCurvature(points,1);
            if (std::isnan(curvab)) {
    //            qDebug()<<a.x<<" "<<a.y<<endl;
    //            qDebug()<<b.x<<" "<<b.y<<endl;
    //            qDebug()<<c.x<<" "<<c.y<<endl;
    //            assert(0);
            }
            else
            {
                res0 += floatAbs(curvab);
                res1 += curvab*curvab;
            }

    //        qDebug()<<"curvature a"<<curva<<" "<<floatAbs(curvab)<< " "<<floatAbs(curvab) - floatAbs(curva)<<endl;
        }
    }
    fea.push_back(res0);
    fea.push_back(res1);

//    std::cout << "fea silhouette curvature " << res0 << std::endl;
//    std::cout << "fea silhouette curvature Extreme " << res1 << std::endl;

}

void FeaGeo::setBoundingBox3D()
{
    double dotval = 0.0;
    double cosTheta = 0.0;
    double theta = 0.0;
    glm::vec4 axisx = glm::vec4(1,0,0,0);
    glm::vec4 axisy = glm::vec4(0,1,0,0);
    glm::vec4 axisz = glm::vec4(0,0,1,0);

//    std::cout << "bounding box 3d "<<std::endl;
    // model_x x
    dotval = glm::dot(model_x,axisx);
    cosTheta = dotval / (glm::length(model_x) * glm::length(axisx));
    theta = acos(cosTheta);
    fea.push_back(theta);
    // model_x y
    dotval = glm::dot(model_x,axisy);
    cosTheta = dotval / (glm::length(model_x) * glm::length(axisy));
    theta = acos(cosTheta);
    fea.push_back(theta);
    // model_x z
    dotval = glm::dot(model_x,axisz);
    cosTheta = dotval / (glm::length(model_x) * glm::length(axisz));
    theta = acos(cosTheta);
    fea.push_back(theta);
    // model_y x
    dotval = glm::dot(model_y,axisx);
    cosTheta = dotval / (glm::length(model_y) * glm::length(axisx));
    theta = acos(cosTheta);
    fea.push_back(theta);
    // model_y y
    dotval = glm::dot(model_y,axisy);
    cosTheta = dotval / (glm::length(model_y) * glm::length(axisy));
    theta = acos(cosTheta);
    fea.push_back(theta);
    // model_y z
    dotval = glm::dot(model_y,axisz);
    cosTheta = dotval / (glm::length(model_y) * glm::length(axisz));
    theta = acos(cosTheta);
    fea.push_back(theta);
    // model_z x
    dotval = glm::dot(model_z,axisx);
    cosTheta = dotval / (glm::length(model_z) * glm::length(axisx));
    theta = acos(cosTheta);
    fea.push_back(theta);
    // model_z y
    dotval = glm::dot(model_z,axisy);
    cosTheta = dotval / (glm::length(model_z) * glm::length(axisy));
    theta = acos(cosTheta);
    fea.push_back(theta);
    // model_z z
    dotval = glm::dot(model_z,axisz);
    cosTheta = dotval / (glm::length(model_z) * glm::length(axisz));
    theta = acos(cosTheta);
    fea.push_back(theta);
}

void FeaGeo::setOutLierCount()
{
    double res = (double) outsidePointsNum / vertice.size();
    fea.push_back(res);

//    std::cout << "outlier count " << res << std::endl;

}

void FeaGeo::setBoundingBox3DAbs()
{
    double dotval = 0.0;
    double cosTheta = 0.0;
    double theta = 0.0;
    glm::vec4 axisx = glm::vec4(1,0,0,0);
    glm::vec4 axisy = glm::vec4(0,1,0,0);
    glm::vec4 axisz = glm::vec4(0,0,1,0);

    // p_model_x x
    dotval = glm::dot(model_x,axisx);
    cosTheta = dotval / (glm::length(model_x) * glm::length(axisx));
    cosTheta = floatAbs(cosTheta);
    theta = acos(cosTheta);
    fea.push_back(theta);

    // p_model_x y
    dotval = glm::dot(model_x,axisy);
    cosTheta = dotval / (glm::length(model_x) * glm::length(axisy));
    cosTheta = floatAbs(cosTheta);
    theta = acos(cosTheta);
    fea.push_back(theta);

    // p_model_x z
    dotval = glm::dot(model_x,axisz);
    cosTheta = dotval / (glm::length(model_x) * glm::length(axisz));
    cosTheta = floatAbs(cosTheta);
    theta = acos(cosTheta);
    fea.push_back(theta);

    // p_model_y x
    dotval = glm::dot(model_y,axisx);
    cosTheta = dotval / (glm::length(model_y) * glm::length(axisx));
    cosTheta = floatAbs(cosTheta);
    theta = acos(cosTheta);
    fea.push_back(theta);

    // p_model_y y
    dotval = glm::dot(model_y,axisy);
    cosTheta = dotval / (glm::length(model_y) * glm::length(axisy));
    cosTheta = floatAbs(cosTheta);
    theta = acos(cosTheta);
    fea.push_back(theta);

    // p_model_y z
    dotval = glm::dot(model_y,axisz);
    cosTheta = dotval / (glm::length(model_y) * glm::length(axisz));
    cosTheta = floatAbs(cosTheta);
    theta = acos(cosTheta);
    fea.push_back(theta);

    // p_model_z x
    dotval = glm::dot(model_z,axisx);
    cosTheta = dotval / (glm::length(model_z) * glm::length(axisx));
    cosTheta = floatAbs(cosTheta);
    theta = acos(cosTheta);
    fea.push_back(theta);

    // p_model_z y
    dotval = glm::dot(model_z,axisy);
    cosTheta = dotval / (glm::length(model_z) * glm::length(axisy));
    cosTheta = floatAbs(cosTheta);
    theta = acos(cosTheta);
    fea.push_back(theta);

    // p_model_z z
    dotval = glm::dot(model_z,axisz);
    cosTheta = dotval / (glm::length(model_z) * glm::length(axisz));
    cosTheta = floatAbs(cosTheta);
    theta = acos(cosTheta);
    fea.push_back(theta);
}

void FeaGeo::setBallCoord()
{
    // ref https://zh.wikipedia.org/wiki/%E7%90%83%E5%BA%A7%E6%A8%99%E7%B3%BB
    glm::mat4 mv = modelView;
    glm::vec4 camera = glm::vec4(0,0,-1,0);
    camera = glm::inverse(mv) * camera;
    camera[3] = 0.0;
//    double r = glm::length(camera);
    double theta = PI + atan(sqrt(camera[0] * camera[0] + camera[1] * camera[1]) / camera[2]);
    double fani;
    if(camera[0] == 0)
        fani = PI/2.0;
    else
        fani = atan(camera[1] / camera[0]);
//    std::cout << "ball coord" << std::endl;
//    std::cout << theta << " " << fani << std::endl;
    fea.push_back(theta);
    fani = floatAbs(fani);
    fea.push_back(fani);
//    std::cout << "ballCoord " << theta << " " << fani << std::endl;
}

void FeaGeo::setTiltAngle()
{
//    glm::mat4 modelView;
    glm::vec3 t = glm::vec3(modelView[3]);
    glm::mat3 R = glm::mat3(modelView);

    glm::vec3 eye = -glm::transpose(R) * t;
    glm::vec3 center = glm::normalize(glm::transpose(R) * glm::vec3(0.f, 0.f, -1.f)) + eye;
    glm::vec3 up = glm::normalize(glm::transpose(R) * glm::vec3(0.f, 1.f, 0.f));

    // ignore the tilt in y direction, so set the y value as 0
    up[1] = 0.f;

    glm::vec3 z_angles(0.f,0.f,1.f);
    // cos theta
    double angle;
    if(glm::length(z_angles) == 0 || glm::length(up) == 0)
        angle = -1;
    else
        angle = glm::dot(z_angles,up) / glm::length(z_angles) / glm::length(up);

    fea.push_back(angle);

//    std::cout << "tiltAngle " << angle << std::endl;

}

void FeaGeo::fillInFeature(cv::Mat &features, int index)
{
    for(int i=0;i<features.cols;i++)
        features.at<double>(index,i) = fea[i];
}

double FeaGeo::getDis2D(CvPoint2D64f *a, CvPoint2D64f *b)
{
    double dx = (a->x-b->x);
    double dy = (a->y-b->y);
    return sqrt(dx*dx+dy*dy);
}

typedef long double LD;
double FeaGeo::getContourCurvature(const std::vector<cv::Point2d> &points, int target)
{
    assert(points.size() == 3);

    double T[3];
    for (int i = 0; i < 3; i++) {
        double t = cv::norm(points[target] - points[i]);
        T[i] = target < i ? t : -t;
    }
    cv::Mat M(3, 3, CV_64F);
    for (int i = 0; i < 3; i++) {
        M.at<double>(i, 0) = 1;
        M.at<double>(i, 1) = T[i];
        M.at<double>(i, 2) = T[i] * T[i];
    }
    cv::Mat invM = M.inv();

    cv::Mat X(3, 1, CV_64F), Y(3, 1, CV_64F);
    for (int i = 0; i < 3; i++) {
        X.at<double>(i, 0) = points[i].x;
        Y.at<double>(i, 0) = points[i].y;
    }

    cv::Mat a, b;
    a = invM * X;
    b = invM * Y;

    LD up = (LD)2 * (a.at<double>(1, 0) * b.at <double>(2, 0) - a.at<double>(2, 0) * b.at <double>(1, 0));
    LD down = pow((LD)a.at<double>(1, 0) * a.at<double>(1, 0) + (LD)b.at <double>(1, 0) * b.at <double>(1, 0), 1.5);
    LD frac = up / down;

    return (double)frac;
}

float FeaGeo::floatAbs(float num)
{
    return num < 0 ? -num : num;
}

