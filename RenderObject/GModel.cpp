﻿#include "GModel.h"

#include <opencv2/opencv.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "OpenGLWidget/shader.hpp"

void *imgData(const char *texturePath, int &width, int &height) {
    cv::Mat img = cv::imread(texturePath);
    if (img.rows <= 0 || img.cols <= 0) {
        return NULL;
    }
    assert(img.channels() == 3);

    size_t mem = img.rows * img.cols * img.channels();
    uchar *data = new uchar[mem];
    size_t rowSize = img.cols * img.channels();
    uchar *pivot = data;
    for (int i = 0; i < img.rows; i++) {
        uchar *ptr = img.ptr(i);
        memcpy(pivot, ptr, rowSize);
        pivot += rowSize;
    }
    assert(pivot - data == mem);
    width = img.size().width;
    height = img.size().height;
    return data;
}

// Can't send color down as a pointer to aiColor4D because AI colors are ABGR.
static void Color4f(const aiColor4D *color)
{
    glColor4f(color->r, color->g, color->b, color->a);
}

static void set_float4(float f[4], float a, float b, float c, float d)
{
    f[0] = a;
    f[1] = b;
    f[2] = c;
    f[3] = d;
}

static void color4_to_float4(const aiColor4D *c, float f[4])
{
    f[0] = c->r;
    f[1] = c->g;
    f[2] = c->b;
    f[3] = c->a;
}

