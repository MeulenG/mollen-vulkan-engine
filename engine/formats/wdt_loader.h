#ifndef MVE_WDT_LOADER_H
#define MVE_WDT_LOADER_H

#include <cstdint>
#include <string>
#include <vector>

namespace mve {

// World Definition Table - the master index for a continent map. For our
// R1 needs the only useful piece is the 64x64 grid in the MAIN chunk that
// says which ADT tiles actually exist.
struct WdtTile {
    int x = 0;  // 0..63
    int y = 0;  // 0..63
};

class WdtLoader {
public:
    // Parses a .wdt file from disk. Returns true on success. On success
    // `out_existing_tiles` is populated with the (x, y) coordinates of
    // every tile that the MAIN chunk marks as present.
    static bool LoadFile(const std::string& path,
                         std::vector<WdtTile>& out_existing_tiles);
};

} // namespace mve

#endif // MVE_WDT_LOADER_H
