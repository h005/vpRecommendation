#ifndef DRAGABLEWIDGET_H
#define DRAGABLEWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <glm/glm.hpp>

class DragableWidget: public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    DragableWidget(QWidget *parent);
    ~DragableWidget();

protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent (QWheelEvent * event) Q_DECL_OVERRIDE;

    virtual glm::mat4 getModelMatrix();
    glm::mat4 m_baseRotate;
    glm::vec3 m_rotateN;
    float m_angle;
    GLfloat m_scale;
    QPoint m_lastPos;

};

#endif // DRAGABLEWIDGET_H