static std::string getBasePath(const std::string& path)
{
    size_t pos = path.find_last_of("\\/");
    return (std::string::npos == pos) ? "" : path.substr(0, pos + 1);
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

void GModel::apply_material(const aiMaterial *mtl)
{
    float c[4];

    GLenum fill_mode;
    int ret1, ret2;
    aiColor4D diffuse;
    aiColor4D specular;
    aiColor4D ambient;
    aiColor4D emission;
    float shininess, strength;
    int two_sided;
    int wireframe;
    unsigned int max;	// changed: to unsigned

    int texIndex = 0;
    aiString texPath;	//contains filename of texture

    if (AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, texIndex, &texPath) && textureIdMap[texPath.data])
    {
        //bind texture
        unsigned int texId = *textureIdMap[texPath.data];
        glBindTexture(GL_TEXTURE_2D, texId);
    }

    set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
    if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse)) {
        color4_to_float4(&diffuse, c);
        std::cout << "AI_MATKEY_COLOR_DIFFUSE"  << c[0] << " " << c[1] << " " << c[2] << std::endl;
    }
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

    set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
    if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular)) {
        color4_to_float4(&specular, c);
        std::cout << "AI_MATKEY_COLOR_SPECULAR" << std::endl;
    }
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

    set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
    if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
        color4_to_float4(&ambient, c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

    set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
    if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
        color4_to_float4(&emission, c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

    max = 1;
    ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
    max = 1;
    ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
    if ((ret1 == AI_SUCCESS) && (ret2 == AI_SUCCESS))
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
    else {
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
        set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
    }

    max = 1;
    if (AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, &max))
        fill_mode = wireframe ? GL_LINE : GL_FILL;
    else
        fill_mode = GL_FILL;
    glPolygonMode(GL_FRONT_AND_BACK, fill_mode);

    max = 1;
    if ((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
}



void GModel::recursive_create(const aiScene *sc, const aiNode* nd, const glm::mat4 &inheritedTransformation)
{
    assert(nd && sc);
    unsigned int n = 0;

    // 相对于父节点的变换，aiMatrix4x4中是行优先存储的，
    // 所以需要先将内存结构变为列优先存储
    glm::mat4 mTransformation = glm::transpose(glm::make_mat4((float *)&nd->mTransformation));
    glm::mat4 absoluteTransformation = inheritedTransformation * mTransformation;
    // 设置shader中的model view矩阵

    for (; n < nd->mNumMeshes; ++n)
    {
        // 一个aiNode中存有其mesh的索引，
        // 在aiScene中可以用这个索引拿到真正的aiMesh
        const struct aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];
        meshEntries.push_back(new GModel::MeshEntry(mesh, absoluteTransformation, m_programID));
    }


    // create all children
    for (n = 0; n < nd->mNumChildren; ++n)
    {
        recursive_create(sc, nd->mChildren[n], absoluteTransformation);
    }
}

void GModel::getVerticesAndFacesRecursive(const aiScene *sc,
                                          const aiNode *nd,
                                          const glm::mat4 &inheritedTransformation,
                                          std::vector<GLfloat> &vertice,
                                          std::vector<GLuint> &indices)
{
    unsigned int n = 0;

    glm::mat4 mTransformation = glm::transpose(glm::make_mat4((float *)&nd->mTransformation));
    glm::mat4 absoluteTransformation = inheritedTransformation * mTransformation;

    for (; n < nd->mNumMeshes; ++n)
    {
        // 一个aiNode中存有其mesh的索引，
        // 在aiScene中可以用这个索引拿到真正的aiMesh
        const struct aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];
//        meshEntries.push_back(new GModel::MeshEntry(mesh, absoluteTransformation, m_programID));
        if(mesh->HasPositions())
        {
            for(int i=0;i<mesh->mNumVertices;i++)
            {
                glm::vec3 position(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
                glm::vec4 absolutePosition = absoluteTransformation * glm::vec4(position, 1.f);

                vertice.push_back(absolutePosition.x);
                vertice.push_back(absolutePosition.y);
                vertice.push_back(absolutePosition.z);
            }
        }

        if(mesh->HasFaces())
        {
            int base = vertice.size() / 3;
            for(int i = 0; i < (int)mesh->mNumFaces; ++i)
            {
                for(int j = 0; j < (int)mesh->mFaces[i].mNumIndices - 2; j++) // 要改成int，并强制类型转换，不然会越界
                {
                    indices.push_back(base + mesh->mFaces[i].mIndices[0]);
                    indices.push_back(base + mesh->mFaces[i].mIndices[j+1]);
                    indices.push_back(base + mesh->mFaces[i].mIndices[j+2]);
                }
            }
        }
    }

    // create all children
    for (n = 0; n < nd->mNumChildren; ++n)
    {
        getVerticesAndFacesRecursive(sc,
                                     nd->mChildren[n],
                                     absoluteTransformation,
                                     vertice,
                                     indices);
    }

}

GModel::GModel()
{
    pImporter = NULL;
    scene = NULL;
    textureIds = NULL;
    m_programID = 0;
}

void GModel::getVerticesAndFaces(std::vector<GLfloat> &vertice, std::vector<GLuint> &indices)
{
    vertice.clear();
    indices.clear();
    getVerticesAndFacesRecursive(scene, scene->mRootNode, glm::mat4(), vertice, indices);
}

bool GModel::load(const char *modelPath) {
    cleanUp();

    pImporter = new Assimp::Importer();
    scene = pImporter->ReadFile(modelPath, aiProcessPreset_TargetRealtime_Quality);
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

void GModel::bindDataToGL() {
    // 读入shader程序并编译
    // 需要在OpenGL环境下调用，放在这里合适
    m_programID = LoadShaders( "shader/simpleShader.vert", "shader/simpleShader.frag" );

    // 1.将纹理读取到显存
    // 2.递归创建meshEntry
    textureIdMap.clear();
    std::cout << "scene materials " << scene->mNumMaterials << std::endl;
    for (unsigned int m = 0; m<scene->mNumMaterials; m++)
    {
        int texIndex = 0;
        aiReturn texFound;
        aiString path;	// filename
        while ((texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path)) == AI_SUCCESS)
        {
            textureIdMap[path.data] = NULL;
            texIndex++;
        }
    }

    size_t numTextures = textureIdMap.size();
    textureIds = new GLuint[numTextures];
    glGenTextures(numTextures, textureIds);
    TextureIdMapType::iterator it;
    int i;
    for (i = 0, it = textureIdMap.begin(); it != textureIdMap.end(); it++, i++) {
        std::string filename = basePath + it->first;

        glBindTexture(GL_TEXTURE_2D, textureIds[i]); /* Binding of texture name */
        //redefine standard texture values
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); /* We will use linear
                                                                          interpolation for magnification filter */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); /* We will use linear
                                                                          interpolation for minifying filter */

        int width, height;
        // 读入数据是b,g,r依次排列的
        void *data = imgData(filename.c_str(), width, height);
        if (data) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            // https://www.opengl.org/sdk/docs/man/html/glTexImage2D.xhtml
            // 参数中的第一个GL_RGB表示，数据有三个通道
            // 第二个GL_BGR表示数据是b,g,r排列的，它会将bgr分别放在显存中的bgr分量上，方便shader使用
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width,
                height, 0, GL_BGR, GL_UNSIGNED_BYTE,
                data); /* Texture specification */
            delete data;

            it->second = &textureIds[i];
        }
    }

    recursive_create(scene, scene->mRootNode, glm::mat4());
}


