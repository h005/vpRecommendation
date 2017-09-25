#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QFileDialog>
#include "imgset.h"
#include "OpenGLWidget/glwidget.h"
#include "features/feageo.h"
#include "SfM/sfmcontainer.h"
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

//    void on_assessModel_clicked();

    void on_Clear_clicked();

    void on_sfm_imgFolder_clicked();

    void on_sfm_outputFolder_clicked();

    void on_SfM_clicked();

    void on_sfm_pt2mesh_clicked();

    void on_sfmClean_clicked();

    void on_cleanLog_clicked();

    void on_recommendKnowAxis_clicked();

private:
    void imageQualityAssessment();

    // knowAxis 1 means we know the axis
    // knwoAxis 0 means we do not know the axis
    void viewpointQualityAssessment(int knowAxis);

    void setUpUiStyle();

    void cleanImgSet();

    void cleanVpSet();

private:
    Ui::MainWindow *ui;
    QGridLayout *mainWidgetLayout;

    ImgSet *imgSet;
    ViewPointSet *vpSet;
    FeaGeo *feaGeo;
    GLWidget *glWidget;
    SfMContainer *sfm;

    // setup UI
    void setImgLabels();
    void setGLWidget();


};

#endif // MAINWINDOW_H
