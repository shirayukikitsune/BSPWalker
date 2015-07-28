#include "bsp.h"

#include "bspdefs.h"

#include <algorithm>

#include <QCryptographicHash>
#include <QFile>

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
    if (!initializeOpenGLFunctions())
        qWarning() << "Unable to initialize OpenGL 4.0 Core profile" << endl;

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

    for (auto i = textures.begin(); i != textures.end(); ++i) {
        (*i)->release();
        (*i)->destroy();
        delete *i;
    }
    textures.clear();
    drawnFaces.clear();

    shaders.clear();
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

void BSP::render()
{
    // Resets the drawn faces bitset
    for(auto i = drawnFaces.begin(); i != drawnFaces.end(); ++i) {
       *i = false;
    }

    int i = (int)surfaces.size();

    vertexInfo->bind();
    shaderProgram->bind();
    vboVertices->bind();
    shaderProgram->enableAttributeArray("vPosition");
    shaderProgram->setAttributeBuffer("vPosition", GL_FLOAT, offsetof(drawVert_t, position), 4, sizeof(drawVert_t));
    shaderProgram->enableAttributeArray("vTexCoord");
    shaderProgram->setAttributeBuffer("vTexCoord", GL_FLOAT, offsetof(drawVert_t, texCoord), 2, sizeof(drawVert_t));
    shaderProgram->enableAttributeArray("vLightmapCoord");
    shaderProgram->setAttributeBuffer("vLightmapCoord", GL_FLOAT, offsetof(drawVert_t, lightmapCoord), 2, sizeof(drawVert_t));
    shaderProgram->enableAttributeArray("vNormal");
    shaderProgram->setAttributeBuffer("vNormal", GL_FLOAT, offsetof(drawVert_t, lightmapCoord), 4, sizeof(drawVert_t));
    shaderProgram->enableAttributeArray("vColor");
    shaderProgram->setAttributeBuffer("vColor", GL_FLOAT, offsetof(drawVert_t, color), 4, sizeof(drawVert_t));

    vboIndexes->bind();

    while (--i > 0) {
        // Check if this surface is a polygon (plane)
        if (surfaces[i].surfaceType != MST_PLANAR) continue;
        // Check if this surface was rendered
        if (drawnFaces[i] == true) continue;

        drawnFaces[i] = true;
        glDrawRangeElements(GL_TRIANGLES, surfaces[i].firstIndex, surfaces[i].firstIndex + surfaces[i].numIndexes, surfaces[i].numIndexes / 3, GL_UNSIGNED_INT, 0);
    }

    vboIndexes->release();
    vboVertices->release();
    shaderProgram->release();
    vertexInfo->release();
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
    if (!loadNotEmptyLump<dshader_t>(file, header->lumps[LUMP_SHADERS], shaders))
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
    if (!loadNotEmptyLump<dsurface_t>(file, header->lumps[LUMP_SURFACES], surfaces))
        return false;
    if (!loadNotEmptyLump<dvert_t>(file, header->lumps[LUMP_DRAWVERTS], vertexData))
        return false;
    if (!loadLump<int>(file, header->lumps[LUMP_DRAWINDEXES], indexes))
        return false;

    return true;
}

void BSP::parseMapData()
{
    drawnFaces.resize(surfaces.size());

    drawVert_t *vertices = new drawVert_t[vertexData.size()];
    int i = 0;
    // Convert from BSP dvert_t to a shader-friendly drawVert_t
    std::for_each(vertexData.begin(), vertexData.end(), [&vertices, &i](dvert_t &data) {
        vertices[i].position = QVector4D(data.xyz[0], data.xyz[2], -data.xyz[1], 1.0);
        vertices[i].texCoord = QVector2D(data.st[0], data.st[1]);
        vertices[i].lightmapCoord = QVector2D(data.lightmap[0], data.lightmap[1]);
        vertices[i].normal = QVector4D(data.normal, 0.0);
        vertices[i].color.setRgb(data.color[0], data.color[1], data.color[2], data.color[3]);
        ++i;
    });
    vertexData.clear();

    vertexInfo = new QOpenGLVertexArrayObject;
    vertexInfo->create();
    vertexInfo->bind();

    vboVertices = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboVertices->create();
    vboVertices->bind();
    vboVertices->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vboVertices->allocate(vertices, i * sizeof(drawVert_t));
    delete[] vertices;

    vboIndexes = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    vboIndexes->create();
    vboIndexes->bind();
    vboIndexes->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vboIndexes->allocate(indexes.data(), indexes.size() * sizeof(int));
    indexes.clear();

    vertexInfo->release();
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