void GModel::draw(const glm::mat4 &inheritModelView, const glm::mat4 &projection) {
    assert(scene != NULL);
    glPushAttrib(GL_ENABLE_BIT);

    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    // 传入的变换已经含有移中和缩放了
    glm::mat4 transformation = inheritModelView;

    // 绑定使用的shader并设置其中的投影矩阵
    glUseProgram(m_programID);
    GLuint projMatrixID = glGetUniformLocation(m_programID, "projMatrix");
    GLuint mvMatrixID = glGetUniformLocation(m_programID, "mvMatrix");
    GLuint normalMatrixID = glGetUniformLocation(m_programID, "normalMatrix");
    GLuint TextureID = glGetUniformLocation(m_programID, "myTextureSampler");
    GLuint diffuseID = glGetUniformLocation(m_programID, "diffuse");
//    GLuint vColorId = glGetUniformLocation(m_programID,"myVertexColor");
    glUniformMatrix4fv(projMatrixID, 1, GL_FALSE, glm::value_ptr(projection));

    std::vector<GModel::MeshEntry *>::iterator it;
    for (it = meshEntries.begin(); it != meshEntries.end(); it++) {
        // 绘制一个mesh主要分为下面几个步骤
        //
        // 1.应用材质、纹理
        // 2 设置Transformation
        // 3.调用MeshEntry的绘制函数:
        // 3.1 切换vao
        // 3.2 绘制三角形

        const aiMesh *mesh = (*it)->mesh;
        // 一个aiMesh拥有一致的纹理和材质
        glActiveTexture(GL_TEXTURE0);

        // 应用材质或环境光
        {
            auto mtl = scene->mMaterials[mesh->mMaterialIndex];

            unsigned int texId = 0;
            int texIndex = 0;
            aiString texPath;	//contains filename of texture
            if (AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, texIndex, &texPath) && textureIdMap[texPath.data])
            {
                //bind texture
                texId = *textureIdMap[texPath.data];
                glBindTexture(GL_TEXTURE_2D, texId);
            }
            glBindTexture(GL_TEXTURE_2D, texId);
            glUniform1i(TextureID, GL_TEXTURE0);


            glm::vec4 c(0.f);
            if (texId == 0) { // 纹理不存在，则使用材质颜色
                 aiColor4D diffuse;
                 if (texId == 0 && AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
                     color4_to_float4(&diffuse, glm::value_ptr(c));
            }
            glUniform4fv(diffuseID, 1, glm::value_ptr(c));
        }
//        std::cout << "has vertex Color "<< mesh->HasVertexColors(55) << std::endl;
        // 为每个顶点加入颜色
//        GLfloat *vColor = new GLfloat[mesh->mNumVertices * 4];
//        for(int i=0;i<mesh->mNumVertices;i++)
//        {
//            vColor[i * 0 + 0] = mesh->mColors[i]->r;
//            vColor[i * 0 + 1] = mesh->mColors[i]->g;
//            vColor[i * 0 + 2] = mesh->mColors[i]->b;
//            vColor[i * 0 + 3] = mesh->mColors[i]->a;
//            std::cout << "vertex color output " << std::endl;
//        }
//        glUniform4fv(vColorId, mesh->mNumVertices, vColor);

        // 计算最终的model view矩阵以及对应的法向变换矩阵
        glm::mat4 modelViewMatrix = transformation * (*it)->finalTransformation;
        glm::mat3 gl_NormalMatrix = glm::inverseTranspose(glm::mat3(modelViewMatrix));
        // 在shader程序中设置
        glUniformMatrix4fv(mvMatrixID, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));
        glUniformMatrix4fv(normalMatrixID, 1, GL_FALSE, glm::value_ptr(gl_NormalMatrix));

        (*it)->render();
    }

    glPopAttrib();

}

std::pair<GLfloat, glm::mat4> GModel::recommandScaleAndShift()
{
    GLfloat scale = drawScale();
    glm::mat4 shiftTransform = glm::translate(glm::mat4(1.f), glm::vec3(-scene_center.x, -scene_center.y, -scene_center.z));
    return std::make_pair(scale, shiftTransform);
}

