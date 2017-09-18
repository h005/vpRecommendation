#include "mesh.h"

#include <fstream>
#include <sstream>

#include <cstdio>
#include <cstdlib>

#include <QFile>

Mesh::Mesh(std::string mesh_path):
    indexBuf(QOpenGLBuffer::IndexBuffer),
    modelPath(mesh_path)
{
    initializeOpenGLFunctions();

    // Generate 2 VBOs
    arrayBuf.create();
    indexBuf.create();
    normalBuf.create();

    // Initializes cube geometry and transfers it to VBOs
    initMesh();

    color = QVector4D(0,0,1,1);
}

Mesh::~Mesh()
{
    arrayBuf.destroy();
    indexBuf.destroy();
    normalBuf.destroy();
}

void Mesh::initMesh()
{
//    loadMesh();
    load();

    arrayBuf.bind();
    arrayBuf.allocate(m_vertices.data(), m_vertices.size() * sizeof(QVector3D));

    indexBuf.bind();
    indexBuf.allocate(m_faces.data(), m_faces.size() * sizeof(unsigned int));

    normalBuf.bind();
    std::cout << "norm size " << m_normals.size() << std::endl;
    normalBuf.allocate(m_normals.data(), m_normals.size() * sizeof(QVector3D));
}

void Mesh::loadMesh()
{
    const aiScene* scene = aiImportFile( "/home/hejw005/Documents/learning/QtProject/QOpenGLWidgetTriangle/teapot.obj",
                                         aiProcess_CalcTangentSpace       |
                                         aiProcess_GenNormals             |
                                         aiProcess_Triangulate            |
                                         aiProcess_JoinIdenticalVertices  |
                                         aiProcess_SortByPType);

      if( !scene)
      {
        std::cout << aiGetErrorString() << std::endl;
        exit(1);
      }

      //loop over meshes
      for (int i = 0; i < scene->mNumMeshes; ++i)
      {
          //loop into current mesh for vertices
          for (int j = 0; j < scene->mMeshes[i]->mNumVertices; ++j)
          {
              QVector3D p = QVector3D(scene->mMeshes[i]->mVertices[j].x,
                                      scene->mMeshes[i]->mVertices[j].y,
                                      scene->mMeshes[i]->mVertices[j].z);
              m_vertices.push_back(p);
          }

          //loop into current mesh for indices
          for (int j = 0; j < scene->mMeshes[i]->mNumFaces; ++j)
          {
              aiFace f = scene->mMeshes[i]->mFaces[j];
              for (int k = 0; k < f.mNumIndices; ++k)
              {
                  m_faces.push_back(f.mIndices[k]);
              }
          }

          //loop into current mesh for normals, one normal per vertex
          for (int j = 0; j < scene->mMeshes[i]->mNumVertices; ++j)
          {
              QVector3D m = QVector3D(scene->mMeshes[i]->mNormals[j].x,
                                      scene->mMeshes[i]->mNormals[j].y,
                                      scene->mMeshes[i]->mNormals[j].z);
              m_normals.push_back(m);
          }
      }
      aiReleaseImport( scene);
}

std::pair<GLfloat, glm::mat4> Mesh::recommandScaleAndShift()
{
    GLfloat scale = drawScale();
    glm::mat4 shiftTransform = glm::translate(glm::mat4(1.f), glm::vec3(-scene_center.x, -scene_center.y, -scene_center.z));
    return std::make_pair(scale, shiftTransform);
}

static void get_bounding_box_for_node(const aiScene *sc,
    const aiNode* nd,
    aiVector3D* min,
    aiVector3D* max,
    aiMatrix4x4 &prev
    ){
    unsigned int n = 0, t;

    aiMatrix4x4 trafo = prev;
    trafo *= nd->mTransformation;

    for (; n < nd->mNumMeshes; ++n) {
        const aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];
        for (t = 0; t < mesh->mNumVertices; ++t) {

            aiVector3D tmp = mesh->mVertices[t];
            tmp *= trafo;

            min->x = aisgl_min(min->x, tmp.x);
            min->y = aisgl_min(min->y, tmp.y);
            min->z = aisgl_min(min->z, tmp.z);

            max->x = aisgl_max(max->x, tmp.x);
            max->y = aisgl_max(max->y, tmp.y);
            max->z = aisgl_max(max->z, tmp.z);
        }
    }

    for (n = 0; n < nd->mNumChildren; ++n) {
        get_bounding_box_for_node(sc, nd->mChildren[n], min, max, trafo);
    }
}

static void get_bounding_box(const aiScene *sc, aiVector3D* min, aiVector3D* max)
{
    // set identity
    aiMatrix4x4 rootTransformation;

    min->x = min->y = min->z = 1e10f;
    max->x = max->y = max->z = -1e10f;
    get_bounding_box_for_node(sc, sc->mRootNode, min, max, rootTransformation);
}

static std::string getBasePath(const std::string& path)
{
    size_t pos = path.find_last_of("\\/");
    return (std::string::npos == pos) ? "" : path.substr(0, pos + 1);
}

bool Mesh::load()
{
    modelPath = "/home/hejw005/Documents/learning/QtProject/QOpenGLWidgetTriangle/teapot.obj";
    pImporter = new Assimp::Importer();
    scene = pImporter->ReadFile(modelPath,
                                aiProcessPreset_TargetRealtime_Quality);
    if (!scene) {
        return 0;
    }
    // Generate Model Information
    get_bounding_box(scene, &scene_min, &scene_max);
    scene_center.x = (scene_min.x + scene_max.x) / 2.f;
    scene_center.y = (scene_min.y + scene_max.y) / 2.f;
    scene_center.z = (scene_min.z + scene_max.z) / 2.f;
    basePath = getBasePath(modelPath);

    return 1;
}

float Mesh::drawScale()
{
    float tmp = -1e10;
    tmp = aisgl_max(scene_max.x - scene_min.x, tmp);
    tmp = aisgl_max(scene_max.y - scene_min.y, tmp);
    tmp = aisgl_max(scene_max.z - scene_min.z, tmp);
    float scale = 2.f / tmp;
//    float scale = 100.f / tmp;
    return scale;
}

void Mesh::drawMesh(QOpenGLShaderProgram *program)
{
    // Tell OpenGL which VBOs to use
    arrayBuf.bind();
    indexBuf.bind();

    program->setUniformValue("objectColor", color);

    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = program->attributeLocation("a_position");
    int normalLocation = program->attributeLocation("a_normal");

    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(QVector3D));

    normalBuf.bind();
    program->enableAttributeArray(normalLocation);
    program->setAttributeBuffer(normalLocation, GL_FLOAT, 0, 3, sizeof(QVector3D));

    // Draw cube geometry using indices from VBO 1
    glDrawElements(GL_TRIANGLES, m_faces.size(), GL_UNSIGNED_INT, 0);
}
