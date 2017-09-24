#include "sfmcontainer.h"

#include <QPlainTextEdit>
#include <iostream>

extern QPlainTextEdit *messageWidget;

SfMContainer::SfMContainer()
{
    imgFolder = "/home/hejw005/Documents/samSung/sfmTest/imgs";
    outputFolder = "/home/hejw005/Documents/samSung/sfmTest/result";

    basePath = "/home/hejw005/Documents/samSung/sfmTools";

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
    int exitCode = 0;
    QProcess::ExitStatus exitStatus = QProcess::NormalExit;
    if(!checkFolder())
    {
        std::cout << "please set the input and output folder first" << std::endl;
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
        std::cout << "please set the input and output folder first" << std::endl;
        return;
    }
    if(openMVG_pipLineStep != openMVG_main_End && openMVG_pipLineStep != openMVG_main_SfMInit_ImageListing_Pipline)
    {
        std::cout << "structure from motion is still running..." << std::endl;
        return;
    }
    if(openMVG_pipLineStep == openMVG_main_SfMInit_ImageListing_Pipline)
    {
        std::cout << "please run sfm first " << std::endl;
        return;
    }
    MVG_MVS = 2;
    openMVS_pipLineStep = openMVG_main_openMVG2openMVS_Pipline;
    reconstruction(exitCode, exitStatus);
//    pt2MeshModel(exitCode, exitStatus);
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
            perform_openMVG_main_SfMInit_ImageListing(exitCode, exitStatus);
            openMVG_pipLineStep++;
            break;
        case openMVG_main_ComputeFeatures_Pipline:
            std::cout << "perform_openMVG_main_SfMInit_ImageListing done" << std::endl;
            perform_openMVG_main_ComputeFeatures(exitCode, exitStatus);
            openMVG_pipLineStep++;
            break;
        case openMVG_main_ComputeMatches_Pipline:
            std::cout << "perform_openMVG_main_ComputeFeatures done" << std::endl;
            perform_openMVG_main_ComputeMatches(exitCode, exitStatus);
            openMVG_pipLineStep++;
            break;
        case openMVG_main_IncrementalSfM_Pipline:
            std::cout << "perform_openMVG_main_ComputeMatches done" << std::endl;
            perform_openMVG_main_IncrementalSfM(exitCode, exitStatus);
            openMVG_pipLineStep++;
            break;
        default:
            openMVG_pipLineStep++;
            std::cout << "perform_openMVG_main_IncrementalSfM done" << std::endl;
            break;
        }
    }
    else
    {
        switch (openMVG_pipLineStep) {
        case openMVG_main_ComputeFeatures_Pipline:
            std::cout << "perform_openMVG_main_SfMInit_ImageListing error" << std::endl;
            break;
        case openMVG_main_ComputeMatches_Pipline:
            std::cout << "perform_openMVG_main_ComputeFeatures error" << std::endl;
            break;
        case openMVG_main_IncrementalSfM_Pipline:
            std::cout << "perform_openMVG_main_ComputeMatches error" << std::endl;
            break;
        default:
            std::cout << "perform_openMVG_main_IncrementalSfM error" << std::endl;
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
    std::cout << "exitCode " << exitCode << std::endl;
    std::cout << "QProcess ExitStatus " << exitStatus << std::endl;

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
            perform_openMVG_main_openMVG2openMVS(exitCode, exitStatus);
            openMVS_pipLineStep++;
            break;
        case openMVS_DensifyPointCloud_Pipline:
            std::cout << "perform_openMVG_main_openMVG2openMVS done" << std::endl;
            perform_openMVS_DensifyPointCloud(exitCode, exitStatus);
            openMVS_pipLineStep++;
            break;
        case openMVS_ReconstructMesh_Pipline:
            std::cout << "perform_openMVS_DensifyPointCloud done" << std::endl;
            perform_openMVS_ReconstructMesh(exitCode, exitStatus);
            openMVS_pipLineStep++;
            break;
        case openMVS_RefineMesh_Pipline:
            std::cout << "perform_openMVS_ReconstructMesh done" << std::endl;
            perform_openMVS_RefineMesh(exitCode, exitStatus);
            openMVS_pipLineStep++;
            break;
        case openMVS_TextureMesh_Pipline:
            std::cout << "perform_openMVS_RefineMesh done" << std::endl;
            perform_openMVS_TextureMesh(exitCode, exitStatus);
            break;
        default:
            std::cout << "perform_openMVS_TextureMesh done" << std::endl;
            openMVS_pipLineStep++;
            break;
        }
    }
    else
    {
        switch (openMVG_pipLineStep) {
        case openMVS_DensifyPointCloud_Pipline:
            std::cout << "perform_openMVG_main_openMVG2openMVS error" << std::endl;
            break;
        case openMVS_ReconstructMesh_Pipline:
            std::cout << "perform_openMVS_DensifyPointCloud error" << std::endl;
            break;
        case openMVS_RefineMesh_Pipline:
            std::cout << "perform_openMVS_ReconstructMesh error" << std::endl;
            break;
        case openMVS_TextureMesh_Pipline:
            std::cout << "perform_openMVS_RefineMesh error" << std::endl;
            break;
        default:
            std::cout << "perform_openMVS_TextureMesh error" << std::endl;
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
              << outputFolder + "/reconstruction/scene_mesh.mvs";

    myProcess->start(TextureMesh, arguments);
}