void GModel::cleanUp() {
    if (pImporter) {
        delete pImporter;
        pImporter = NULL;
    }

    if (textureIds) {
        glDeleteTextures(textureIdMap.size(), textureIds);
        delete textureIds;
        textureIds = NULL;
    }
    if (m_programID) {
        glDeleteProgram(m_programID);
        m_programID = 0;
    }

    scene = NULL;

    std::vector<GModel::MeshEntry *>::iterator it;
    for (it = meshEntries.begin(); it != meshEntries.end(); it++) {
        delete *it;
    }
    meshEntries.clear();
}

bool GModel::hasModel() {
    return scene != NULL;
}

float GModel::drawScale() {
    float tmp = -1e10;
    tmp = aisgl_max(scene_max.x - scene_min.x, tmp);
    tmp = aisgl_max(scene_max.y - scene_min.y, tmp);
    tmp = aisgl_max(scene_max.z - scene_min.z, tmp);
    float scale = 2.f / tmp;
//    float scale = 100.f / tmp;
    return scale;
}


GModel::~GModel()
{
}

GModel::MeshEntry::MeshEntry(const aiMesh *mesh, const glm::mat4 &transformation, GLuint programID)
    : finalTransformation(transformation), mesh(mesh) {
    m_vbo[VERTEX_BUFFER] = 0;
    m_vbo[TEXCOORD_BUFFER] = 0;
    m_vbo[NORMAL_BUFFER] = 0;
    m_vbo[TRIANGLE_INDEX_BUFFER] = 0;
    m_vbo[POLYGON_INDEX_BUFFER] = 0;

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    if(mesh->HasPositions()) {
        std::vector<GLfloat> vertices;
        for(int i = 0; i < mesh->mNumVertices; ++i) {
            vertices.push_back(mesh->mVertices[i].x);
            vertices.push_back(mesh->mVertices[i].y);
            vertices.push_back(mesh->mVertices[i].z);
        }

        assert(vertices.size() % 3 == 0);
        if (vertices.size() > 0) {
            glGenBuffers(1, &m_vbo[VERTEX_BUFFER]);
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo[VERTEX_BUFFER]);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

            GLuint vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
            glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray (vertexPosition_modelspaceID);
        }
    }


    if(mesh->HasTextureCoords(0)) {
        std::vector<GLfloat> texCoords;
        for(int i = 0; i < mesh->mNumVertices; ++i) {
            texCoords.push_back(mesh->mTextureCoords[0][i].x);
            texCoords.push_back(1.f - mesh->mTextureCoords[0][i].y);
        }

        assert(texCoords.size() % 2 == 0);
        if (texCoords.size() > 0) {
            glGenBuffers(1, &m_vbo[TEXCOORD_BUFFER]);
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo[TEXCOORD_BUFFER]);
            glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(GLfloat), &texCoords[0], GL_STATIC_DRAW);

            GLuint vertexUVID = glGetAttribLocation(programID, "vertexUV");
            glVertexAttribPointer(vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray (vertexUVID);
        }
    }

    if(mesh->HasNormals()) {
        std::vector<GLfloat> normals;
        for(int i = 0; i < mesh->mNumVertices; ++i) {
            normals.push_back(mesh->mNormals[i].x);
            normals.push_back(mesh->mNormals[i].y);
            normals.push_back(mesh->mNormals[i].z);
        }

        // 必须是3的倍数
        assert(normals.size() % 3 == 0);
        if (normals.size() > 0) {
            glGenBuffers(1, &m_vbo[NORMAL_BUFFER]);
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo[NORMAL_BUFFER]);
            glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), &normals[0], GL_STATIC_DRAW);

            GLuint vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");
            glVertexAttribPointer(vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray (vertexNormal_modelspaceID);
        }
    }

    if (mesh->HasVertexColors(0))
        std::cout << "mesh->HasVertexColors(0)" << std::endl;

    if(mesh->HasFaces()) {
        std::vector<GLuint> indices;
        // 一个face代表一个面（暂时只考虑三角形，其余类型pass），其存储着各个顶点的索引
        // 可以根据索引到mesh->mVertices[]中找到对应顶点的数据(x, y, z)
        for(uint32_t i = 0; i < mesh->mNumFaces; ++i) {
            if (mesh->mFaces[i].mNumIndices == 3) {
                indices.push_back(mesh->mFaces[i].mIndices[0]);
                indices.push_back(mesh->mFaces[i].mIndices[1]);
                indices.push_back(mesh->mFaces[i].mIndices[2]);
            }
        }

        // 由于采用了vbo索引，元素数目一般大于mesh->mNumVertices
        // elementCount是顶点的数目
        triangular_vertex_count = indices.size();

        if (indices.size() > 0) {
            // 注意索引缓存的声明方式, GL_ELEMENT_ARRAY_BUFFER，上面是GL_ARRAY_BUFFER
            glGenBuffers(1, &m_vbo[TRIANGLE_INDEX_BUFFER]);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[TRIANGLE_INDEX_BUFFER]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
            // 索引不需要调用 glVertexAttribPointer 和 glEnableVertexAttribArray
        }
    }

    if(mesh->HasFaces()) {
        // 多边形，使用线性buffer存储
        // 用"隔板"分割各个多边形
        std::vector<GLuint> indices;
        for(uint32_t i = 0; i < mesh->mNumFaces; ++i) {
            if (mesh->mFaces[i].mNumIndices > 3) {
                for (uint32_t j = 0; j < mesh->mFaces[i].mNumIndices; j++) {
                    indices.push_back(mesh->mFaces[i].mIndices[j]);
                }
                // 插入隔板
                indices.push_back(GL_PRIMITIVE_RESTART_FIXED_INDEX);
            }
        }

        polygon_vertex_count = indices.size();
        if (indices.size() > 0) {
            // 注意索引缓存的声明方式
            glGenBuffers(1, &m_vbo[POLYGON_INDEX_BUFFER]);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[POLYGON_INDEX_BUFFER]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

/**
*	Deletes the allocated OpenGL buffers
**/
GModel::MeshEntry::~MeshEntry() {
    if(m_vbo[VERTEX_BUFFER]) {
        glDeleteBuffers(1, &m_vbo[VERTEX_BUFFER]);
    }

    if(m_vbo[TEXCOORD_BUFFER]) {
        glDeleteBuffers(1, &m_vbo[TEXCOORD_BUFFER]);
    }

    if(m_vbo[NORMAL_BUFFER]) {
        glDeleteBuffers(1, &m_vbo[NORMAL_BUFFER]);
    }

    if(m_vbo[TRIANGLE_INDEX_BUFFER]) {
        glDeleteBuffers(1, &m_vbo[TRIANGLE_INDEX_BUFFER]);
    }

    if(m_vbo[POLYGON_INDEX_BUFFER]) {
        glDeleteBuffers(1, &m_vbo[POLYGON_INDEX_BUFFER]);
    }

    glDeleteVertexArrays(1, &m_vao);
}

/**
*	Renders this MeshEntry
**/
void GModel::MeshEntry::render() {
    // 3.调用MeshEntry的绘制函数:
    // 3.1 切换vao
    // 3.2 绘制三角形
    // 3.3 绘制多边形
    glBindVertexArray(m_vao);
    glDisable(GL_CULL_FACE);

    if (m_vbo[TRIANGLE_INDEX_BUFFER]) {
        // 绘制三角形前关闭重置选项，因为三角形绘制不需要“隔板”
        // 若开启且不设置PrimitiveIndex可能会造成面绘制错乱
        // http://stackoverflow.com/questions/26944959/opengl-separating-polygons-inside-vbo
        glDisable(GL_PRIMITIVE_RESTART);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[TRIANGLE_INDEX_BUFFER]);
        glDrawElements(GL_TRIANGLES, triangular_vertex_count, GL_UNSIGNED_INT, NULL);
        // 注意和下面的区别，上面的使用顶点索引，下面的不使用顶点索引
        //glDrawArrays(GL_TRIANGLES, 0, mesh->mNumVertices);
    }

    if (m_vbo[POLYGON_INDEX_BUFFER]) {
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(GL_PRIMITIVE_RESTART_FIXED_INDEX);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[POLYGON_INDEX_BUFFER]);
        glDrawElements(GL_TRIANGLE_FAN, polygon_vertex_count, GL_UNSIGNED_INT, NULL);
    }

    glBindVertexArray(0);
}
