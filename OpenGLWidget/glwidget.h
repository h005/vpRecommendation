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

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <GL/glew.h>
#include "dragablewidget.h"
#include "RenderObject/GModel.h"
#include "RenderObject/sphere.h"
#include <opencv.hpp>

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class PointsMatchRelation;
class QString;
class GLWidget : public DragableWidget
{
    Q_OBJECT

public:
    GLWidget(const QString &modelPath, QWidget *parent = 0);
    ~GLWidget();

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    int addPoint(const QPoint &p);
    bool removeLastPoint();

    PointsMatchRelation *m_relation;

    // these functions were created to set the parameters for geo features
    // img is the render result for the given MVP matrix
    // mask is the mask of the render result
    void setImgMask(cv::Mat &img, cv::Mat &mask);

    void setVpRecommendationMatrix(glm::mat4 &vpRecommendationMatrix);

    void setRecommendationLocations(std::vector<glm::vec2> &vpRcameraLocations,
                                    float sceneZ,
                                    std::vector<int> &index);
    void clearVpRcameraLocations();

//    glm::mat4 getMMatrix();
    glm::mat4 getOriginalMatrix();
    glm::mat4 getMVMatrix();
    // projection matrix
    glm::mat4 getProjMatrix();

    // render the model
    void render();

    void setParameters(std::vector<GLfloat> &vertice,
                                              std::vector<GLuint> &indices,
                                              glm::vec4 &p_model_x,
                                              glm::vec4 &p_model_y,
                                              glm::vec4 &p_model_z,
                                              int &outsidePointsNum);

private:
    void cleanup();

signals:

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int width, int height) Q_DECL_OVERRIDE;
    glm::mat4 getModelViewMatrix();
    virtual glm::mat4 getModelMatrix() override;

    void keyPressEvent(QKeyEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

protected:
    glm::mat4 m_camera;
    glm::mat4 m_proj;
    glm::mat4 vpRecommendationMatrix;
    GLfloat m_scaleBeforeRender;
    glm::mat4 m_shiftBeforeRender;
    GModel model;
    GModel cameraModel;
    GLuint m_sphereProgramID = 0;
    Sphere sphere;
    glm::vec3 cameraPos;
    std::vector< glm::vec3 > vpRcameraLocations;
    QPointF m_lastPos;
    bool flag_move;
    float camPosLength;
    float rate;
private:

    bool m_transparent;
};

#endif
