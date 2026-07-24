#ifndef MVE_ICONS_FORK_AWESOME_H
#define MVE_ICONS_FORK_AWESOME_H

// Fork Awesome icon font codepoint range.
// Used to merge an icon font into the main ImGui atlas; see editor_style.cpp.
#define ICON_MIN_FA 0xF000
#define ICON_MAX_FA 0xF2E0

// Subset of Fork Awesome 1.x codepoints. Extend as needed.
// Cheatsheet: https://forkawesome.io/cheatsheet/
#define ICON_FA_BOOK          "\xEF\x80\xAD" // U+F02D
#define ICON_FA_COG           "\xEF\x80\x93" // U+F013  (also: GEAR)
#define ICON_FA_GEAR          ICON_FA_COG
#define ICON_FA_BOLT          "\xEF\x83\xA7" // U+F0E7
#define ICON_FA_MAGIC         "\xEF\x83\x90" // U+F0D0
#define ICON_FA_SHIELD        "\xEF\x84\xB2" // U+F132
#define ICON_FA_FIGHTER_JET   "\xEF\x83\xBB" // U+F0FB
#define ICON_FA_SWORD         ICON_FA_FIGHTER_JET   // Fork Awesome has no sword glyph
#define ICON_FA_EYE           "\xEF\x81\xAE" // U+F06E
#define ICON_FA_PENCIL        "\xEF\x81\x80" // U+F040  (also: EDIT)
#define ICON_FA_EDIT          ICON_FA_PENCIL
#define ICON_FA_FLOPPY_O      "\xEF\x83\x87" // U+F0C7  (also: SAVE)
#define ICON_FA_SAVE          ICON_FA_FLOPPY_O
#define ICON_FA_REFRESH       "\xEF\x80\xA1" // U+F021
#define ICON_FA_SEARCH        "\xEF\x80\x82" // U+F002
#define ICON_FA_PLUS          "\xEF\x81\xA7" // U+F067
#define ICON_FA_MINUS         "\xEF\x81\xA8" // U+F068
#define ICON_FA_CHECK         "\xEF\x80\x8C" // U+F00C
#define ICON_FA_TIMES         "\xEF\x80\x8D" // U+F00D
#define ICON_FA_INFO_CIRCLE   "\xEF\x81\x9A" // U+F05A
#define ICON_FA_EXCLAMATION_TRIANGLE "\xEF\x81\xB1" // U+F071
#define ICON_FA_WARNING       ICON_FA_EXCLAMATION_TRIANGLE

#endif // MVE_ICONS_FORK_AWESOME_H
