#ifndef BSP_H
#define BSP_H

#include "bspdefs.h"

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
    void render(QMatrix4x4 modelView, QMatrix4x4 projection);

    /**
     * @brief Returns the center of the level
     */
    QVector3D getCenter() { return center; }

private:
    /**
     * @brief Calculates the checksum of the block having the specified length
     */
    unsigned blockChecksum(const char *buffer, int length);

    /**
     * @brief Loads data from the file into the vectors
     */
    bool internalLoadMap(QFile &file);

    /**
     * @brief Parses the map data and frees unused resources
     */
    void parseMapData();

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

        file.seek(lump.fileofs);
        QByteArray data = file.read(lump.filelen);

        if (file.error() != QFile::NoError) {
            emit loadError(QString("Error loading BSP file: %1s").arg(file.errorString()));
            return false;
        }

        int count = lump.filelen / sizeof(T);

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

    /**
     * @brief Initializes the OpenGL functions
     */
    void initializeGL();

    /**
     * @brief Release all used shaders
     */
    void releaseShaders();

    std::vector<dshader_t> shaders;
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

    QOpenGLShaderProgram *shaderProgram;
    QOpenGLShader *vertexShader;
    QOpenGLShader *fragmentShader;

    /**
     * @brief Stores the rendered faces
     * This is used to optimize the rendering process. During the BSP rendering, a face might be rendered twice. Needed for VIS
     */
    std::vector<bool> drawnFaces;
    std::vector<QOpenGLTexture*> textures;
    QOpenGLVertexArrayObject *vertexInfo;
    QOpenGLBuffer *vboVertices;
    QOpenGLBuffer *vboIndexes;

    QVector3D center;
    unsigned checksum;

signals:
    void loadError(QString error);
};

#endif // BSP_H
