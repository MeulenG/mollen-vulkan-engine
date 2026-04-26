#ifndef MVE_DBC_BROWSER_SYSTEM_H
#define MVE_DBC_BROWSER_SYSTEM_H

#include "../resources/dbc_registry.h"

#include <string>

namespace mve {

// Read-only ImGui panel for browsing extracted .dbc files.
//
// Left pane: filterable list of DBCs the registry discovered on disk.
// Right pane: scrollable table view of the selected DBC's records, with
// columns derived from the registered schema.
class DbcBrowserSystem {
public:
    explicit DbcBrowserSystem(DbcRegistry& registry);

    void Update();

private:
    void DrawDbcList();
    void DrawRecordTable();

    DbcRegistry& pm_registry;
    std::string pm_selected;
    char pm_filter[64] = {0};
    int pm_max_rows = 1000;
};

} // namespace mve

#endif // MVE_DBC_BROWSER_SYSTEM_H
