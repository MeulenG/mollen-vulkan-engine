#ifndef MVE_TERRAIN_MESH_H
#define MVE_TERRAIN_MESH_H

#include "mesh.h"
#include "../formats/adt_types.h"

#include <memory>

namespace mve {

// Builds a single Mesh from an entire ADT tile's heightmap. Each MCNK
// contributes 145 vertices and 256 triangles; one tile produces ~37k
// vertices and ~65k triangles, fitting comfortably in a single buffer.
//
// Coordinate convention: WoW Z-up is converted to engine Y-up at build
// time. The (wow_x, wow_y, wow_z) triple becomes engine (wow_y, wow_z,
// wow_x) so the X axis points east (WoW Y), Y points up (WoW Z), and Z
// points south (WoW X).
class TerrainMesh {
public:
    // Build a Mesh from `tile`. Uses the existing Vertex layout so it can
    // be drawn by the model pipeline (or a new terrain pipeline once we
    // wire one up).
    static std::unique_ptr<Mesh> Build(Device& device, const AdtTile& tile);
};

} // namespace mve

#endif // MVE_TERRAIN_MESH_H
