#include "bsp.h"

#include "bspdefs.h"
#include "q3parser.h"

#include <algorithm>
#include <iostream>

#include <QCryptographicHash>
#include <QFile>
#include <QOpenGLPixelTransferOptions>
#include <QRegularExpression>

BSP::BSP()
{
    vboIndexes = nullptr;
    vboVertices = nullptr;
    vertexInfo = nullptr;

    vertexShader = nullptr;
    fragmentShader = nullptr;
    shaderProgram = nullptr;

    visibilityData = nullptr;

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

    // Load shader data
    QRegularExpression re("(.*)(\\\\|/)(maps)(\\\\|/)(.+)(\\.bsp)", QRegularExpression::CaseInsensitiveOption);
    QString shaderFile = file;
    shaderFile.replace(re, "\\1\\2scripts\\4\\5.shader");

    parseShaderData(shaderFile);
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

    if (visibilityData) {
        delete[] visibilityData->bitset;
        delete visibilityData;
        visibilityData = nullptr;
    }
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

void BSP::render(QMatrix4x4 modelView, QMatrix4x4 projection, QVector3D cameraPosition)
{
    if (!vboIndexes)
        return;

    // Animate the shaders
    for(auto shader = shaders.begin(); shader != shaders.end(); ++shader)
        (*shader)->update();

    // Resets the drawn faces bitset
    for(auto i = drawnFaces.begin(); i != drawnFaces.end(); ++i) {
       *i = false;
    }

    int currentLeafIndex = findNodeForPosition(cameraPosition);
    int currentCluster = leafs[currentLeafIndex].cluster;
    int i = (int)leafs.size();

    shaderProgram->bind();
    shaderProgram->setUniformValue("modelView", modelView);
    shaderProgram->setUniformValue("normalMatrix", modelView.normalMatrix());
    shaderProgram->setUniformValue("projectionMatrix", projection);
    shaderProgram->setUniformValue("albedoTexture", 0);
    shaderProgram->setUniformValue("lightmapTexture", 1);
    shaderProgram->setUniformValue("lightDirection", skyLight.direction);
    shaderProgram->setUniformValue("lightColor", skyLight.color);
    shaderProgram->setUniformValue("lightIntensity", skyLight.intensity);

    vertexInfo->bind();
    vboIndexes->bind();

    while (i --> 0) {
        dleaf_t& drawLeaf = leafs[i];

        if (!canSee(currentCluster, drawLeaf.cluster))
            continue;

        int faceCount = drawLeaf.numLeafSurfaces;

        while (faceCount --> 0) {
            int surfaceIndex = leafSurfaces[drawLeaf.firstLeafSurface + faceCount];
            dsurface_t &surface = surfaces[surfaceIndex];

            // Check if this surface is a polygon (plane)
            if (surface.surfaceType != MST_PLANAR && surface.surfaceType != MST_PATCH) continue;
            // Check if this surface was rendered
            if (drawnFaces[surfaceIndex] == true) continue;

            drawnFaces[surfaceIndex] = true;

            shaders[surface.shaderNum]->bind(shaderProgram);

            if (surface.lightmapNum >= 0)
                lightmaps[surface.lightmapNum]->bind(1);

            // Since BSP indices are relative to the first vertex of the surface, we use glDrawElementsBaseVertex
            glDrawElementsBaseVertex(GL_TRIANGLES, surface.numIndexes, GL_UNSIGNED_INT, reinterpret_cast<void*>(surface.firstIndex * sizeof(GLuint)), surface.firstVert);

            if (surface.lightmapNum >= 0)
                lightmaps[surface.lightmapNum]->release(1);
            shaders[surface.shaderNum]->release();
        }
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
    if (!loadVisData(file, header->lumps[LUMP_VISIBILITY]))
        return false;

    return true;
}

void BSP::parseShaderData(QString fileName)
{
    QFile file(fileName);
    if (!file.exists() || !file.open(QFile::ReadOnly))
        return;

    QByteArray data = file.readAll();

    file.close();

    Q3Parser parser(data.data());
    Q3TokenType tokenType;

    int nestLevel = 0;
    BSPShader *currentShader;

    QString currentAlbedo = "";
    QVector2D uvMod;

    // Look for the sun color and direction
    while ((tokenType = parser.next()) != Q3TOK_EOF) {
        if (tokenType == Q3TOK_LITERAL || tokenType == Q3TOK_STRING) {
            if (nestLevel == 0) {
                // Root level, shader name
                QString shaderName = parser.getCurrentToken();
                currentShader = new BSPShader(shaderName);
                namedShaders[shaderName] = currentShader;
            }
            else {
                QString attributeName = parser.getCurrentToken();
                //
                if (attributeName == "q3map_sun") {
                    parser.next();
                    QString red = parser.getCurrentToken();
                    parser.next();
                    QString green = parser.getCurrentToken();
                    parser.next();
                    QString blue = parser.getCurrentToken();

                    skyLight.color = QVector3D(atof(red.toLatin1().data()), atof(green.toLatin1().data()), atof(blue.toLatin1().data()));

                    parser.next();
                    QString intensity = parser.getCurrentToken();
                    skyLight.intensity = atof(intensity.toLatin1().data());

                    parser.next();
                    QString zRotation = parser.getCurrentToken();
                    parser.next();
                    QString xRotation = parser.getCurrentToken();

                    // Calculate the direction
                    QVector3D direction(0, 1, 0);
                    QQuaternion rotation = QQuaternion::fromAxisAndAngle(0, 0, 1, atof(zRotation.toLatin1().data()));
                    rotation *= QQuaternion::fromAxisAndAngle(1, 0, 0, atof(xRotation.toLatin1().data()));
                    skyLight.direction = rotation.rotatedVector(direction);
                }
                else if (attributeName == "map") {
                    // Texture file
                    parser.next();
                    QString fileName = parser.getCurrentToken();
                    if (fileName[0] == '$'){
                        // Special name, ignore it
                        continue;
                    }
                    currentAlbedo = fileName;
                }
                else if (attributeName == "tcMod") {
                    // get mod type
                    parser.next();
                    QString modType = parser.getCurrentToken();
                    if (modType.toLower() == "scroll") {
                        // get mod values
                        parser.next();
                        QString xMod = parser.getCurrentToken();
                        parser.next();
                        QString yMod = parser.getCurrentToken();
                        uvMod = QVector2D(atof(xMod.toLatin1().data()), atof(yMod.toLatin1().data()));
                    }
                }
            }
        }
        else if (tokenType == Q3TOK_LIST_START) {
            currentAlbedo.clear();
            uvMod = QVector2D(0, 0);
            nestLevel++;
        }
        else if (tokenType == Q3TOK_LIST_END) {
            if (currentShader && !currentAlbedo.isNull()) {
                currentShader->setAlbedo(currentAlbedo);
                currentShader->setUVModValue(uvMod);
            }
            nestLevel--;
        }
    }
}

bool BSP::loadVisData(QFile &file, lump_t &lump)
{
    if (lump.filelen == 0)
        return true;

    file.seek(lump.fileofs);

    visibilityData = new dvisdata_t;

    file.read((char*)&visibilityData->clusterNum, sizeof (int));
    file.read((char*)&visibilityData->clusterSize, sizeof (int));

    int totalSize = visibilityData->clusterNum * visibilityData->clusterSize;

    visibilityData->bitset = new unsigned char[totalSize];
    file.read((char*)visibilityData->bitset, totalSize);

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
        auto named = namedShaders.find(shader->shader);
        if (named == namedShaders.end()) {
            BSPShader *bspShader = new BSPShader(shader->shader);
            bspShader->create();
            shaders.push_back(bspShader);
        }
        else {
            shaders.push_back(named->second);
        }
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

int BSP::findNodeForPosition(const QVector3D &position)
{
    int nodeIndex = 0;

    while (nodeIndex >= 0)
    {
        auto &node = nodes[nodeIndex];
        auto &plane = planes[node.planeNum];

        float distance = plane.normal[0] * position.x() + plane.normal[1] * position.y() + plane.normal[2] * position.z() - plane.dist;

        nodeIndex = node.children[distance > 0 ? 1 : 0];
    }

    return ~nodeIndex;
}
