#include "sfmcontainer.h"

#include <QPlainTextEdit>
#include <iostream>
#include <QDir>

extern QPlainTextEdit *messageWidget;

SfMContainer::SfMContainer()
{
//    imgFolder = "/home/h005/Documents/samSung/sfmTest/imgs";
//    outputFolder = "/home/h005/Documents/samSung/sfmTest/result";
    imgFolder = "";
    outputFolder = "";
    basePath = "sfmTools";

    cameraDatabase = basePath + "/sensor_width_camera_database.txt";

    openMVG_main_SfMInit_ImageListing = basePath + "/openMVG/openMVG_main_SfMInit_ImageListing";
    openMVG_main_ComputeFeatures = basePath + "/openMVG/openMVG_main_ComputeFeatures";
    openMVG_main_ComputeMatches = basePath + "/openMVG/openMVG_main_ComputeMatches";
    openMVG_main_IncrementalSfM = basePath + "/openMVG/openMVG_main_IncrementalSfM";

    openMVG_main_openMVG2openMVS = basePath + "/openMVG/openMVG_main_openMVG2openMVS";

    DensifyPointCloud = basePath + "/openMVS/DensifyPointCloud";
    ReconstructMesh = basePath + "/openMVS/ReconstructMesh";
    RefineMesh = basePath + "/openMVS/RefineMesh";
    TextureMesh = basePath + "/openMVS/TextureMesh";

    myProcess = new QProcess();

    connect(myProcess,
            SIGNAL(finished(int, QProcess::ExitStatus)),
            this,
            SLOT(reconstruction(int, QProcess::ExitStatus)));

    openMVG_pipLineStep = openMVG_main_SfMInit_ImageListing_Pipline;
    openMVS_pipLineStep = openMVG_main_openMVG2openMVS_Pipline;

    MVG_MVS = 0;
}

SfMContainer::~SfMContainer()
{
    delete myProcess;
}

void SfMContainer::setImgFolder(QString imgFolder)
{
    this->imgFolder = imgFolder;
}

void SfMContainer::setOutputFolder(QString outputFolder)
{
    this->outputFolder = outputFolder;
}

void SfMContainer::sfm_imgs2ptModel()
{
    openMVG_pipLineStep = openMVG_main_SfMInit_ImageListing_Pipline;
    int exitCode = 0;
    QProcess::ExitStatus exitStatus = QProcess::NormalExit;
    if(!checkFolder())
    {
        messageWidget->appendPlainText("please set the input and output folder first");
        return;
    }
    MVG_MVS = 1;
    reconstruction(exitCode, exitStatus);
}

void SfMContainer::sfm_pt2MeshModel()
{
    int exitCode = 0;
    QProcess::ExitStatus exitStatus = QProcess::NormalExit;
    if(!checkFolder())
    {
        messageWidget->appendPlainText("please set the input and output folder first");
        return;
    }
    if(openMVG_pipLineStep != openMVG_main_End && openMVG_pipLineStep != openMVG_main_SfMInit_ImageListing_Pipline)
    {
        messageWidget->appendPlainText("structure from motion is still running...");
        return;
    }
    if(openMVG_pipLineStep == openMVG_main_SfMInit_ImageListing_Pipline)
    {
        messageWidget->appendPlainText("please run sfm first");
        return;
    }
    MVG_MVS = 2;
    openMVS_pipLineStep = openMVG_main_openMVG2openMVS_Pipline;
    reconstruction(exitCode, exitStatus);
    //    pt2MeshModel(exitCode, exitStatus);
}

void SfMContainer::cleanFiles()
{
    QDir output(outputFolder + "/model");
    output.removeRecursively();
    output = QDir(outputFolder + "/reconstruction");
    output.removeRecursively();
    cleanLog();
}

void SfMContainer::cleanLog()
{
    // remove the log and dmap files
    QDir thisProgram;
    QStringList filters;
    filters << "*.log"
            << "*.dmap";
    thisProgram.setFilter(QDir::Files | QDir::NoSymLinks);
    thisProgram.setNameFilters(filters);
    foreach(QFileInfo mfi, thisProgram.entryInfoList())
    {
        QFile::remove(mfi.fileName());
        std::cout << "remove " << mfi.fileName().toStdString() << std::endl;
    }
}

bool SfMContainer::checkFolder()
{
    if(imgFolder == "")
    {
        messageWidget->appendPlainText("sfm: please select the image folder first");
        return false;
    }
    if(outputFolder == "")
    {
        messageWidget->appendPlainText("sfm: please select the output folder first");
        return false;
    }
    return true;
}

void SfMContainer::reconstruction(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(MVG_MVS == 1)
        imgs2ptModel(exitCode, exitStatus);
    else if(MVG_MVS == 2)
        pt2MeshModel(exitCode, exitStatus);
}

