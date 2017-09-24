/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "glwidget.h"
#include <iostream>
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <qdebug.h>

#include "RenderObject/pointsmatchrelation.h"
#include "OpenGLWidget/shader.hpp"

GLWidget::GLWidget(const QString &modelPath, QWidget *parent)
    : DragableWidget(parent),
      m_relation(NULL),
      m_sphereProgramID(0)
{
    model.load(modelPath.toLocal8Bit().data());
    cameraModelPos.load("./camera/camera2.obj");
    cameraModelNeg.load("./camera/camera3.obj");
    std::pair<GLfloat, glm::mat4> scaleAndShift = model.recommandScaleAndShift();
    m_scaleBeforeRender = scaleAndShift.first;
    m_shiftBeforeRender = scaleAndShift.second;
    cameraPos = glm::vec3(0.f,0.f,0.f);
    flag_move = false;
//    camPosLength = 50000.f;
    camPosLength = 3.f;
    rate = 0.05;
//    std::cout << "glwidget initial ... " << std::endl;
//    m_relation = new PointsMatchRelation("1.txt");
}

GLWidget::~GLWidget()
{
    cleanup();
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(400, 400);
}

QSize GLWidget::sizeHint() const
{
    return QSize(400, 400);
}

static void qNormalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}

void GLWidget::cleanup()
{
    // 仅清理该子类生成的对象
    makeCurrent();

    if (m_sphereProgramID) {
        glDeleteProgram(m_sphereProgramID);
        m_sphereProgramID = 0;
    }
    sphere.cleanup();
    model.cleanUp();
    cameraModelPos.cleanUp();
    cameraModelNeg.cleanUp();
    clearVpRcameraLocations();

    doneCurrent();
}

void GLWidget::initializeGL()
{
    // http://stackoverflow.com/a/8303331
    glewExperimental = GL_TRUE;
    
    GLenum err = glewInit();
    assert(err == GLEW_OK);
    // In this example the widget's corresponding top-level window can change
    // several times during the widget's lifetime. Whenever this happens, the
    // QOpenGLWidget's associated context is destroyed and a new one is created.
    // Therefore we have to be prepared to clean up the resources on the
    // aboutToBeDestroyed() signal, instead of the destructor. The emission of
    // the signal will be followed by an invocation of initializeGL() where we
    // can recreate all resources.
    initializeOpenGLFunctions();
//    printf("OpenGL version... is (%s)\n", glGetString(GL_VERSION));
    glClearColor(0.368, 0.368, 0.733, m_transparent ? 0 : 1);
//    glClearColor(1.0, 1.0, 1.0, m_transparent ? 0 : 1);
//    glEnable(GL_COLOR_MATERIAL);
//    glColorMaterial(GL_FRONT,GL_AMBIENT);
//    glColor3f(1.0,1.0,1.0);
    // Our camera never changes in this example.
    // Equal to:
    // m_camera = glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, -3.f));
    m_camera = glm::lookAt(glm::vec3(0.f, 0.f, camPosLength), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));

    // load data for model rendering
    model.bindDataToGL();
    cameraModelPos.bindDataToGL();
    cameraModelNeg.bindDataToGL();

    // link program for drawing sphere
    m_sphereProgramID = LoadShaders("shader/sphereShader.vert", "shader/sphereShader.frag");
    GLuint vertexPosition_modelspaceID = glGetAttribLocation(m_sphereProgramID, "vertexPosition_modelspace");

    GLuint args[] = {vertexPosition_modelspaceID};
    sphere.bindDataToGL(args);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    // 默认开启背面剔除:GL_CULL_FACE

    // 计算modelView矩阵
    glm::mat4 modelViewMatrix = m_camera * getModelMatrix();


    // 绘制模型
    model.draw(modelViewMatrix, m_proj);

    // 绘制模型上被选择的点

