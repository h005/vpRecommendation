#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "svm2k/predictor.h"

#include <QDebug>

QPlainTextEdit *messageWidget;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setStyleSheet("background-color:rgb(75,75,75);");

    setUpUiStyle();
    // mainWidgetLayout Setup
    mainWidgetLayout = new QGridLayout();
    mainWidgetLayout->setSpacing(10);
    mainWidgetLayout->setMargin(10);

    // setup the imgSet
    imgSet = new ImgSet();
    // setup the vpSet
    vpSet = new ViewPointSet();

    glWidget = NULL;

    // set the short cut
    // std::cout << "svm2k fill data"
    ui->importImgs->setShortcut(Qt::Key_I);

    // set feaGeo to NULL pointer
    feaGeo = NULL;
    messageWidget = ui->plainTextEdit;

    // initial sfm
    sfm = new SfMContainer();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_Quit_clicked()
{
    this->close();
}



void MainWindow::setGLWidget()
{
//    mainWidgetLayout->addWidget(myGLWidget);
    mainWidgetLayout->addWidget(glWidget);
    ui->mainWidget->setLayout(mainWidgetLayout);
    messageWidget->appendPlainText("set OpenGL widget done");
//    qDebug() << "set glWidget done" << endl;
}

void MainWindow::on_importImgs_clicked()
{
    QStringList imgFiles = QFileDialog::getOpenFileNames(this,
                                             QString("Load images"),
                                             QString("/home/hejw005/Documents/learning/QtProject/vpRecommendation/data/imgs/"),
                                             QString("Image Files(*.jpg *.jpeg *.JPG *.JPEG *.png)"),
                                             nullptr,
                                             QFileDialog::DontUseNativeDialog);

    imgSet->setImgFiles(imgFiles);
    imgSet->initialImgLabels();

    // set the status message
    QString num;
    num = num.setNum(imgFiles.size());

    if(imgFiles.size() >= 1)
        statusBar()->showMessage(num + " images were loaded.");
    else
        statusBar()->showMessage(num + " image was loaded.");

    setImgLabels();
}

void MainWindow::setImgLabels()
{
    int minColWidth = 300;
    int minRowHeigth = 225;
    int numImgs = imgSet->size();
    int rows = numImgs / 2;
    // fill the images in the girds
    for(int i = 0; i < rows; i++)
    {
        for(int j = 0; j < 2; j++)
            mainWidgetLayout->addWidget(imgSet->getImgLabel(i * 2 + j), i, j);
        // set the row minimum width
        mainWidgetLayout->setRowMinimumHeight(i, minRowHeigth);
    }

    // set the column minimum width
    mainWidgetLayout->setColumnMinimumWidth(0, minColWidth);
    mainWidgetLayout->setColumnMinimumWidth(1, minColWidth);

    // fill in the last image
    if(numImgs % 2)
    {
        mainWidgetLayout->addWidget(imgSet->getImgLabel(numImgs - 1), rows + 1, 0);
        mainWidgetLayout->setRowMinimumHeight(rows + 1, minRowHeigth);
    }

    ui->mainWidget->setLayout(mainWidgetLayout);
//    qDebug() << "set Background done" << endl;
    messageWidget->document()->setPlainText("load the images");
}


void MainWindow::on_assess_clicked()
{
    statusBar()->showMessage("busy");
    statusBar()->repaint();
    imageQualityAssessment();
    statusBar()->showMessage("");
////    qDebug() << "imgLabel size " << imgSet->imgLabelSize() << endl;
//    imgSet->setFeatures();
//    Predictor *predictor = new Predictor();
//    // set img features ie 2D features ie XTest1
//    predictor->setImgFeatures(imgSet);
//    cv::Mat label;
//    predictor->predictLabelWithViewId(label, Predictor::ViewId_Img);
//    imgSet->printLabel(label);
}

