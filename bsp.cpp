#include "bsp.h"

#include "bspdefs.h"
#include "q3parser.h"

#include <algorithm>
#include <iostream>

#include <QCryptographicHash>
#include <QFile>
#include <QOpenGLPixelTransferOptions>

BSP::BSP()
{
    vboIndexes = nullptr;
    vboVertices = nullptr;
    vertexInfo = nullptr;

    vertexShader = nullptr;
    fragmentShader = nullptr;
    shaderProgram = nullptr;

    initializeGL();
}

BSP::~BSP()
{
    releaseMap();
    releaseShaders();
}

void BSP::initializeGL()
{
    if (!initializeOpenGLFunctions()) {
        qWarning() << "Unable to initialize OpenGL 4.0 Core profile" << endl;
        return;
    }

    vertexShader = new QOpenGLShader(QOpenGLShader::Vertex);
    if (!vertexShader->compileSourceFile(":/shaders/bsp.vert"))
        qWarning() << vertexShader->log() << endl;

    fragmentShader = new QOpenGLShader(QOpenGLShader::Fragment);
    if (!fragmentShader->compileSourceFile(":/shaders/bsp.frag"))
        qWarning() << fragmentShader->log() << endl;

    shaderProgram = new QOpenGLShaderProgram;
    shaderProgram->addShader(vertexShader);
    shaderProgram->addShader(fragmentShader);

    if (!shaderProgram->link())
        qWarning() << shaderProgram->log() << endl;
}

void BSP::releaseShaders()
{
    delete vertexShader;
    vertexShader = nullptr;

    delete fragmentShader;
    fragmentShader = nullptr;

    if (shaderProgram) {
        shaderProgram->release();
        delete shaderProgram;
        shaderProgram = nullptr;
    }
}

void BSP::loadMap(const QString &file)
{
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly)) {
        emit loadError(QString("Failed to open file %1s").arg(file));
    }

    if (f.error() != QFile::NoError) {
        emit loadError(QString("Error loading BSP file %1s: %2s").arg(file, f.errorString()));
        return;
    }

    releaseMap();

    if (!internalLoadMap(f)) {
        releaseMap();
        return;
    }

    f.close();

    parseMapData();
}

void BSP::releaseMap()
{
    destroyGPUObjects();
    destroyLumpData();

    for (auto i = entities.begin(); i != entities.end(); ++i) {
        delete *i;
    }
    entities.clear();
}

void BSP::destroyGPUObjects()
{
    if (vboIndexes) {
        vboIndexes->release();
        vboIndexes->destroy();
        delete vboIndexes;
        vboIndexes = nullptr;
    }

    if (vboVertices) {
        vboVertices->release();
        vboVertices->destroy();
        delete vboVertices;
        vboVertices = nullptr;
    }

    if (vertexInfo) {
        vertexInfo->release();
        vertexInfo->destroy();
        delete vertexInfo;
        vertexInfo = nullptr;
    }

    for (auto i = shaders.begin(); i != shaders.end(); ++i) {
        delete *i;
    }
    shaders.clear();
    drawnFaces.clear();

    for (auto i = lightmaps.begin(); i != lightmaps.end(); ++i) {
        (*i)->release();
        (*i)->destroy();
        delete *i;
    }
    lightmaps.clear();
}

void BSP::destroyLumpData()
{
    lumpShaders.clear();
    leafs.clear();
    leafBrushes.clear();
    leafSurfaces.clear();
    planes.clear();
    brushSides.clear();
    brushes.clear();
    models.clear();
    nodes.clear();
    entityString.clear();
    surfaces.clear();
    vertexData.clear();
    indexes.clear();
}

void BSP::render(QMatrix4x4 modelView, QMatrix4x4 projection)
{
    if (!vboIndexes)
        return;

    // Resets the drawn faces bitset
    for(auto i = drawnFaces.begin(); i != drawnFaces.end(); ++i) {
       *i = false;
    }

    int i = (int)surfaces.size();

    shaderProgram->bind();
    shaderProgram->setUniformValue("modelView", modelView);
    shaderProgram->setUniformValue("normalMatrix", modelView.normalMatrix());
    shaderProgram->setUniformValue("projectionMatrix", projection);
    shaderProgram->setUniformValue("albedoTexture", 0);
    shaderProgram->setUniformValue("lightmapTexture", 1);

    vertexInfo->bind();
    vboIndexes->bind();

    while (i --> 0) {
        // Check if this surface is a polygon (plane)
        if (surfaces[i].surfaceType != MST_PLANAR && surfaces[i].surfaceType != MST_PATCH) continue;
        // Check if this surface was rendered
        if (drawnFaces[i] == true) continue;

        dsurface_t &surface = surfaces[i];
        drawnFaces[i] = true;

        shaders[surface.shaderNum]->bind();

        if (surface.lightmapNum >= 0)
            lightmaps[surface.lightmapNum]->bind(1);

        // Since BSP indices are relative to the first vertex of the surface, we use glDrawElementsBaseVertex
        glDrawElementsBaseVertex(GL_TRIANGLES, surface.numIndexes, GL_UNSIGNED_INT, reinterpret_cast<void*>(surface.firstIndex * sizeof(GLuint)), surface.firstVert);

        if (surface.lightmapNum >= 0)
            lightmaps[surface.lightmapNum]->release(1);
        shaders[surface.shaderNum]->release();
    }

    vboIndexes->release();
    vertexInfo->release();

    shaderProgram->release();
}

