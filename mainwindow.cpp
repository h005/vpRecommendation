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
                                             QString("/home/h005/Documents/sumSang/BrandenburgGate/imgs/"),
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
    int numImgs = imgSet->size();
    int rows = numImgs / 2;
    // fill the images in the girds
    for(int i = 0; i < rows; i++)
    {
        for(int j = 0; j < 2; j++)
            mainWidgetLayout->addWidget(imgSet->getImgLabel(i * 2 + j), i, j);
        // set the row minimum width
        mainWidgetLayout->setRowMinimumHeight(i, 320);
    }

    // set the column minimum width
    mainWidgetLayout->setColumnMinimumWidth(0, 480);
    mainWidgetLayout->setColumnMinimumWidth(1, 480);

    // fill in the last image
    if(numImgs % 2)
    {
        mainWidgetLayout->addWidget(imgSet->getImgLabel(numImgs - 1), rows + 1, 0);
        mainWidgetLayout->setRowMinimumHeight(rows + 1, 320);
    }

    ui->mainWidget->setLayout(mainWidgetLayout);
    qDebug() << "set Background done" << endl;
}


void MainWindow::on_assess_clicked()
{
//    qDebug() << "imgLabel size " << imgSet->imgLabelSize() << endl;
    imgSet->setFeatures();
}
