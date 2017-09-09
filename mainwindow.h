#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QFileDialog>
#include "imgset.h"
#include "glwidget.h"
#include "OpenGLWidget/myglwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_Quit_clicked();

    void on_importImgs_clicked();

    void on_assess_clicked();

    void on_importModel_clicked();

private:
    Ui::MainWindow *ui;
    QGridLayout *mainWidgetLayout;
//    ImgLabel *imgLabel;
//    ImgLabel **imglabels;

    ImgSet *imgSet;
//    GLWidget *glWidget;
    MyGLWidget *myGLWidget;

    // setup UI
    void setImgLabels();
    void setGLWidget();


};

#endif // MAINWINDOW_H
