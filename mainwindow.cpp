#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // set the background color
    setStyleSheet("background-color:rgb(225,255,255);");

    // mainWidgetLayout Setup
    mainWidgetLayout = new QGridLayout();
    mainWidgetLayout->setSpacing(10);
    mainWidgetLayout->setMargin(10);

    // setup the imgSet
    imgSet = new ImgSet();

    // set the short cuts
    ui->importImgs->setShortcut(Qt::Key_I);
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
    this->glWidget = new GLWidget();
    mainWidgetLayout->addWidget(glWidget);
}

void MainWindow::on_importImgs_clicked()
{

    QStringList imgFiles = QFileDialog::getOpenFileNames(this,
                                             QString("Load images"),
                                             QString("/home/hejw005/Documents/learning/QtProject/vpRecommendation/data/imgs/"),
                                             QString("Image Files(*.jpg *.JPG *.png)"));
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
    int minColWidth = 480;
    int minRowHeigth = 320;
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
    qDebug() << "set Background done" << endl;
}


void MainWindow::on_assess_clicked()
{
//    qDebug() << "imgLabel size " << imgSet->imgLabelSize() << endl;
    imgSet->setFeatures();
}

void MainWindow::on_importModel_clicked()
{
    QString modelFile = QFileDialog::getOpenFileName(this,
                                             QString("Load 3D model"),
                                             QString("/home/hejw005/Documents/learning/QtProject/vpRecommendation/data/models/"),
                                             QString("Image Files(*.ply *.off *.obj)"));

    qDebug() << "modelFile " << modelFile << endl;

    if(modelFile == "")
    {
        statusBar()->showMessage("no selected model");
        return;
    }


    setGLWidget();
}
