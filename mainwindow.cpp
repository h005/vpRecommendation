#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "svm2k/predictor.h"
#include <QTime>
#include <QDebug>
#include <QStringList>

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
    vpSet = NULL;

    glWidget = NULL;

    // set the short cut
    // std::cout << "svm2k fill data"
    ui->importImgs->setShortcut(Qt::Key_I);

    // set feaGeo to NULL pointer
    feaGeo = NULL;
    messageWidget = ui->plainTextEdit;
    messageWidget->document()->setPlainText("");

    // initial sfm
    sfm = new SfMContainer();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_Quit_clicked()
{
    on_Clear_clicked();
    delete sfm;
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

    if(imgFiles.size() == 0)
        return;

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
    int minColWidth = 360;
    int minRowHeigth = 240;
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
//    messageWidget->document()->setPlainText("load the images");
    messageWidget->appendPlainText("load the images");
    messageWidget->repaint();
//    update();
    repaint();
    std::cout << "load images done " << std::endl;
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
//    QString modelFile = QFileDialog::getOpenFileName(this,
//                                             QString("Load 3D model"),
//                                             QString("/home/eye/Documents/vpDataSet/validMeshModel"),
//                                             QString("Image Files(*.off *.obj)"),
//                                             nullptr,
//                                             QFileDialog::DontUseNativeDialog);

    QString modelFile = "/home/miracle/Documents/dataSet/facetune/femaleDataset5/faceData/test/bad/9/AF681_l.obj";
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
    viewpointQualityAssessment(1);
    statusBar()->showMessage("");
}

//void MainWindow::on_assessModel_clicked()
//{
//    feaGeo = new FeaGeo(this->glWidget);
//    feaGeo->extractFeaturesPipline();
//}

void MainWindow::imageQualityAssessment()
{
    messageWidget->appendPlainText("set features");
    messageWidget->repaint();
    imgSet->setFeatures();
    Predictor *predictor = new Predictor();
    // set img features ie 2D features ie XTest1
    predictor->setImgFeatures(imgSet);
    cv::Mat label;
    cv::Mat score;
    // these two sentences is equals to call predictor->predictLabelWithViewId(label, Predictor::ViewId_Img);
    predictor->predictScoreWithViewId(score, Predictor::ViewId_Img);
    predictor->score2Label(label,score);
//    predictor->predictLabelWithViewId(label, Predictor::ViewId_Img);
    imgSet->setFlagPosNeg(label);
//    imgSet->printLabel(label);
    imgSet->printScore(score);

    delete predictor;
}

void MainWindow::viewpointQualityAssessment(int knowAxis)
{
    QTime time;
    time.start();
    if(!glWidget)
    {
        std::cout << "please input the model first" << std::endl;
        return;
    }
    if(vpSet)
        delete vpSet;
    vpSet = new ViewPointSet(cameraLocations, groundPlane);
    vpSet->setFeatures(glWidget,knowAxis);
    Predictor *predictor = new Predictor();
    // set geo features ie 3D features ie XTest2
//    predictor->setGeoFeatures(vpSet);
    predictor->setImgGeoFeatures(vpSet);
    cv::Mat score;
    predictor->predictScoreWithViewId(score, Predictor::ViewId_ImgGeo);
//    vpSet->setRecommendationLocations(score);

    vpSet->setRecommendationLocationsWithRatio(score);
    score.release();
    delete predictor;

    std::cout << "vp recommendation elapsed time: " << (double)time.elapsed() / 1000.0 << " ms" << std::endl;
}

