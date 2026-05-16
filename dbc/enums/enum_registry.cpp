#include "enum_registry.h"

#include <cstring>

// One include per registered enum - adding a new enum is a two-line change:
// add the include here and append the table to the array below.
#include "enum_sex.h"
#include "enum_item_quality.h"
#include "enum_item_class.h"
#include "enum_inventory_type.h"
#include "enum_material.h"
#include "enum_sheath.h"
#include "enum_bonding.h"
#include "enum_spell_school.h"
#include "enum_power_type.h"
#include "enum_creature_type.h"
#include "enum_blood_level.h"
#include "enum_dispel_type.h"
#include "enum_race_mask.h"
#include "enum_class_mask.h"
#include "enum_spell_school_mask.h"
#include "enum_item_flags.h"

// Extracted from TrinityCore 3.3.5 by tools/extract_tc_enums.py.
#include "enum_achievement_flags.h"
#include "enum_achievement_criteria_flags.h"
#include "enum_area_flags.h"
#include "enum_map_flags.h"
#include "enum_faction_template_flags.h"
#include "enum_vehicle_seat_flags.h"
#include "enum_vehicle_seat_flags_b.h"
#include "enum_skill_race_class_info_flags.h"
#include "enum_spell_attr0.h"
#include "enum_spell_attr1.h"
#include "enum_spell_attr2.h"
#include "enum_spell_attr3.h"
#include "enum_spell_attr4.h"
#include "enum_spell_attr5.h"
#include "enum_spell_attr6.h"
#include "enum_spell_attr7.h"
#include "enum_creature_type_flags.h"
#include "enum_character_flags.h"
#include "enum_unit_dyn_flags.h"
#include "enum_dungeon_status_flag.h"
#include "enum_race.h"
#include "enum_class.h"
#include "enum_language.h"
#include "enum_spell_family.h"
#include "enum_skill_category.h"
#include "enum_totem_category.h"
#include "enum_creature_elite_type.h"
#include "enum_spell_effects.h"
#include "enum_spell_aura_type.h"

namespace {

const DbcEnum* const k_all_enums[] = {
    &sex_enum,
    &item_quality_enum,
    &item_class_enum,
    &inventory_type_enum,
    &material_enum,
    &sheath_enum,
    &bonding_enum,
    &spell_school_enum,
    &power_type_enum,
    &creature_type_enum,
    &blood_level_enum,
    &dispel_type_enum,
    &race_mask_enum,
    &class_mask_enum,
    &spell_school_mask_enum,
    &item_flags_enum,
    // From TrinityCore 3.3.5 extraction:
    &achievement_flags_enum,
    &achievement_criteria_flags_enum,
    &area_flags_enum,
    &map_flags_enum,
    &faction_template_flags_enum,
    &vehicle_seat_flags_enum,
    &vehicle_seat_flags_b_enum,
    &skill_race_class_info_flags_enum,
    &spell_attr0_enum,
    &spell_attr1_enum,
    &spell_attr2_enum,
    &spell_attr3_enum,
    &spell_attr4_enum,
    &spell_attr5_enum,
    &spell_attr6_enum,
    &spell_attr7_enum,
    &creature_type_flags_enum,
    &character_flags_enum,
    &unit_dyn_flags_enum,
    &dungeon_status_flag_enum,
    &race_enum,
    &class_enum,
    &language_enum,
    &spell_family_enum,
    &skill_category_enum,
    &totem_category_enum,
    &creature_elite_type_enum,
    &spell_effects_enum,
    &spell_aura_type_enum,
};

constexpr uint32_t k_enum_count = sizeof(k_all_enums) / sizeof(k_all_enums[0]);

} // namespace

const DbcEnum* GetDbcEnum(const char* name) {
    if (!name) return nullptr;
    // Linear scan is fine - fewer than ~50 enums total expected, and lookups
    // happen at UI render time per cell, not in a hot loop.
    for (uint32_t i = 0; i < k_enum_count; i++) {
        if (std::strcmp(k_all_enums[i]->name, name) == 0) {
            return k_all_enums[i];
        }
    }
    return nullptr;
}

const DbcEnum* const* GetAllDbcEnums(uint32_t* count) {
    if (count) *count = k_enum_count;
    return k_all_enums;
}
