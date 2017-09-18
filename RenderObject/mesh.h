#ifndef MESH_H
#define MESH_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

#include <iostream>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/DefaultLogger.hpp"
#include "assimp/LogStream.hpp"

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)

class Mesh : protected QOpenGLFunctions
{
public:
    Mesh(std::string mesh_path);
    virtual ~Mesh();

    void drawMesh(QOpenGLShaderProgram *program);

    void loadMesh();

    std::pair<GLfloat, glm::mat4> recommandScaleAndShift();

    bool load();

private:
    float drawScale();
    void initMesh();

    QVector4D color;

    std::string modelPath;

    std::vector<QVector3D> m_vertices;
    std::vector<QVector2D> m_texcoord;
    std::vector<unsigned int> m_faces;
    std::vector<QVector3D> m_normals;

    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
    QOpenGLBuffer normalBuf;

    // model importer
    Assimp::Importer *pImporter;
    const aiScene *scene;
    std::string basePath;
    aiVector3D scene_min, scene_max, scene_center;
};

#endif // MESH_H
