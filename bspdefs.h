#ifndef BSPDEFS_H
#define BSPDEFS_H

#include <QColor>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

// Extracted from https://github.com/darklegion/tremulous/blob/1ba4af154e471e3e00890bf034fc8dde410d951e/src/qcommon/qfiles.h

// Force alignment of 1 byte on structs
#pragma pack(push, 1)

#define MAX_QPATH 64

#define BSP_IDENT (('P'<<24)+('S'<<16)+('B'<<8)+'I')

#define BSP_VERSION			46

#define	MAX_MAP_MODELS		0x400
#define	MAX_MAP_BRUSHES		0x8000
#define	MAX_MAP_ENTITIES	0x800
#define	MAX_MAP_ENTSTRING	0x40000
#define	MAX_MAP_SHADERS		0x400

#define	MAX_MAP_AREAS		0x100
#define	MAX_MAP_FOGS		0x100
#define	MAX_MAP_PLANES		0x20000
#define	MAX_MAP_NODES		0x20000
#define	MAX_MAP_BRUSHSIDES	0x20000
#define	MAX_MAP_LEAFS		0x20000
#define	MAX_MAP_LEAFFACES	0x20000
#define	MAX_MAP_LEAFBRUSHES 0x40000
#define	MAX_MAP_PORTALS		0x20000
#define	MAX_MAP_LIGHTING	0x800000
#define	MAX_MAP_LIGHTGRID	0x800000
#define	MAX_MAP_VISIBILITY	0x200000

#define	MAX_MAP_DRAW_SURFS	0x20000
#define	MAX_MAP_DRAW_VERTS	0x80000
#define	MAX_MAP_DRAW_INDEXES	0x80000


// key / value pair sizes in the entities lump
#define	MAX_KEY				32
#define	MAX_VALUE			1024

// the editor uses these predefined yaw angles to orient entities up or down
#define	ANGLE_UP			-1
#define	ANGLE_DOWN			-2

#define	LIGHTMAP_WIDTH		128
#define	LIGHTMAP_HEIGHT		128

#define MAX_WORLD_COORD		( 128*1024 )
#define MIN_WORLD_COORD		( -128*1024 )
#define WORLD_SIZE			( MAX_WORLD_COORD - MIN_WORLD_COORD )

//=============================================================================


typedef struct {
    int		fileofs, filelen;
} lump_t;

#define	LUMP_ENTITIES		0
#define	LUMP_SHADERS		1
#define	LUMP_PLANES			2
#define	LUMP_NODES			3
#define	LUMP_LEAFS			4
#define	LUMP_LEAFSURFACES	5
#define	LUMP_LEAFBRUSHES	6
#define	LUMP_MODELS			7
#define	LUMP_BRUSHES		8
#define	LUMP_BRUSHSIDES		9
#define	LUMP_DRAWVERTS		10
#define	LUMP_DRAWINDEXES	11
#define	LUMP_FOGS			12
#define	LUMP_SURFACES		13
#define	LUMP_LIGHTMAPS		14
#define	LUMP_LIGHTGRID		15
#define	LUMP_VISIBILITY		16
#define	HEADER_LUMPS		17

typedef struct {
    int			ident;
    int			version;

    lump_t		lumps[HEADER_LUMPS];
} dheader_t;

typedef struct {
    float		mins[3], maxs[3];
    int			firstSurface, numSurfaces;
    int			firstBrush, numBrushes;
} dmodel_t;

typedef struct {
    char		shader[MAX_QPATH];
    int			surfaceFlags;
    int			contentFlags;
} dshader_t;

// planes x^1 is allways the opposite of plane x

typedef struct {
    float		normal[3];
    float		dist;
} dplane_t;

typedef struct {
    int			planeNum;
    int			children[2];	// negative numbers are -(leafs+1), not nodes
    int			mins[3];		// for frustom culling
    int			maxs[3];
} dnode_t;

typedef struct {
    int			cluster;			// -1 = opaque cluster (do I still store these?)
    int			area;

    int			mins[3];			// for frustum culling
    int			maxs[3];

    int			firstLeafSurface;
    int			numLeafSurfaces;

    int			firstLeafBrush;
    int			numLeafBrushes;
} dleaf_t;

typedef struct {
    int			planeNum;			// positive plane side faces out of the leaf
    int			shaderNum;
} dbrushside_t;

typedef struct {
    int			firstSide;
    int			numSides;
    int			shaderNum;		// the shader that determines the contents flags
} dbrush_t;

typedef struct {
    char		shader[MAX_QPATH];
    int			brushNum;
    int			visibleSide;	// the brush side that ray tests need to clip against (-1 == none)
} dfog_t;

typedef struct {
    float           position[3];
    float           textureCoords[2];
    float           lightmap[2];
    float           normal[3];
    unsigned char   color[4];
} dvert_t;

typedef struct {
    QVector3D   position;
    QVector2D   texCoord;
    QVector2D   lightmapCoord;
    QVector3D   normal;
    QVector4D   color;
} drawVert_t;

typedef struct {
    unsigned char data[LIGHTMAP_WIDTH][LIGHTMAP_HEIGHT][3];
} dlightmap_t;

#define drawVert_t_cleared(x) drawVert_t (x) = {{0, 0, 0}, {0, 0}, {0, 0}, {0, 0, 0}, {0, 0, 0, 0}}

typedef enum {
    MST_BAD,
    MST_PLANAR,
    MST_PATCH,
    MST_TRIANGLE_SOUP,
    MST_FLARE
} mapSurfaceType_t;

typedef struct {
    int			shaderNum;
    int			fogNum;
    int			surfaceType;

    int			firstVert;
    int			numVerts;

    int			firstIndex;
    int			numIndexes;

    int			lightmapNum;
    int			lightmapX, lightmapY;
    int			lightmapWidth, lightmapHeight;

    float   	lightmapOrigin[3];
    float       lightmapVecs[2][3];	// for patches, [0] and [1] are lodbounds
    float       normal[3];

    int			patchWidth;
    int			patchHeight;
} dsurface_t;

typedef struct {
    int             clusterNum;
    int             clusterSize;
    unsigned char*  bitset;
} dvisdata_t;

// Disable alignment options
#pragma pack(pop)

#endif // BSPDEFS_H