bool BSP::internalLoadMap(QFile &file)
{
    checksum = blockChecksum(file.readAll(), file.size());

    // readAll moves the pointer to the EOF, so reset it to the start
    file.seek(0);

    QByteArray buffer = file.read(sizeof (dheader_t));
    dheader_t *header = (dheader_t*)buffer.data();

    if (header->ident != BSP_IDENT) {
        emit loadError(QString("Unsupported BSP identifier"));
        return false;
    }

    if (header->version != BSP_VERSION) {
        emit loadError(QString("Unsupported BSP version. Expecting %1d, got %2d").arg(BSP_VERSION).arg(header->version));
        return false;
    }

    // Load all lumps
    if (!loadNotEmptyLump<dshader_t>(file, header->lumps[LUMP_SHADERS], lumpShaders))
        return false;
    if (!loadNotEmptyLump<dleaf_t>(file, header->lumps[LUMP_LEAFS], leafs))
        return false;
    if (!loadLump<int>(file, header->lumps[LUMP_LEAFBRUSHES], leafBrushes))
        return false;
    if (!loadLump<int>(file, header->lumps[LUMP_LEAFSURFACES], leafSurfaces))
        return false;
    if (!loadNotEmptyLump<dplane_t>(file, header->lumps[LUMP_PLANES], planes))
        return false;
    if (!loadLump<dbrushside_t>(file, header->lumps[LUMP_BRUSHSIDES], brushSides))
        return false;
    if (!loadLump<dbrush_t>(file, header->lumps[LUMP_BRUSHES], brushes))
        return false;
    if (!loadNotEmptyLump<dmodel_t>(file, header->lumps[LUMP_MODELS], models))
        return false;
    if (!loadNotEmptyLump<dnode_t>(file, header->lumps[LUMP_NODES], nodes))
        return false;
    if (!loadLump<char>(file, header->lumps[LUMP_ENTITIES], entityString))
        return false;
    if (!loadLump<dsurface_t>(file, header->lumps[LUMP_SURFACES], surfaces))
        return false;
    if (!loadLump<dvert_t>(file, header->lumps[LUMP_DRAWVERTS], vertexData))
        return false;
    if (!loadLump<int>(file, header->lumps[LUMP_DRAWINDEXES], indexes))
        return false;
    if (!loadNotEmptyLump<dlightmap_t>(file, header->lumps[LUMP_LIGHTMAPS], lightmapImages))
        return false;

    return true;
}

void BSP::parseMapData()
{
    parseShaders();

    createLightmaps();

	createVBOs();

    parseEntities();
}

void BSP::parseEntities()
{
    Q3TokenType tokenType;
    Q3Parser parser(entityString.data());

    BSPEntity *entity = nullptr;
    QString key;

    // Parsing entities is rather simple:
    // They have no name definitions and the contents are always a K/V pair
    while ((tokenType = parser.next()) != Q3TOK_EOF) {
        if (tokenType == Q3TOK_LIST_START) { // Starting a new group, so create a new entity
            entity = new BSPEntity;
        }
        else if (tokenType == Q3TOK_LIST_END) { // Ending a group, add the current entity to the entity list
            if (entity != nullptr)
                entities.push_back(entity);

            entity = nullptr;
        }
        else if (tokenType == Q3TOK_LITERAL || tokenType == Q3TOK_STRING) {
            if (key.isNull()) {
                key = parser.getCurrentToken();
            }
            else {
                entity->addSetting(key, parser.getCurrentToken());
                key.clear();
            }
        }
    }

    entityString.clear();
}