void MainWindow::setUpUiStyle()
{
    ui->importImgs->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->assess->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->Quit->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->importModel->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->recommend->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->SfM->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->Clear->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->sfm_imgFolder->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->sfm_outputFolder->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->sfm_pt2mesh->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->sfmClean->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->cleanLog->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->loadCameras->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
//    ui->recommendKnowAxis->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");

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
    {
//        there is a bug for memeroy leak
        delete glWidget;
        glWidget = NULL;
    }
//    std::cout << "clean vp set glwidget" << std::endl;
    if(feaGeo)
    {
        delete feaGeo;
        feaGeo = NULL;
    }
//    std::cout << "clean vp set feaGeo" << std::endl;
//    if(vpSet)
//    {
//        delete vpSet;
//        vpSet = NULL;
//    }
    //    std::cout << "clean vp set vpSet" << std::endl;
}

void MainWindow::getFileList(QString path, QFileInfoList &fileInfoList)
{
    QDir dir(path);
    // get folders
    dir.setFilter(QDir::Hidden | QDir::NoSymLinks | QDir::Dirs | QDir::NoDotAndDotDot);

    QFileInfoList folderList = dir.entryInfoList();
//    for(int i = 0; i < folderList.size(); ++i)
//    {
//        QFileInfo fileInfo = folderList.at(i);
//        qDebug() << fileInfo.fileName() << endl;
//    }
    // get files
    QStringList filters;
    filters << QString("*.jpg") << QString("*.png");
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setNameFilters(filters);
    QFileInfoList fileList = dir.entryInfoList();
    fileInfoList.append(fileList);
//    for(int i = 0; i < fileList.size(); ++i)
//    {
//        QFileInfo fileInfo = fileList.at(i);
//        qDebug() << fileList.at(i).absoluteFilePath() << endl;
////        qDebug() << fileList.at(i) << endl;
//    }
    getFileList(folderList, fileInfoList);
}

void MainWindow::getFileList(QFileInfoList folderList, QFileInfoList &fileInfoList)
{
    for(int i=0; i < folderList.size(); i++)
        getFileList(folderList.at(i).absoluteFilePath(), fileInfoList);
}

void MainWindow::on_Clear_clicked()
{
    cleanImgSet();
    cleanVpSet();
//    std::cout << "clear done" << std::endl;
//    messageWidget->clear();
    statusBar()->showMessage("clear done");
}

void MainWindow::on_sfm_imgFolder_clicked()
{
    QString imgFolder = QFileDialog::getExistingDirectory(this,
                                                          QString("select the image folder"),
                                                          QString());
    messageWidget->appendPlainText("sfm select the imgFolder:");
    messageWidget->appendPlainText(imgFolder);
    sfm->setImgFolder(imgFolder);
}

void MainWindow::on_sfm_outputFolder_clicked()
{
    QString outputFolder = QFileDialog::getExistingDirectory(this,
                                                             QString("select the output folder"),
                                                             QString());
    messageWidget->appendPlainText("sfm select the outputFolder:");
    messageWidget->appendPlainText(outputFolder);
    sfm->setOutputFolder(outputFolder);
}

void MainWindow::on_SfM_clicked()
{
    statusBar()->showMessage("sfm imgs 2 point cloud");
    sfm->sfm_imgs2ptModel();
}

void MainWindow::on_sfm_pt2mesh_clicked()
{
    statusBar()->showMessage("sfm pt 2 mesh");
    sfm->sfm_pt2MeshModel();
}

void MainWindow::on_sfmClean_clicked()
{
    sfm->cleanFiles();
}

void MainWindow::on_cleanLog_clicked()
{
    sfm->cleanLog();
}

//void MainWindow::on_recommendKnowAxis_clicked()
//{
//    statusBar()->showMessage("busy");
//    statusBar()->repaint();
//    viewpointQualityAssessment(0);
//    statusBar()->showMessage("");
//}

