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
    vpSet = NULL;

    glWidget = NULL;

    // set the short cut
    // std::cout << "svm2k fill data"
//    ui->importImgs->setShortcut(Qt::Key_I);

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
    if(!glWidget)
    {
        std::cout << "please input the model first" << std::endl;
        return;
    }
    if(vpSet)
        delete vpSet;
    vpSet = new ViewPointSet();
    vpSet->setFeatures(glWidget,knowAxis);
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
//    ui->importImgs->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
//    ui->assess->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->Quit->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
//    ui->importModel->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
//    ui->recommend->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->SfM->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->Clear->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->sfm_imgFolder->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->sfm_outputFolder->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->sfm_pt2mesh->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->sfmClean->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->cleanLog->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
//    ui->recommendKnowAxis->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->showSfM->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");
    ui->plainTextEdit->setStyleSheet("QPlainTextEdit{color:white;background:rgb(35,35,35)}");
    ui->exportModel->setStyleSheet("QPushButton{color:white;background:rgb(35,35,35)}");

//    ui->geoLabel->setStyleSheet("QLabel{color:white;background:rgb(75,75,75)}");
//    ui->imgLabel->setStyleSheet("QLabel{color:white;background:rgb(75,75,75)}");
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
        delete glWidget;
        glWidget = NULL;
    }
    if(feaGeo)
    {
        delete feaGeo;
        feaGeo = NULL;
    }
    if(vpSet)
    {
        delete vpSet;
        vpSet = NULL;
    }

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

    QDir sfmInputDir(imgFolder);
    QStringList filters;
    filters << "*.jpg" << "*.jpeg" << "*.JPG" << "*.JPEG" << "*.png";
    sfmInputDir.setNameFilters(filters);
    QStringList imgLists = sfmInputDir.entryList(filters);
    QStringList imgFiles;
    if(imgLists.size() == 0)
        return;

    for(int i=0;i<imgLists.size();i++)
//    {
        imgFiles.append(imgFolder + "/" + imgLists.at(i));
//        qDebug() << i << " " << imgLists.at(i) << endl;
//    }

    imgSet->setImgFiles(imgFiles);
    imgSet->initialImgLabels();

    // set the status message
    QString num;
    num = num.setNum(imgFiles.size());

    if(imgFiles.size() > 1)
        statusBar()->showMessage(num + " images were loaded.");
    else
        statusBar()->showMessage(num + " image was loaded.");

    setImgLabels();

    messageWidget->appendPlainText("sfm select the imgFolder:");
    messageWidget->appendPlainText(imgFolder);
    sfm->setImgFolder(imgFolder);
    QString outputFolder = imgFolder + "/result";
    QDir reconstructionFolder(imgFolder);
    if(!reconstructionFolder.exists(QString("result")))
    {
        qDebug() << "reconstruction does not exists" << endl;
        reconstructionFolder.mkdir(QString("result"));
    }
    else
        qDebug() << "reconstruction exists" << endl;

    sfm->setOutputFolder(outputFolder);
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

void MainWindow::on_showSfM_clicked()
{
    on_Clear_clicked();
    QString modelFile = sfm->getOutputFolder();
    modelFile = modelFile + "/reconstruction/scene_dense_mesh_texture.obj";
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


void MainWindow::on_exportModel_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                   ".obj",
                                   tr("obj Files (*.obj)"));

    QFileInfo fileInfo(fileName);


    QString modelFile = sfm->getOutputFolder();

    QDir sfmModelFolder(modelFile + "/reconstruction");
    if(!sfmModelFolder.exists("scene_dense_mesh_texture.obj"))
    {
        messageWidget->appendPlainText("The model file does not exists");
//        std::cout << "the model file does not exists" << std::endl;
        return;
    }
    QString tmpModelFile = modelFile + "/reconstruction/scene_dense_mesh_texture.obj";
    QString tmpTextureFile = modelFile + "/reconstruction/scene_dense_mesh_texture_material_0_map_Kd.jpg";
    QString tmpMtlFile = modelFile + "/reconstruction/scene_dense_mesh_texture.mtl";


    // copy model file
    QFile::copy(tmpModelFile,fileName);
    // copy texture file
    QFileInfo auxFileInfo(tmpTextureFile);
    QFile::copy(tmpTextureFile, fileInfo.absolutePath() + "/" + auxFileInfo.fileName());
    // copy mtl file
    auxFileInfo = QFileInfo(tmpMtlFile);
    QFile::copy(tmpMtlFile, fileInfo.absolutePath() + "/" + auxFileInfo.fileName());


    messageWidget->appendPlainText("export the obj model to " + fileName);

    statusBar()->showMessage("export model done");
}
