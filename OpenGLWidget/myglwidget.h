#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include "OpenGLWidget/dragablewidget.h"
//#include "GModel/

class MyGLWidget : public DragableWidget
{
    Q_OBJECT

public:
//    MyGLWidget();
    MyGLWidget(const QString &modelPath, QWidget *parent = 0);
    ~MyGLWidget();

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w,int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
//    void resizeEvent(QResizeEvent *e);

private:
    QString modelPath;
};

#endif // MYGLWIDGET_H