void MainWindow::on_loadCameras_clicked()
{
    QString cameraFile = QFileDialog::getOpenFileName(this,
                                             QString("Load camera locations"),
                                             QString("/home/eye/Documents/vpDataSet/validMeshModel"),
                                             QString("Image Files(*.lst)"),
                                             nullptr,
                                             QFileDialog::DontUseNativeDialog);

    std::fstream fs;
    std::vector< glm::vec3 > recommendedCamerasLocations;

    cameraLocations.clear();
    groundPlane.clear();

///
/// cameraLocations.list
/// #NUM
/// x1 y1 z1
/// x2 y2 z2
/// ...
/// x_NUM y_NUM z_NUM
///

    // read in the locations from the file
    fs.open(cameraFile.toStdString(), std::fstream::in);
    int NUM;
    fs >> NUM;
    float tmp;
    for(int i=0;i<NUM;i++)
    {
        glm::vec3 pos;
        fs >> tmp;
        pos.x = tmp;
        fs >> tmp;
        pos.y = tmp;
        fs >> tmp;
        pos.z = tmp;
        cameraLocations.push_back(pos);
    }
    fs.close();

///
/// plane.txt
/// #NUM
/// nx ny nz
/// px py pz
///

    QString plane = cameraFile;
    int ind = plane.lastIndexOf('/');
    plane.replace(ind + 1, 30, QString("planeZ0.txt"));
//    std::cout << plane.toStdString() << std::endl;
    groundPlane.clear();
    fs.open(plane.toStdString());
    fs >> NUM;
    for(int i=0;i<NUM;i++)
    {
        glm::vec3 pos;
        fs >> tmp;
        pos.x = tmp;
        fs >> tmp;
        pos.y = tmp;
        fs >> tmp;
        pos.z = tmp;
//        recommendedCamerasLocations.push_back(pos);
        groundPlane.push_back(pos);
    }
    fs.close();

    if(!glWidget)
    {
        std::cout << "please input the model first" << std::endl;
        return;
    }
    glWidget->setCamerasLocation(cameraLocations);
//    glWidget->setRecommendationCameraLocations(recommendedCamerasLocations);

//    std::cout << "camera postion loaded done" << std::endl;
}

void MainWindow::on_showGround_clicked()
{
    if(glWidget)
    {
        cv::Mat img;
        cv::Mat mask;
        glWidget->setImgMask(img, mask);
        cv::namedWindow("depthImg");
        cv::imshow("depthImg", mask);
        cv::waitKey(0);
        cv::namedWindow("rgbImg");
        cv::imshow("rgbImg", img);
        cv::waitKey(0);

//        QString groundModel = glWidget->getModelPath();
//        int ind = groundModel.lastIndexOf('/');
//        groundModel.replace(ind + 1, 30, QString("groundZ0.obj"));
//        std::cout << groundModel.toStdString() << std::endl;
//        glWidget->addModel(groundModel);
//        std::cout << "show ground done" << std::endl;
    }
    else
        std::cout << "please load a model first" << std::endl;
}

void MainWindow::on_saveImage_clicked()
{
    if(glWidget)
    {
        cv::Mat img;
        glWidget->setRetangleMaskImage(img);
        cv::imwrite("/home/miracle/Documents/result.png", img);
        std::cout << "save image to ~/Documents/result.png" << std::endl;
//        cv::Mat img;
//        cv::Mat mask;
//        glWidget->setImgMask(img, mask);
//        cv::namedWindow("depthImg");
//        cv::imshow("depthImg", mask);
//        cv::waitKey(0);
//        cv::namedWindow("rgbImg");
//        cv::imshow("rgbImg", img);
//        cv::waitKey(0);
    }
    else
        std::cout << "please load a model first" << std::endl;
}

void MainWindow::on_renderBatch_clicked()
{
    std::vector<std::string> model_list;

    QString base_path = "/home/miracle/Documents/dataSet/facetune/femaleDataset5/faceData/test";
    QFileInfoList fileInfoList;
    fileInfoList.clear();
    getFileList(base_path, fileInfoList);
    for(int i=0; i < fileInfoList.size(); i++)
    {
        QFileInfo fileinfo = fileInfoList.at(i);
        qDebug() << i << " " << fileinfo.absoluteFilePath() << endl;
    }

}
