#include "myglwidget.h"

#include <QMouseEvent>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>

MyGLWidget::MyGLWidget(const QString &modelPath, QWidget *parent)
    : DragableWidget(parent),
      modelPath(modelPath)
//      m_mesh(0)
{
    this->modelPath = modelPath;
//    model.load(modelPath.toStdString().c_str());
//    m_mesh = new Mesh(modelPath.toStdString());
//    m_mesh->load();
//    std::pair<GLfloat, glm::mat4> scaleAndShift = m_mesh->recommandScaleAndShift();
//    m_scaleBeforeRender = scaleAndShift.first;
//    m_shiftBeforeRender = scaleAndShift.second;



    matrix.translate(0.0,0.0,-5.0);

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setSamples(16);
    format.setSwapInterval(0);
    QSurfaceFormat::setDefaultFormat(format);
    this->setFormat(format);


    // camera position
    m_scaleBeforeRender = 1.0;
    cameraPos = glm::vec3(0.f,0.f,0.f);
    flag_move = false;
    camPosLength = 30.f;
    rate = 0.05;

}

MyGLWidget::~MyGLWidget()
{
    makeCurrent();
    delete m_mesh;
//    model.cleanUp();
    doneCurrent();
}

void MyGLWidget::initializeGL()
{
//    QOpenGLFunctions_3_3_Core *f = QOpenGLContext::currentContext()->functions();
//    f->glClearColor(1.0f,1.0f,1.0f,1.0f);

    initializeOpenGLFunctions();

    glClearColor(0,0,0,1);

    // our camera never changes in this example
    m_camera = glm::lookAt(glm::vec3(0.f, 0.f, camPosLength), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));

    // load data for model rendering
//    model.bindDataToGL();

    initShaders();

    glEnable(GL_DEPTH_TEST);

    m_mesh = new Mesh(modelPath.toStdString());


}

void MyGLWidget::resizeGL(int w, int h)
{
//    qreal aspect = qreal(w) / qreal(h ? h : 1);

//    const qreal zNear = 3.0, zFar = 30.0, fov = 45.0;

//    projection.setToIdentity();
//    projection.perspective(fov, aspect, zNear, zFar);
    m_proj = glm::perspective(glm::pi<float>() / 3, GLfloat(w) / h, 0.1f, 30.0f);
}

void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // get model-view matrix
    glm::mat4 modelViewMatrix = m_camera * getModelMatrix();

//    model.draw(modelViewMatrix, m_proj);

    program.setUniformValue("normal_matrix", normal);

    QMatrix4x4 mvpMatrix = QMatrix4x4(glm::value_ptr(m_proj * modelViewMatrix));
    program.setUniformValue("mvp_matrix", mvpMatrix);

//    std::cout << glm::to_string(modelViewMatrix) << std::endl;
//    std::cout << "m_scaleBeforeRender " << m_scaleBeforeRender
//              << " m_shiftBeforeRender " << glm::to_string(m_shiftBeforeRender) << std::endl;

    m_lightPosLoc = program.uniformLocation("lightPos");
    program.setUniformValue(m_lightPosLoc, QVector3D(0, 0, 70));

    m_mesh->drawMesh(&program);

}

void MyGLWidget::initShaders()
{
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":shader/simple.vert"))
        close();

    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":shader/simple.frag"))
        close();

    if (!program.link())
        close();

    if (!program.bind())
        close();
}

glm::mat4 MyGLWidget::getModelViewMatrix()
{
    return m_camera * getModelMatrix();
}

glm::mat4 MyGLWidget::getModelMatrix()
{
    return DragableWidget::getModelMatrix() * glm::scale(glm::mat4(1.f), glm::vec3(m_scaleBeforeRender)) * m_shiftBeforeRender;
}

void MyGLWidget::mousePressEvent(QMouseEvent *e)
{
    DragableWidget::mousePressEvent(e);
    if(e->button() == Qt::MiddleButton)
    {
        flag_move = true;
        m_lastPos = e->pos();
    }
}

void MyGLWidget::mouseReleaseEvent(QMouseEvent *e)
{
    DragableWidget::mouseReleaseEvent(e);
    flag_move = false;
}

void MyGLWidget::mouseMoveEvent(QMouseEvent *e)
{
    DragableWidget::mouseMoveEvent(e);
    if(flag_move)
    {
        QPointF pos = m_lastPos - e->pos();
        cameraPos.r += pos.rx() * rate;
        cameraPos.g -= pos.ry() * rate;
        m_camera = glm::lookAt(glm::vec3(0.f, 0.f, camPosLength) + cameraPos, glm::vec3(0.f) + cameraPos, glm::vec3(0.f, 1.f, 0.f));
        update();
    }
}

static void qNormalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}
