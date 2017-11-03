#ifndef SFMCONTAINER_H
#define SFMCONTAINER_H

#include <QString>
#include <QProcess>

class SfMContainer : QObject
{
    Q_OBJECT

public:
    enum openMVG_Pipline{openMVG_main_SfMInit_ImageListing_Pipline,
                        openMVG_main_ComputeFeatures_Pipline,
                        openMVG_main_ComputeMatches_Pipline,
                        openMVG_main_IncrementalSfM_Pipline,
                        openMVG_main_IncrementalSfM_PiplineEnd,
                        openMVG_main_End};

    enum openMVS_Pipline{openMVG_main_openMVG2openMVS_Pipline,
                        openMVS_DensifyPointCloud_Pipline,
                        openMVS_ReconstructMesh_Pipline,
                        openMVS_RefineMesh_Pipline,
                        openMVS_TextureMesh_Pipline,
                        openMVS_End};

public:
    SfMContainer();
    ~SfMContainer();

    void setImgFolder(QString imgFolder);

    void setOutputFolder(QString outputFolder);

    void sfm_imgs2ptModel();

    void sfm_pt2MeshModel();

    void cleanFiles();

    void cleanLog();

    QString getOutputFolder();

private:

    bool checkFolder();

private slots:

    void reconstruction(int exitCode, QProcess::ExitStatus exitStatus);

    void imgs2ptModel(int exitCode, QProcess::ExitStatus exitStatus);

    void perform_openMVG_main_SfMInit_ImageListing(int exitCode, QProcess::ExitStatus exitStatus);

    void perform_openMVG_main_ComputeFeatures(int exitCode, QProcess::ExitStatus exitStatus);

    void perform_openMVG_main_ComputeMatches(int exitCode, QProcess::ExitStatus exitStatus);

    void perform_openMVG_main_IncrementalSfM(int exitCode, QProcess::ExitStatus exitStatus);

    void pt2MeshModel(int exitCode, QProcess::ExitStatus exitStatus);

    void perform_openMVG_main_openMVG2openMVS(int exitCode, QProcess::ExitStatus exitStatus);

    void perform_openMVS_DensifyPointCloud(int exitCode, QProcess::ExitStatus exitStatus);

    void perform_openMVS_ReconstructMesh(int exitCode, QProcess::ExitStatus exitStatus);

    void perform_openMVS_RefineMesh(int exitCode, QProcess::ExitStatus exitStatus);

    void perform_openMVS_TextureMesh(int exitCode, QProcess::ExitStatus exitStatus);

private:

    QProcess *myProcess;

    QString imgFolder;

    QString outputFolder;

    // related program
    QString basePath;
    QString openMVG_path;
    QString openMVS_path;
    // openMVG
    QString cameraDatabase;
    QString openMVG_main_SfMInit_ImageListing;
    QString openMVG_main_ComputeFeatures;
    QString openMVG_main_ComputeMatches;
    QString openMVG_main_IncrementalSfM;
    // convert openMVG format 2 openMVS format
    QString openMVG_main_openMVG2openMVS;
    // openMVS
    QString DensifyPointCloud;
    QString ReconstructMesh;
    QString RefineMesh;
    QString TextureMesh;

    int openMVG_pipLineStep;
    int openMVS_pipLineStep;

    int MVG_MVS;

};

#endif // SFMCONTAINER_H