///
/// \brief SfMContainer::imgs2ptModel
///
/// pipline:
/// 1. openMVG_main_SfMInit_ImageListing
/// 2. openMVG_main_ComputeFeatures
/// 3. openMVG_main_ComputeMatches
/// 4. openMVG_main_IncrementalSfM
///
void SfMContainer::imgs2ptModel(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(!exitCode && !exitStatus)
    {
        switch (openMVG_pipLineStep) {
        case openMVG_main_SfMInit_ImageListing_Pipline:
            messageWidget->appendPlainText("perform_openMVG_main_SfMInit_ImageListing ...");
            perform_openMVG_main_SfMInit_ImageListing(exitCode, exitStatus);
            openMVG_pipLineStep++;
            break;
        case openMVG_main_ComputeFeatures_Pipline:
            messageWidget->appendPlainText("done");
            messageWidget->appendPlainText("perform_openMVG_main_ComputeFeatures ...");
            perform_openMVG_main_ComputeFeatures(exitCode, exitStatus);
            openMVG_pipLineStep++;
            break;
        case openMVG_main_ComputeMatches_Pipline:
            messageWidget->appendPlainText("done");
            messageWidget->appendPlainText("perform_openMVG_main_ComputeMatches ...");
            perform_openMVG_main_ComputeMatches(exitCode, exitStatus);
            openMVG_pipLineStep++;
            break;
        case openMVG_main_IncrementalSfM_Pipline:
            messageWidget->appendPlainText("done");
            messageWidget->appendPlainText("perform_openMVG_main_IncrementalSfM ...");
            perform_openMVG_main_IncrementalSfM(exitCode, exitStatus);
            openMVG_pipLineStep++;
            break;
        default:
            openMVG_pipLineStep++;
            messageWidget->appendPlainText("done");
            messageWidget->appendPlainText("#################################\nstructure from motion done");
            break;
        }
    }
    else
    {
        switch (openMVG_pipLineStep) {
        case openMVG_main_ComputeFeatures_Pipline:
            messageWidget->appendPlainText("perform_openMVG_main_SfMInit_ImageListing error");
            break;
        case openMVG_main_ComputeMatches_Pipline:
            messageWidget->appendPlainText("perform_openMVG_main_ComputeFeatures error");
            break;
        case openMVG_main_IncrementalSfM_Pipline:
            messageWidget->appendPlainText("perform_openMVG_main_ComputeMatches error");
            break;
        default:
            messageWidget->appendPlainText("perform_openMVG_main_IncrementalSfM error");
            break;
        }
    }
}

void SfMContainer::perform_openMVG_main_SfMInit_ImageListing(int exitCode,
                                                             QProcess::ExitStatus exitStatus)
{

    QStringList arguments;
    arguments.clear();
    arguments << "-i"
              << imgFolder
              << "-o"
              << outputFolder + "/model"
              << "-d"
              << cameraDatabase
              << "-g"
              << "0"
              << "-c"
              << "1";

    myProcess->start(openMVG_main_SfMInit_ImageListing, arguments);
}

void SfMContainer::perform_openMVG_main_ComputeFeatures(int exitCode,
                                                        QProcess::ExitStatus exitStatus)
{

    QStringList arguments;
    arguments.clear();
    arguments << "-i"
              << outputFolder + "/model/sfm_data.json"
              << "-o"
              << outputFolder + "/model";

    myProcess->start(openMVG_main_ComputeFeatures, arguments);
}


void SfMContainer::perform_openMVG_main_ComputeMatches(int exitCode,
                                                       QProcess::ExitStatus exitStatus)
{
    QStringList arguments;
    arguments.clear();
    arguments << "-n"
              << "ANNL2"
              << "-g"
              << "f"
              << "-i"
              << outputFolder + "/model/sfm_data.json"
              << "-o"
              << outputFolder + "/model";

    myProcess->start(openMVG_main_ComputeMatches, arguments);
}

void SfMContainer::perform_openMVG_main_IncrementalSfM(int exitCode,
                                                       QProcess::ExitStatus exitStatus)
{
    QStringList arguments;
    arguments.clear();
    arguments << "-c"
              << "1"
              << "-f"
              << "ADJUST_ALL"
              << "-i"
              << outputFolder + "/model/sfm_data.json"
              << "-m"
              << outputFolder + "/model"
              << "-o"
              << outputFolder + "/reconstruction";

    myProcess->start(openMVG_main_IncrementalSfM, arguments);
}