//    std::vector<glm::vec3> &points = m_relation->getModelPoints();

    if (vpRcameraLocationsPos.size() > 0) {
        glUseProgram(m_sphereProgramID);
//        std::cout << "draw pos cameras " << vpRcameraLocationsPos.size() << std::endl;
        std::vector<glm::vec3>::iterator it;
        for (it = vpRcameraLocationsPos.begin(); it != vpRcameraLocationsPos.end(); it++) {
            // multiple point's position
            glm::mat4 pointMV = glm::translate(modelViewMatrix, *it);
            pointMV = glm::scale(pointMV, glm::vec3(0.45 / m_scaleBeforeRender));
            cameraModelPos.draw(pointMV, m_proj);
        }
    }
    if (vpRcameraLocationsNeg.size() > 0) {
        glUseProgram(m_sphereProgramID);
//        std::cout << "draw neg cameras " << vpRcameraLocationsNeg.size() << std::endl;
        std::vector<glm::vec3>::iterator it;
        for (it = vpRcameraLocationsNeg.begin(); it != vpRcameraLocationsNeg.end(); it++) {
            // multiple point's position
            glm::mat4 pointMV = glm::translate(modelViewMatrix, *it);
            pointMV = glm::scale(pointMV, glm::vec3(0.45 / m_scaleBeforeRender));
            cameraModelNeg.draw(pointMV, m_proj);
        }
    }

}

void GLWidget::resizeGL(int w, int h)
{
    m_proj = glm::perspective(glm::pi<float>() / 3, GLfloat(w) / h, 0.01f, 1000.0f);
}

glm::mat4 GLWidget::getModelViewMatrix()
{
    return m_camera * getModelMatrix();
}

glm::mat4 GLWidget::getModelMatrix()
{
    return DragableWidget::getModelMatrix() * glm::scale(glm::mat4(1.f), glm::vec3(m_scaleBeforeRender)) * m_shiftBeforeRender;
}

void GLWidget::keyPressEvent(QKeyEvent *e)
{
//    std::cout << "key Pressed "<< std::endl;
//    Qt::Key_Left
    if(e->key() == Qt::Key_Left)
    {
        cameraPos.r += rate * camPosLength;
//        std::cout << "left arrow" << std::endl;
    }
    else if(e->key() == Qt::Key_Right)
    {
        cameraPos.r -= rate * camPosLength;
//        std::cout << "right arrow" << std::endl;
    }
    else if(e->key() == Qt::Key_Up)
    {
        cameraPos.g -= rate * camPosLength;
//        std::cout << "up arrow" << std::endl;
    }
    else if(e->key() == Qt::Key_Down)
    {
        cameraPos.g += rate * camPosLength;
//        std::cout << "down arrow" << std::endl;
    }
//    if(cameraPos.g < 0)
//        cameraPos.g = 0;
//    if(cameraPos.r < 0)
//        cameraPos.r = 0;
    m_camera = glm::lookAt(glm::vec3(0.f, 0.f, camPosLength) + cameraPos, glm::vec3(0.f) + cameraPos, glm::vec3(0.f, 1.f, 0.f));
    update();
}

void GLWidget::mousePressEvent(QMouseEvent *e)
{
    DragableWidget::mousePressEvent(e);
    if(e->button() ==  Qt::MiddleButton)
    {
        flag_move = true;
        m_lastPos = e->pos();
    }
    if(e->button() == Qt::RightButton)
    {
        addPoint(e->pos());
    }
}

void GLWidget::mouseReleaseEvent(QMouseEvent *e)
{
    DragableWidget::mouseReleaseEvent(e);
    flag_move = false;
}

void GLWidget::mouseMoveEvent(QMouseEvent *e)
{
    DragableWidget::mouseMoveEvent(e);
    if(flag_move)
    {
        QPointF pos = m_lastPos - e->pos();
        cameraPos.r += pos.rx() * rate;
        cameraPos.g -= pos.ry() * rate;
        m_camera = glm::lookAt(glm::vec3(0.f, 0.f, camPosLength) + cameraPos, glm::vec3(0.f) + cameraPos, glm::vec3(0.f, 1.f, 0.f));
        update();
    }
}




