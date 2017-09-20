#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QFileDialog>
#include "imgset.h"
#include "OpenGLWidget/glwidget.h"
#include "OpenGLWidget/myglwidget.h"
#include "features/feageo.h"
#include "viewpointset.h"

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

    void on_recommend_clicked();

    void on_assessModel_clicked();

    void on_Clear_clicked();

private:
    void imageQualityAssessment();

    void viewpointQualityAssessment();

    void setUpUiStyle();

    void cleanImgSet();

    void cleanVpSet();

private:
    Ui::MainWindow *ui;
    QGridLayout *mainWidgetLayout;
//    ImgLabel *imgLabel;
//    ImgLabel **imglabels;

    ImgSet *imgSet;
    ViewPointSet *vpSet;
    FeaGeo *feaGeo;
    GLWidget *glWidget;

    // setup UI
    void setImgLabels();
    void setGLWidget();


};

#endif // MAINWINDOW_H
