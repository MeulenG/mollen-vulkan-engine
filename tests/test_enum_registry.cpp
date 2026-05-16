#include <catch2/catch_test_macros.hpp>

#include "enum_registry.h"

#include <cstring>
#include <string>
#include <unordered_set>

TEST_CASE("Sex enum has Male and Female with the canonical values", "[enum]") {
    const DbcEnum* sex = GetDbcEnum("Sex");
    REQUIRE(sex != nullptr);
    REQUIRE(sex->count >= 2);

    bool found_male = false;
    bool found_female = false;
    for (uint32_t i = 0; i < sex->count; i++) {
        if (sex->values[i].value == 0 &&
            std::strcmp(sex->values[i].label, "Male") == 0) found_male = true;
        if (sex->values[i].value == 1 &&
            std::strcmp(sex->values[i].label, "Female") == 0) found_female = true;
    }
    CHECK(found_male);
    CHECK(found_female);
}

TEST_CASE("ItemQuality covers all 8 standard tiers", "[enum]") {
    const DbcEnum* q = GetDbcEnum("ItemQuality");
    REQUIRE(q != nullptr);
    REQUIRE(q->count == 8);
    // Values must be 0..7 in order - the editor relies on this for combo
    // index → value mapping.
    for (uint32_t i = 0; i < q->count; i++) {
        CHECK(q->values[i].value == static_cast<int32_t>(i));
        CHECK(q->values[i].label != nullptr);
        CHECK(q->values[i].label[0] != '\0');
    }
}

TEST_CASE("Unknown enum name returns null", "[enum]") {
    CHECK(GetDbcEnum("ThisEnumDoesNotExist") == nullptr);
    CHECK(GetDbcEnum(nullptr) == nullptr);
}

TEST_CASE("All registered enum names are unique", "[enum]") {
    uint32_t count = 0;
    const DbcEnum* const* all = GetAllDbcEnums(&count);
    REQUIRE(count > 0);

    std::unordered_set<std::string> seen;
    for (uint32_t i = 0; i < count; i++) {
        REQUIRE(all[i] != nullptr);
        REQUIRE(all[i]->name != nullptr);
        auto [_, inserted] = seen.insert(all[i]->name);
        CHECK(inserted);  // duplicates would silently shadow each other
    }
}

TEST_CASE("Every registered enum is non-empty and well-formed", "[enum]") {
    uint32_t count = 0;
    const DbcEnum* const* all = GetAllDbcEnums(&count);
    for (uint32_t i = 0; i < count; i++) {
        const DbcEnum* e = all[i];
        INFO("enum: " << e->name);
        CHECK(e->count > 0);
        CHECK(e->values != nullptr);
        for (uint32_t j = 0; j < e->count; j++) {
            INFO("value index: " << j);
            CHECK(e->values[j].label != nullptr);
            CHECK(e->values[j].label[0] != '\0');
        }
    }
}

TEST_CASE("PowerType supports negative values for Health", "[enum]") {
    const DbcEnum* pt = GetDbcEnum("PowerType");
    REQUIRE(pt != nullptr);
    bool found_health = false;
    for (uint32_t i = 0; i < pt->count; i++) {
        if (pt->values[i].value == -2 &&
            std::strcmp(pt->values[i].label, "Health") == 0) {
            found_health = true;
        }
    }
    CHECK(found_health);
}