void MainWindow::on_importModel_clicked()
{
    QString modelFile = QFileDialog::getOpenFileName(this,
                                             QString("Load 3D model"),
                                             QString("/home/hejw005/Documents/learning/QtProject/vpRecommendation/data/models/"),
                                             QString("Image Files(*.off *.obj)"),
                                             nullptr,
                                             QFileDialog::DontUseNativeDialog);

//    qDebug() << "modelFile " << modelFile << endl;
    messageWidget->appendPlainText(modelFile + " loading");

    if(modelFile == "")
    {
        statusBar()->showMessage("no selected model");
        return;
    }
    statusBar()->showMessage(modelFile + " loading");

    glWidget = new GLWidget(modelFile);
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
    glWidget->setFormat(format);
    setGLWidget();
    messageWidget->appendPlainText(modelFile + " is loaded");
    statusBar()->showMessage(modelFile + " is loaded");
}

void MainWindow::on_recommend_clicked()
{
//    feaGeo = new FeaGeo(this->glWidget);
//    feaGeo->extractFeaturesPipline();
//    if(!feaGeo)
//        feaGeo = new FeaGeo(this->glWidget);
//    feaGeo->vpRecommendPipLine();
    statusBar()->showMessage("busy");
    statusBar()->repaint();
    viewpointQualityAssessment();
    statusBar()->showMessage("");
}

void MainWindow::on_assessModel_clicked()
{
    feaGeo = new FeaGeo(this->glWidget);
    feaGeo->extractFeaturesPipline();
}

void MainWindow::imageQualityAssessment()
{
    messageWidget->appendPlainText("set features");
    messageWidget->repaint();
    imgSet->setFeatures();
    Predictor *predictor = new Predictor();
    // set img features ie 2D features ie XTest1
    predictor->setImgFeatures(imgSet);
    cv::Mat label;
    predictor->predictLabelWithViewId(label, Predictor::ViewId_Img);
    imgSet->printLabel(label);
    delete predictor;
}

void MainWindow::viewpointQualityAssessment()
{
    vpSet->setFeatures(glWidget);
    Predictor *predictor = new Predictor();
    // set geo features ie 3D features ie XTest2
    predictor->setGeoFeatures(vpSet);
    cv::Mat score;
    predictor->predictScoreWithViewId(score, Predictor::ViewId_Geo);
//    vpSet->setRecommendationLocations(score);
    vpSet->setRecommendationLocationsWithRatio(score);
    score.release();
    delete predictor;

}

void MainWindow::setUpUiStyle()
{
    ui->importImgs->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->assess->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->Quit->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->importModel->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->recommend->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->assessModel->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->SfM->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->Clear->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->plainTextEdit->setStyleSheet("QPlainTextEdit{color:white;background:rgb(35,35,35)}");

    ui->geoLabel->setStyleSheet("QLabel{color:white;background:rgb(75,75,75)}");
    ui->imgLabel->setStyleSheet("QLabel{color:white;background:rgb(75,75,75)}");
    ui->sfmLabel->setStyleSheet("QLabel{color:white;background:rgb(75,75,75)}");
}

void MainWindow::cleanImgSet()
{
    // clear the mainWidgetLayout
    int numImgs = imgSet->size();
    // fill the images in the grids
    for(int i = 0; i < numImgs; i++)
        mainWidgetLayout->removeWidget(imgSet->getImgLabel(i));
    delete mainWidgetLayout;
    mainWidgetLayout = new QGridLayout();
    mainWidgetLayout->setSpacing(5);
    mainWidgetLayout->setMargin(5);
    imgSet->clean();
    statusBar()->showMessage("");
}

void MainWindow::cleanVpSet()
{
    if(glWidget)
        delete glWidget;
    if(feaGeo)
        delete feaGeo;

}

void MainWindow::on_Clear_clicked()
{
    cleanImgSet();
    cleanVpSet();
//    std::cout << "clear done" << std::endl;
    messageWidget->clear();
    statusBar()->showMessage("clear done");
}

void MainWindow::on_sfm_imgFolder_clicked()
{
    QString imgFolder = QFileDialog::getExistingDirectory(this,
                                                          QString("select the image folder"),
                                                          QString());
//    qDebug() << imgFolder << endl;
//    sfm->setImgFolder();
}

void MainWindow::on_sfm_outputFolder_clicked()
{
    QString outputFolder = QFileDialog::getExistingDirectory(this,
                                                             QString("select the output folder"),
                                                             QString());
//    sfm->setOutputFolder();
}

void MainWindow::on_SfM_clicked()
{
    sfm->sfm_imgs2ptModel();
}

void MainWindow::on_sfm_pt2mesh_clicked()
{
    sfm->sfm_pt2MeshModel();
}
