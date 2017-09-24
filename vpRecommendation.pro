#-------------------------------------------------
#
# Project created by QtCreator 2017-09-05T20:33:53
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = vpRecommendation
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    main.cpp \
    mainwindow.cpp \
    imglabel.cpp \
    imgset.cpp \
    features/feaimg.cpp \
    features/lineSegmentFeature/vpdetection.cpp \
    features/lineSegmentFeature/lsd.cpp \
    features/lineSegmentFeature/linesegmentfea.cpp \
    features/lineSegmentFeature/linesegmentstart.cpp \
    features/gistFeature/gist.cpp \
    features/gistFeature/standalone_image.cpp \
    features/gistFeature/libgist.cpp \
    OpenGLWidget/dragablewidget.cpp \
    OpenGLWidget/myglwidget.cpp \
    OpenGLWidget/trackball.cpp \
    OpenGLWidget/glwidget.cpp \
    OpenGLWidget/shader.cpp \
    RenderObject/mesh.cpp \
    RenderObject/GModel.cpp \
    RenderObject/sphere.cpp \
    RenderObject/baserenderobject.cpp \
    RenderObject/pointsmatchrelation.cpp \
    features/feageo.cpp \
    svm2k/predictor.cpp \
    viewpointset.cpp \
    SfM/sfmcontainer.cpp

HEADERS += \
    mainwindow.h \
    imglabel.h \
    imgset.h \
    features/feaimg.h \
    features/lineSegmentFeature/vpdetection.h \
    features/lineSegmentFeature/lsd.h \
    features/lineSegmentFeature/linesegmentfea.h \
    features/gistFeature/gist.h \
    features/gistFeature/standalone_image.h \
    features/gistFeature/libgist.h \
    features/gistFeature/clany/clany_defs.h \
    OpenGLWidget/dragablewidget.h \
    OpenGLWidget/myglwidget.h \
    OpenGLWidget/trackball.h \
    OpenGLWidget/glwidget.h \
    OpenGLWidget/shader.hpp \
    RenderObject/mesh.h \
    RenderObject/GModel.h \
    RenderObject/sphere.h \
    RenderObject/baserenderobject.h \
    RenderObject/pointsmatchrelation.h \
    features/feageo.h \
    svm2k/predictor.h \
    viewpointset.h \
    SfM/sfmcontainer.h

DISTFILES += \
    shader/simple.vert \
    shader/simple.frag



FORMS += \
        mainwindow.ui

# fftw3
LIBS += -L/usr/local/lib/ \
        -lfftw3f

# glew
LIBS += -lGLEW -lGLU -lGL

# assimp
LIBS += -lassimp

# opencv
INCLUDEPATH += /usr/local/include \
                /usr/local/include/opencv \
                /usr/local/include/opencv2 \

LIBS += /usr/local/lib/libopencv_aruco.so.3.2 \
        /usr/local/lib/libopencv_bgsegm.so.3.2 \
        /usr/local/lib/libopencv_bioinspired.so.3.2 \
        /usr/local/lib/libopencv_calib3d.so.3.2 \
        /usr/local/lib/libopencv_ccalib.so.3.2 \
        /usr/local/lib/libopencv_core.so.3.2 \
        /usr/local/lib/libopencv_datasets.so.3.2 \
        /usr/local/lib/libopencv_dnn.so.3.2 \
        /usr/local/lib/libopencv_dpm.so.3.2 \
        /usr/local/lib/libopencv_face.so.3.2 \
        /usr/local/lib/libopencv_features2d.so.3.2 \
        /usr/local/lib/libopencv_flann.so.3.2 \
        /usr/local/lib/libopencv_fuzzy.so.3.2 \
        /usr/local/lib/libopencv_hdf.so.3.2 \
        /usr/local/lib/libopencv_highgui.so.3.2 \
        /usr/local/lib/libopencv_imgcodecs.so.3.2 \
        /usr/local/lib/libopencv_imgproc.so.3.2 \
        /usr/local/lib/libopencv_line_descriptor.so.3.2 \
        /usr/local/lib/libopencv_ml.so.3.2 \
        /usr/local/lib/libopencv_objdetect.so.3.2 \
        /usr/local/lib/libopencv_optflow.so.3.2 \
        /usr/local/lib/libopencv_photo.so.3.2 \
        /usr/local/lib/libopencv_plot.so.3.2 \
        /usr/local/lib/libopencv_reg.so.3.2 \
        /usr/local/lib/libopencv_rgbd.so.3.2 \
        /usr/local/lib/libopencv_saliency.so.3.2 \
#        /usr/local/lib/libopencv_sfm.so.3.2 \
        /usr/local/lib/libopencv_shape.so.3.2 \
        /usr/local/lib/libopencv_stereo.so.3.2 \
        /usr/local/lib/libopencv_stitching.so.3.2 \
        /usr/local/lib/libopencv_structured_light.so.3.2 \
        /usr/local/lib/libopencv_superres.so.3.2 \
        /usr/local/lib/libopencv_surface_matching.so.3.2 \
        /usr/local/lib/libopencv_text.so.3.2 \
        /usr/local/lib/libopencv_tracking.so.3.2 \
        /usr/local/lib/libopencv_videoio.so.3.2 \
        /usr/local/lib/libopencv_video.so.3.2 \
        /usr/local/lib/libopencv_videostab.so.3.2 \
        /usr/local/lib/libopencv_viz.so.3.2 \
        /usr/local/lib/libopencv_xfeatures2d.so.3.2 \
        /usr/local/lib/libopencv_ximgproc.so.3.2 \
        /usr/local/lib/libopencv_xobjdetect.so.3.2 \
        /usr/local/lib/libopencv_xphoto.so.3.2

RESOURCES += \
    shaders.qrc \
    svm2kdata.qrc


# glm
# sudo apt-get install libglm-dev
# glew
# sudo apt-get install libglew-dev