int GLWidget::addPoint(const QPoint &p) {
    makeCurrent();

    std::vector<glm::vec3> &points = m_relation->getModelPoints();
    GLfloat x = p.x();
    GLfloat y = p.y();

    GLint viewport[4];
    GLdouble object_x,object_y,object_z;
    GLfloat realy, winZ = 0;

    glGetIntegerv(GL_VIEWPORT, viewport);
    realy=(GLfloat)viewport[3] - (GLfloat)y;
    glReadBuffer(GL_BACK);
    glReadPixels(x,int(realy),1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&winZ);
    if (winZ < 1 - 1e-5) {
        glm::mat4 modelViewMatrix = getModelViewMatrix();
        glm::dmat4 mvDouble, projDouble;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                mvDouble[i][j] = modelViewMatrix[i][j];
                projDouble[i][j] = m_proj[i][j];
            }
        }
        gluUnProject((GLdouble)x,(GLdouble)realy,winZ, glm::value_ptr(mvDouble), glm::value_ptr(projDouble),viewport,&object_x,&object_y,&object_z);
        points.push_back(glm::vec3(object_x, object_y, object_z));
        std::cout << "picked points " << object_x << " " << object_y << " " << object_z << std::endl;
    }

    doneCurrent();
    update();
    return points.size();
}

bool GLWidget::removeLastPoint() {
    std::vector<glm::vec3> &points = m_relation->getModelPoints();
    if (points.size() > 0) {
        points.pop_back();
        update();
        return true;
    } else
        return false;
}

void GLWidget::setImgMask(cv::Mat &img, cv::Mat &mask)
{
    makeCurrent();
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    int p_width,p_height;
    p_width = viewport[3] - viewport[1];
    p_height = viewport[2] - viewport[0];
    GLfloat *p_img = new GLfloat[p_width * p_height];
    glReadBuffer(GL_BACK_LEFT);
//    glReadBuffer(GL_BACK);
    glReadPixels(0,0,p_height,p_width,GL_DEPTH_COMPONENT,GL_FLOAT,p_img);

    float min = 1.0;
    for(int i=0;i<viewport[2]*viewport[3];i++)
        min = min < p_img[i] ? min : p_img[i];

    // mask is the depthImg
//    cv::Mat depthImg = cv::Mat((viewport[3] - viewport[1]),
//            (viewport[2] - viewport[0]),
//            CV_32FC1,
//            p_img);
    mask = cv::Mat((viewport[3] - viewport[1]),
            (viewport[2] - viewport[0]),
            CV_32FC1,
            p_img);

    cv::flip(mask,mask,0);
    cv::resize(mask,mask,cv::Size(p_height,p_width));
    mask.convertTo(mask,CV_8UC1,255.0 / (1.0 - min), 255.0 * min / (min - 1.0));

    // show the image
//    cv::namedWindow("depthImg");
//    cv::imshow("depthImg",mask);
//    cv::waitKey(0);

    GLubyte *p_rgbImg =
            new GLubyte[(viewport[2]-viewport[0])
            *(viewport[3]-viewport[1])*4];
    glReadBuffer(GL_BACK);
    glReadPixels(0,
                 0,
                 (viewport[2] - viewport[0]),
                 (viewport[3] - viewport[1]),
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 p_rgbImg);

//    cv::Mat rgbImg = cv::Mat((viewport[3] - viewport[1]),
//            (viewport[2] - viewport[0]),
//            CV_8UC4,p_rgbImg);

    img = cv::Mat((viewport[3] - viewport[1]),
            (viewport[2] - viewport[0]),
            CV_8UC4,p_rgbImg);

    cv::flip(img,img,0);
    cv::resize(img, img, cv::Size(p_height,p_width));
    cv::cvtColor(img,img,CV_RGBA2BGR);
    img.convertTo(img,CV_8UC3);

    // show the rendered image
//    cv::namedWindow("rgbImg");
//    cv::imshow("rgbImg",img);
//    cv::waitKey(0);

    doneCurrent();

    delete p_img;
    delete p_rgbImg;
}

