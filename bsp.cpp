#include "bsp.h"

#include "bspdefs.h"

#include <algorithm>

#include <QCryptographicHash>
#include <QFile>

BSP::BSP()
{

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

    internalLoadMap(file);

    f.close();

    parseMapData();
}

void BSP::internalLoadMap(QFile &file)
{
    checksum = blockChecksum(f.readAll(), f.size());

    // readAll moves the pointer to the EOF, so reset it to the start
    f.seek(0);

    QByteArray buffer = f.read(sizeof (dheader_t));
    dheader_t *header = (dheader_t*)buffer.data();

    if (header->ident != BSP_IDENT) {
        emit loadError(QString("Unsupported BSP identifier"));
        return;
    }

    if (header->version != BSP_VERSION) {
        emit loadError(QString("Unsupported BSP version. Expecting %1d, got %2d").arg(BSP_VERSION).arg(header->version));
        return;
    }

    // Load all lumps
    if (!loadNotEmptyLump<dshader_t>(f, header->lumps[LUMP_SHADERS], shaders))
        return;
    if (!loadNotEmptyLump<dleaf_t>(f, header->lumps[LUMP_LEAFS], leafs))
        return;
    if (!loadLump<int>(f, header->lumps[LUMP_LEAFBRUSHES], leafBrushes))
        return;
    if (!loadLump<int>(f, header->lumps[LUMP_LEAFSURFACES], leafSurfaces))
        return;
    if (!loadNotEmptyLump<dplane_t>(f, header->lumps[LUMP_PLANES], planes))
        return;
    if (!loadLump<dbrushside_t>(f, header->lumps[LUMP_BRUSHSIDES], brushSides))
        return;
    if (!loadLump<dbrush_t>(f, header->lumps[LUMP_BRUSHES], brushes))
        return;
    if (!loadNotEmptyLump<dmodel_t>(f, header->lumps[LUMP_MODELS], models))
        return;
    if (!loadNotEmptyLump<dnode_t>(f, header->lumps[LUMP_NODES], nodes))
        return;
    if (!loadLump<char>(f, header->lumps[LUMP_ENTITIES], entityString))
        return;
    if (!loadNotEmptyLump<dsurface_t>(f, header->lumps[LUMP_SURFACES], surfaces))
        return;
    if (!loadNotEmptyLump<drawVert_t>(f, header->lumps[LUMP_DRAWVERTS], vertices))
        return;
}

void BSP::parseMapData()
{

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
