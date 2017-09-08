#include "myglwidget.h"

MyGLWidget::MyGLWidget(const QString &modelPath, QWidget *parent)
    : DragableWidget(parent)
{
    this->modelPath = modelPath;
}

//void MyGLWidget::MyGLWidget()
//{

//}

void MyGLWidget::initializeGL()
{
//    QOpenGLFunctions_3_3_Core *f = QOpenGLContext::currentContext()->functions();
//    f->glClearColor(1.0f,1.0f,1.0f,1.0f);
}

void MyGLWidget::resizeGL(int w, int h)
{

}

void MyGLWidget::paintGL()
{
//    QOpenGLFunctions_3_3_Core *f = QOpenGLContext::currentContext()->functions();
//    QOpenGLContext::currentContext()->functions();
//    f->glClear(GL_COLOR_BUFFER_BIT);
}