void GLWidget::setVpRecommendationMatrix(glm::mat4 &vpRecommendationMatrix)
{
    this->vpRecommendationMatrix = vpRecommendationMatrix;
}

void GLWidget::setRecommendationLocationsPos(std::vector<glm::vec2> &vpRcameraLocations,
                                          float sceneZ,
                                          std::vector<int> &index)
{
    vpRcameraLocationsPos.clear();
    glm::mat4 shift_scale = getOriginalMatrix();
//    this->vpRcameraLocations = vpRcameraLocations;
    for(int i=0;i<index.size();i++)
    {
        glm::vec4 pos(vpRcameraLocations[index[i]].x,vpRcameraLocations[index[i]].y,sceneZ,1.0);
        pos = glm::inverse(shift_scale) * pos;
        this->vpRcameraLocationsPos.push_back(glm::vec3(pos.x, pos.y, pos.z));
    }
}

void GLWidget::setRecommendationLocationsNeg(std::vector<glm::vec2> &vpRcameraLocations,
                                             float sceneZ,
                                             std::vector<int> &index)
{
    vpRcameraLocationsNeg.clear();
    glm::mat4 shift_scale = getOriginalMatrix();
    for(int i=0;i<index.size();i++)
    {
        glm::vec4 pos(vpRcameraLocations[index[i]].x,vpRcameraLocations[index[i]].y,sceneZ,1.0);
        pos = glm::inverse(shift_scale) * pos;
        this->vpRcameraLocationsNeg.push_back(glm::vec3(pos.x, pos.y, pos.z));
    }
}

void GLWidget::clearVpRcameraLocations()
{
    vpRcameraLocationsPos.clear();
    vpRcameraLocationsNeg.clear();
}

glm::mat4 GLWidget::getOriginalMatrix()
{
    return glm::scale(glm::mat4(1.f), glm::vec3(m_scaleBeforeRender)) * m_shiftBeforeRender;
}

glm::mat4 GLWidget::getMVMatrix()
{
    return m_camera * getModelMatrix();
}

glm::mat4 GLWidget::getProjMatrix()
{
    return m_proj;
}

void GLWidget::render()
{
    makeCurrent();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    // 计算modelView矩阵 it works
    glm::mat4 modelViewMatrix = vpRecommendationMatrix;

    // 绘制模型
    model.draw(modelViewMatrix, m_proj);
//    cameraModel.dra
//    update();
    doneCurrent();
}

