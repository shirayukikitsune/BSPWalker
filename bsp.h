#ifndef BSP_H
#define BSP_H

#include "bspdefs.h"
#include "bspentity.h"
#include "bspshader.h"
#include "light.h"

#include <vector>

#include <QFile>
#include <QMatrix4x4>
#include <QObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_4_0_Core>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QString>

class BSP : public QObject, private QOpenGLFunctions_4_0_Core
{
    Q_OBJECT

public:
    BSP();
    ~BSP();

    /**
     * @brief Loads a BSP map from the specified filename
     */
    void loadMap(const QString& file);

    /**
     * @brief Unloads the BSP map and release all associated resources
     */
    void releaseMap();

    /**
     * @brief Renders the BSP level
     */
    void render(QMatrix4x4 modelView, QMatrix4x4 projection, QVector3D cameraPosition);

    /**
     * @brief Returns the center of the level
     */
    QVector3D getCenter() { return center; }

    BSPEntity *findEntityByClassname(QString classname);

private:
    /**
     * @brief Releases all allocated VBOs, VAOs and textures
     */
    void destroyGPUObjects();

    /**
     * @brief Releases all allocated data related to BSP
     */
    void destroyLumpData();

    /**
     * @brief Calculates the checksum of the block having the specified length
     */
    unsigned blockChecksum(const char *buffer, int length);

    /**
     * @brief Loads data from the file into the vectors
     */
    bool internalLoadMap(QFile &file);
    
    void parseShaderData(QString fileName);

    /**
     * @brief Parses the map data and frees unused resources
     */
    void parseMapData();

    /**
     * @brief Allocates all GPU resources for vertices and indexes
     */
    void createVBOs();

    /**
     * @brief Loads textures and associated resources
     */
    void parseShaders();

    /**
     * @brief Loads the entities information from the lump data
     */
    void parseEntities();

    /**
     * @brief Initializes all lightmap textures
     */
    void createLightmaps();

    /**
     * @brief Loads data from a lump and fills a vector
     */
    template <class T>
    bool loadLump(QFile &file, lump_t &lump, std::vector<T> &vec)
    {
        if (lump.filelen % sizeof(T) != 0) {
            emit loadError(QString("Invalid lump size, expected multiple of %1d, got %2d (remaining %3d bytes)").arg(sizeof(T)).arg(lump.filelen).arg(lump.filelen % sizeof(T)));
            return false;
        }

        int count = lump.filelen / sizeof(T);

        // If the lump is empty, exit silently
        if (count == 0)
            return true;

        file.seek(lump.fileofs);
        QByteArray data = file.read(lump.filelen);

        if (file.error() != QFile::NoError) {
            emit loadError(QString("Error loading BSP file: %1s").arg(file.errorString()));
            return false;
        }

        vec.resize(count);

        memcpy(vec.data(), data.data(), data.length());

        return true;
    }

    /**
     * @brief Loads data from a lump and fills a vector
     * @remarks Throws an error when the lump is empty
     */
    template <class T>
    bool loadNotEmptyLump(QFile &file, lump_t &lump, std::vector<T> &vec)
    {
        if (lump.filelen % sizeof(T) != 0) {
            emit loadError(QString("Invalid lump size, expected multiple of %1d, got %2d (remaining %3d bytes)").arg(sizeof(T)).arg(lump.filelen).arg(lump.filelen % sizeof(T)));
            return false;
        }

        file.seek(lump.fileofs);
        QByteArray data = file.read(lump.filelen);

        if (file.error() != QFile::NoError) {
            emit loadError(QString("Error loading BSP file: %1s").arg(file.errorString()));
            return false;
        }

        int count = lump.filelen / sizeof(T);

        if (count < 1) {
            emit loadError(QString("Empty lump"));
            return false;
        }

        vec.resize(count);

        memcpy(vec.data(), data.data(), data.length());

        return true;
    }

    bool loadVisData(QFile &file, lump_t &lump);

    /**
     * @brief Initializes the OpenGL functions
     */
    void initializeGL();

    /**
     * @brief Release all used shaders
     */
    void releaseShaders();

    /**
     * @brief Returns which node the specified position belongs to
     */
    int findNodeForPosition(const QVector3D& position);

    /**
     * @brief Returns if node ''current'' can see node ''test''
     */
    inline bool canSee(int current, int test) {
        if (!visibilityData || current < 0) return true;

        // Look for the byte containing the data
        unsigned char set = visibilityData->bitset[current * visibilityData->clusterSize + (test / 8)];

        // Returns whether the bit is set or not
        return !(set & (1 << (test & 7)));
    }

    std::vector<dshader_t> lumpShaders;
    std::vector<dleaf_t> leafs;
    std::vector<int> leafBrushes;
    std::vector<int> leafSurfaces;
    std::vector<dplane_t> planes;
    std::vector<dbrushside_t> brushSides;
    std::vector<dbrush_t> brushes;
    std::vector<dmodel_t> models;
    std::vector<dnode_t> nodes;
    std::vector<char> entityString;
    std::vector<dsurface_t> surfaces;
    std::vector<dvert_t> vertexData;
    std::vector<int> indexes;
    std::vector<dlightmap_t> lightmapImages;

    QOpenGLShaderProgram *shaderProgram;
    QOpenGLShader *vertexShader;
    QOpenGLShader *fragmentShader;

    /**
     * @brief Stores the rendered faces
     * This is used to optimize the rendering process. During the BSP rendering, a face might be rendered twice. Needed for VIS
     */
    std::vector<bool> drawnFaces;
    std::vector<BSPShader*> shaders;
    std::vector<QOpenGLTexture*> lightmaps;
    std::vector<BSPEntity*> entities;
    dvisdata_t *visibilityData;

    QOpenGLVertexArrayObject *vertexInfo;
    QOpenGLBuffer *vboVertices;
    QOpenGLBuffer *vboIndexes;

    std::map<QString, BSPShader*> namedShaders;

    Light skyLight;

    QVector3D center;
    unsigned checksum;

signals:
    void loadError(QString error);
};

#endif // BSP_H
