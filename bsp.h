#ifndef BSP_H
#define BSP_H

#include "bspdefs.h"

#include <vector>

#include <QObject>
#include <QString>
#include <QFile>

class BSP : public QObject
{
    Q_OBJECT

public:
    BSP();

    /**
     * @brief Loads a BSP map from the specified filename
     */
    void loadMap(const QString& file);

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
    std::vector<drawVert_t> vertices;

    unsigned checksum;

signals:
    void loadError(QString error);
};

#endif // BSP_H