///
/// \brief GLWidget::setVerticesFaces
/// \param vertice is the vertex after model-view and projection transformation
/// \param faces
///
void GLWidget::setParameters(std::vector<GLfloat> &vertice,
                                          std::vector<GLuint> &indices,
                                          glm::vec4 &p_model_x,
                                          glm::vec4 &p_model_y,
                                          glm::vec4 &p_model_z,
                                          int &p_outsidePointsNum)
{
    std::vector<GLuint> p_VisibleFaces;
    p_VisibleFaces.clear();
//    glm::vec4 p_model_x;
//    glm::vec4 p_model_y;
//    glm::vec4 p_model_z;
    float p_xmax,p_xmin;
    float p_ymax,p_ymin;
    float p_zmax,p_zmin;
    std::vector<bool> p_isVertexVisible;
    p_isVertexVisible.clear();
    p_outsidePointsNum = 0;
    // p_vertice is the original vertex int the model
    std::vector<GLfloat> p_vertices;
    model.getVerticesAndFaces(p_vertices, indices);

    makeCurrent();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);

    glm::mat4 modelViewMatrix = getModelViewMatrix();
    glm::mat4 mvp = m_proj * modelViewMatrix;
    int p_height = viewport[3]-viewport[1];
    int p_width = viewport[2]-viewport[0];
    GLfloat *p_img = new GLfloat[p_width*p_height];
    glReadBuffer(GL_BACK);
    glReadPixels(0,0,p_width,p_height,GL_DEPTH_COMPONENT,GL_FLOAT,p_img);

    int visibleVertexCount = 0;

    GLfloat xmin,xmax,ymin,ymax,zmin,zmax;

    xmin = xmax = p_vertices[0];
    ymin = ymax = p_vertices[1];
    zmin = zmax = p_vertices[2];

    for(int i=0;i<p_vertices.size();i+=3)
    {
        glm::vec4 position = mvp * glm::vec4(p_vertices[i],p_vertices[i+1],p_vertices[i+2],1.0);
        position = position / position.w;


        xmin = xmin < p_vertices[i] ? xmin : p_vertices[i];
        xmax = xmax > p_vertices[i] ? xmax : p_vertices[i];
        ymin = ymin < p_vertices[i+1] ? ymin : p_vertices[i+1];
        ymax = ymax > p_vertices[i+1] ? ymax : p_vertices[i+1];
        zmin = zmin < p_vertices[i+2] ? zmin : p_vertices[i+2];
        zmax = zmax > p_vertices[i+2] ? zmax : p_vertices[i+2];
        // 看来读到的z-buffer并不是position.z，而是将position.z变换到[0, 1]之间
        // ref http://gamedev.stackexchange.com/a/18858
        GLfloat finalZ = position.z * 0.5 + 0.5;


        // 假设所有点都在裁剪平面内，1.off符合
        // TODO: position.x, position.y的边界检查
        GLfloat ax = (position.x + 1) / 2 * viewport[2];
        GLfloat ay = (position.y + 1) / 2 * viewport[3];
        vertice.push_back(ax);
        vertice.push_back(ay);
        vertice.push_back(finalZ);
        bool isVisible = false;

        bool pointOutsideScreen = true;
        // 在3*3邻域内找相似的深度值
        for (int i = -1; i <= 1; i++){

            for (int j = -1; j <= 1; j++) {

                int x = (int)ax + i, y = (int)ay + j;
                if (x >= 0 && x < p_width && y >= 0 && y < p_height) {
                    pointOutsideScreen = false;
                    GLfloat winZ = p_img[y * p_height + x];

                    // 它们的z-buffer值相差不大，表示这是一个可见点
                    GLfloat zAbs = winZ - finalZ > 0 ? winZ - finalZ : finalZ - winZ;
//                    std::cout << " z buffer value " << zAbs << std::endl;
                    if (zAbs < 0.015) {
                        isVisible = true;
                        break;
                    }
                }

            }
            if(isVisible)
                break;
        }
        if (pointOutsideScreen) {
            // 渲染出的点在可视区域外
            p_outsidePointsNum++;
        }
        p_isVertexVisible.push_back(isVisible);
        visibleVertexCount += isVisible ? 1 : 0;
    }

    p_xmin = xmin;
    p_xmax = xmax;
    p_ymin = ymin;
    p_ymax = ymax;
    p_zmin = zmin;
    p_zmax = zmax;

    p_model_x = glm::vec4(xmax - xmin,0,0,0);
    p_model_y = glm::vec4(0,ymax - ymin,0,0);
    p_model_z = glm::vec4(0,0,zmax - zmin,0);
    p_model_x = modelViewMatrix * p_model_x;
    p_model_y = modelViewMatrix * p_model_y;
    p_model_z = modelViewMatrix * p_model_z;
    p_model_x[3] = 0;
    p_model_y[3] = 0;
    p_model_z[3] = 0;

    // 筛选出可见面
    // 所谓可见面，就是指该面上其中一个顶点可见
    p_VisibleFaces.clear();
    for (int i = 0; i < indices.size(); i += 3)
        if (p_isVertexVisible[indices[i]]
                && p_isVertexVisible[indices[i+1]]
                && p_isVertexVisible[indices[i+2]]) {
            p_VisibleFaces.push_back(indices[i]);
            p_VisibleFaces.push_back(indices[i+1]);
            p_VisibleFaces.push_back(indices[i+2]);
        }

    doneCurrent();
}