///
/// \brief SfMContainer::pt2MeshModel
///
/// 1. openMVG_main_openMVG2openMVS
/// 2. openMVS_DensifyPointCloud
/// 3. openMVS_ReconstructMesh
/// 4. openMVS_RefineMesh
/// 5. openMVS_TextureMesh
///
void SfMContainer::pt2MeshModel(int exitCode, QProcess::ExitStatus exitStatus)
{
    QString message;
    QByteArray msg;
//    QStringList arguments;
//    perform_openMVG_main_openMVG2openMVS(arguments);
//    perform_openMVS_DensifyPointCloud(arguments);
//    perform_openMVS_ReconstructMesh(arguments);
//    perform_openMVS_RefineMesh(arguments);
//    perform_openMVS_TextureMesh(arguments);
    if(!exitCode && !exitStatus)
    {
        switch (openMVS_pipLineStep) {
        case openMVG_main_openMVG2openMVS_Pipline:
            messageWidget->appendPlainText("perform_openMVG_main_openMVG2openMVS ...");
            perform_openMVG_main_openMVG2openMVS(exitCode, exitStatus);
            openMVS_pipLineStep++;
            break;
        case openMVS_DensifyPointCloud_Pipline:
            messageWidget->appendPlainText("done");
            messageWidget->appendPlainText("perform_openMVS_DensifyPointCloud ...");
            perform_openMVS_DensifyPointCloud(exitCode, exitStatus);
            openMVS_pipLineStep++;
            break;
        case openMVS_ReconstructMesh_Pipline:
            message = myProcess->readAll();
            std::cout << message.toStdString() << std::endl;
            messageWidget->appendPlainText("done");
            messageWidget->appendPlainText("perform_openMVS_ReconstructMesh ...");
            perform_openMVS_ReconstructMesh(exitCode, exitStatus);
            openMVS_pipLineStep++;
            // refineMesh is time consuming, pass
            openMVS_pipLineStep++;
            break;
        case openMVS_RefineMesh_Pipline:
            messageWidget->appendPlainText("done");
            messageWidget->appendPlainText("perform_openMVS_RefineMesh ...");
            perform_openMVS_RefineMesh(exitCode, exitStatus);
            openMVS_pipLineStep++;
            break;
        case openMVS_TextureMesh_Pipline:
            messageWidget->appendPlainText("done");
            messageWidget->appendPlainText("perform_openMVS_TextureMesh ...");
            perform_openMVS_TextureMesh(exitCode, exitStatus);
            openMVS_pipLineStep++;
            break;
        default:
            messageWidget->appendPlainText("done");
            messageWidget->appendPlainText("#################################\npt 2 mesh done");
            openMVS_pipLineStep++;
            break;
        }
    }
    else
    {
        switch (openMVS_pipLineStep) {
        case openMVS_DensifyPointCloud_Pipline:
            message = myProcess->readAll();
            std::cout << message.toStdString() << std::endl;
            messageWidget->appendPlainText("perform_openMVG_main_openMVG2openMVS error");
            break;
        case openMVS_ReconstructMesh_Pipline:
//            message = myProcess->readAll();
//            msg = myProcess->readAllStandardOutput();
//            std::cout << message.toStdString() << std::endl;
//            std::cout <<"msg " << msg.toStdString() << std::endl;
            messageWidget->appendPlainText("perform_openMVS_DensifyPointCloud error");
            break;
        case openMVS_RefineMesh_Pipline:
            message = myProcess->readAll();
            std::cout << message.toStdString() << std::endl;
            messageWidget->appendPlainText("perform_openMVS_ReconstructMesh error");
            break;
        case openMVS_TextureMesh_Pipline:
            message = myProcess->readAll();
            std::cout << message.toStdString() << std::endl;
            messageWidget->appendPlainText("perform_openMVS_RefineMesh error");
            break;
        default:
            message = myProcess->readAll();
            std::cout << message.toStdString() << std::endl;
            messageWidget->appendPlainText("perform_openMVS_TextureMesh error");
            break;
        }
    }

}

void SfMContainer::perform_openMVG_main_openMVG2openMVS(int exitCode,
                                                        QProcess::ExitStatus exitStatus)
{
    QStringList arguments;
    arguments.clear();
    arguments << "-i"
              << outputFolder + "/reconstruction/sfm_data.bin"
              << "-o"
              << outputFolder + "/reconstruction/scene.mvs";

    myProcess->start(openMVG_main_openMVG2openMVS, arguments);
}

void SfMContainer::perform_openMVS_DensifyPointCloud(int exitCode,
                                                     QProcess::ExitStatus exitStatus)
{
    QStringList arguments;
    arguments.clear();
    arguments << outputFolder + "/reconstruction/scene.mvs";

    std::cout << "DensifyPointCloud "<< DensifyPointCloud.toStdString() << std::endl;
    std::cout << "arguments " << std::endl;
    for(int i =0 ;i< arguments.size();i++)
    {
        std::cout << arguments[i].toStdString() << std::endl;
    }

    myProcess->start(DensifyPointCloud, arguments);

}

void SfMContainer::perform_openMVS_ReconstructMesh(int exitCode,
                                                   QProcess::ExitStatus exitStatus)
{
    QStringList arguments;
    arguments.clear();
    arguments << outputFolder + "/reconstruction/scene_dense.mvs";

    myProcess->start(ReconstructMesh, arguments);
}

void SfMContainer::perform_openMVS_RefineMesh(int exitCode,
                                              QProcess::ExitStatus exitStatus)
{
    QStringList arguments;
    arguments.clear();
    arguments << outputFolder + "/reconstruction/scene_dense_mesh.mvs";

    myProcess->start(RefineMesh, arguments);
}

void SfMContainer::perform_openMVS_TextureMesh(int exitCode,
                                               QProcess::ExitStatus exitStatus)
{
    QStringList arguments;
    arguments.clear();
    arguments << "--export-type"
              << "obj"
              << outputFolder + "/reconstruction/scene_dense_mesh.mvs";

    myProcess->start(TextureMesh, arguments);
}


