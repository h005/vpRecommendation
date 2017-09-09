#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include "OpenGLWidget/dragablewidget.h"
#include "RenderObject/mesh.h"
#include "RenderObject/gmodel.h"
#include <glm/glm.hpp>

//#include "GModel/

class MyGLWidget : public DragableWidget
{
    Q_OBJECT

public:
    explicit MyGLWidget(const QString &modelPath, QWidget *parent = 0);
    ~MyGLWidget();

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w,int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
//    void resizeEvent(QResizeEvent *e);

    void initShaders();

    glm::mat4 getModelViewMatrix();
    virtual glm::mat4 getModelMatrix() override;

    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

private:
    QString modelPath;

    // geometry
    Mesh *m_mesh;

    // lights
    int m_lightPosLoc;

    // shaders
    QOpenGLShaderProgram program;

    //matrices
    QMatrix4x4 projection; // projection matrix
    QMatrix4x4 matrix; //model view matrix
    QMatrix3x3 normal; // normal matrix

    // model-view and projection matrices
    glm::mat4 m_camera;
    glm::mat4 m_proj;
    GLfloat m_scaleBeforeRender;
    glm::mat4 m_shiftBeforeRender;

    glm::vec3 cameraPos;
    // Render Object
//    GModel model;

    bool flag_move;
    float camPosLength;
    float rate;


};

#endif // MYGLWIDGET_H