void BSP::createVBOs()
{
    int size = vertexData.size();
    drawVert_t *vertices = new drawVert_t[size];
    int i = 0;
    QVector3D center;
    // Convert from BSP dvert_t to a shader-friendly drawVert_t
    std::for_each(vertexData.begin(), vertexData.end(), [&vertices, &i, size, &center](dvert_t &data) {
        vertices[i].position = QVector3D(data.position[0], data.position[1], data.position[2]);
        vertices[i].texCoord = QVector2D(data.textureCoords[0], data.textureCoords[1]);
        vertices[i].lightmapCoord = QVector2D(data.lightmap[0], data.lightmap[1]);
        vertices[i].normal = QVector3D(data.normal[0], data.normal[1], data.normal[2]);
        vertices[i].color = QVector4D(data.color[0], data.color[1], data.color[2], data.color[3]) / 255;

        center = center + (vertices[i].position / size);
        ++i;
    });
    this->center = center;
    vertexData.clear();

    drawnFaces.resize(surfaces.size());

    vertexInfo = new QOpenGLVertexArrayObject;
    vertexInfo->create();
    vertexInfo->bind();

    vboVertices = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboVertices->create();
    vboVertices->bind();
    vboVertices->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vboVertices->allocate(vertices, size * sizeof(drawVert_t));
    delete[] vertices;

    shaderProgram->bind();

    vboVertices->bind();

    int attribute = shaderProgram->attributeLocation("vPosition");
    shaderProgram->enableAttributeArray(attribute);
    shaderProgram->setAttributeBuffer(attribute, GL_FLOAT, offsetof(drawVert_t, position), 3, sizeof(drawVert_t));

    attribute = shaderProgram->attributeLocation("vTexCoord");
    shaderProgram->enableAttributeArray(attribute);
    shaderProgram->setAttributeBuffer(attribute, GL_FLOAT, offsetof(drawVert_t, texCoord), 2, sizeof(drawVert_t));

    attribute = shaderProgram->attributeLocation("vLightmapCoord");
    shaderProgram->enableAttributeArray(attribute);
    shaderProgram->setAttributeBuffer(attribute, GL_FLOAT, offsetof(drawVert_t, lightmapCoord), 2, sizeof(drawVert_t));

    attribute = shaderProgram->attributeLocation("vNormal");
    shaderProgram->enableAttributeArray(attribute);
    shaderProgram->setAttributeBuffer(attribute, GL_FLOAT, offsetof(drawVert_t, normal), 3, sizeof(drawVert_t));

    attribute = shaderProgram->attributeLocation("vColor");
    shaderProgram->enableAttributeArray(attribute);
    shaderProgram->setAttributeBuffer(attribute, GL_FLOAT, offsetof(drawVert_t, color), 4, sizeof(drawVert_t));

    vertexInfo->release();

    vboIndexes = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    vboIndexes->create();
    vboIndexes->bind();
    vboIndexes->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vboIndexes->allocate(indexes.data(), indexes.size() * sizeof(int));
}

void BSP::parseShaders()
{
    for (auto shader = lumpShaders.begin(); shader != lumpShaders.end(); ++shader) {
        BSPShader *bspShader = new BSPShader;
        bspShader->createFromTextureFile(shader->shader);
        shaders.push_back(bspShader);
    }

    // Free the BSP raw data
    lumpShaders.clear();
}

void BSP::createLightmaps()
{
    // Set the buffer alignment to 1 byte
    QOpenGLPixelTransferOptions options;
    options.setAlignment(1);

    for (auto img = lightmapImages.begin(); img != lightmapImages.end(); ++img) {
        QOpenGLTexture *texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        texture->create();
        texture->setSize(LIGHTMAP_WIDTH, LIGHTMAP_HEIGHT);
        texture->setFormat(QOpenGLTexture::RGB8_UNorm);
        texture->allocateStorage();
        texture->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt8, img->data, &options);
		// Mipmaps are be created by default, so enable their use
        texture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
        lightmaps.push_back(texture);
    }

    // Free BSP raw data
    lightmapImages.clear();
}

unsigned BSP::blockChecksum(const char *buffer, int length)
{
    QCryptographicHash hash(QCryptographicHash::Md4);
    hash.addData(buffer, length);
    QByteArray result = hash.result();

    unsigned* digest = (unsigned*)result.data();
    unsigned val = digest[0] ^ digest[1] ^ digest[2] ^ digest[3];

    return val;
}

BSPEntity* BSP::findEntityByClassname(QString classname)
{
    for (auto entity = entities.begin(); entity != entities.end(); ++entity) {
        QString entityClass = (*entity)->getSetting("classname");
        if (!entityClass.isNull() && entityClass.compare(classname, Qt::CaseInsensitive) == 0) {
            return *entity;
        }
    }

    return nullptr;
}
