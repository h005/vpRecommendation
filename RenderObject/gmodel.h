#ifndef GMODEL_H
#define GMODEL_H

#include <map>
#include <utility>
#include <glm/glm.hpp>

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/DefaultLogger.hpp"
#include "assimp/LogStream.hpp"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>

typedef std::map<std::string, GLuint *> TextureIdMapType;

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)

void *imgData(const char *texturePath, int &width, int &height);

class GModel
{
    class MeshEntry {
        enum BUFFERS {
            VERTEX_BUFFER, TEXCOORD_BUFFER, NORMAL_BUFFER, TRIANGLE_INDEX_BUFFER, POLYGON_INDEX_BUFFER, BUFFER_COUNT
        };
    private:
        // 该mesh最终的Model矩阵
        GLuint m_vao;
        GLuint m_vbo[BUFFER_COUNT];
        int triangular_vertex_count;
        int polygon_vertex_count;

    public:
        const aiMesh *mesh;
        glm::mat4 finalTransformation;

        MeshEntry(const aiMesh *mesh, const glm::mat4 &transformation, GLuint programID);
        ~MeshEntry();
        void render();
    };

private:
    Assimp::Importer *pImporter;
    const aiScene *scene;
    std::string basePath;
    TextureIdMapType textureIdMap;
    GLuint *textureIds;
    std::vector<MeshEntry *> meshEntries;
    GLuint m_programID = 0;

public:
    aiVector3D scene_min, scene_max, scene_center;

    GModel();
    bool load(const char *modelPath);
    bool hasModel();
    void bindDataToGL();
    void cleanUp();
    void draw(const glm::mat4 &inheritModelView, const glm::mat4 &projection);
    glm::mat4 getInnerTransformation();
    std::pair<GLfloat, glm::mat4> recommandScaleAndShift();
    ~GModel();

private:
    float drawScale();
    void apply_material(const aiMaterial *mtl);
    void recursive_create(const aiScene *sc, const aiNode* nd, const glm::mat4 &inheritedTransformation);

};

#endif // GMODEL_H
