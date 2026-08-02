#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <filesystem>
#include <algorithm>

#include "mpq_archive.h"

namespace fs = std::filesystem;

static void PrintUsage(const char* program) {
    printf("Usage: %s <data_dir> [options]\n", program);
    printf("  data_dir         Path to WoW Data directory containing MPQ files\n");
    printf("  --output <dir>   Output directory (default: assets)\n");
    printf("  --creature <name> Extract a specific creature (e.g., Bear)\n");
    printf("  --path <prefix>  Extract all files matching a path prefix\n");
    printf("  --all-creatures  Extract all creature M2 models + textures\n");
    printf("  --locale <loc>   Locale subdirectory (default: enUS)\n");
    printf("  -v               Verbose output\n");
    printf("\nExamples:\n");
    printf("  %s \"C:\\WoW\\Data\" --creature Bear\n", program);
    printf("  %s \"C:\\WoW\\Data\" --path \"Creature/Bear\"\n", program);
    printf("  %s \"C:\\WoW\\Data\" --all-creatures --output assets\n", program);
}

static int MpqLoadOrder(const std::string& name) {
    if (name.find("common") == 0) return 0;
    if (name.find("expansion") == 0) return 1;
    if (name.find("lichking") == 0) return 2;
    if (name.find("locale-") == 0) return 3;
    if (name.find("lichking-locale") == 0) return 4;
    if (name.find("lichking-speech") == 0) return 5;
    if (name.find("speech-") == 0) return 5;
    if (name.find("wow-update") == 0) return 6;
    if (name.find("patch") == 0) return 7;
    return 99;
}

static std::vector<fs::path> CollectMpqs(const fs::path& data_dir, const std::string& locale) {
    std::vector<fs::path> mpqs;

    for (const auto& entry : fs::directory_iterator(data_dir)) {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == ".mpq") mpqs.push_back(entry.path());
    }

    fs::path locale_dir = data_dir / locale;
    if (fs::is_directory(locale_dir)) {
        for (const auto& entry : fs::directory_iterator(locale_dir)) {
            if (!entry.is_regular_file()) continue;
            auto ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".mpq") mpqs.push_back(entry.path());
        }
    }

    std::sort(mpqs.begin(), mpqs.end(), [](const fs::path& a, const fs::path& b) {
        std::string na = a.stem().string();
        std::string nb = b.stem().string();
        std::transform(na.begin(), na.end(), na.begin(), ::tolower);
        std::transform(nb.begin(), nb.end(), nb.begin(), ::tolower);
        int oa = MpqLoadOrder(na);
        int ob = MpqLoadOrder(nb);
        if (oa != ob) return oa < ob;
        return na < nb;
    });

    return mpqs;
}

static std::string ToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

static std::string NormalizePath(std::string path) {
    std::replace(path.begin(), path.end(), '\\', '/');
    return path;
}

static bool EndsWith(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) return false;
    return ToLower(str.substr(str.size() - suffix.size())) == ToLower(suffix);
}

static bool MatchesFilter(const std::string& file_path,
                           const std::string& creature_name,
                           const std::string& path_prefix,
                           bool all_creatures) {
    std::string lower = ToLower(NormalizePath(file_path));

    // File type filter: model, skin, BLP, plus terrain files (ADT/WDT/WMO).
    bool is_model = EndsWith(lower, ".m2") || EndsWith(lower, ".mdx");
    bool is_skin = EndsWith(lower, ".skin");
    bool is_blp = EndsWith(lower, ".blp");
    bool is_terrain = EndsWith(lower, ".adt") || EndsWith(lower, ".wdt") ||
                      EndsWith(lower, ".wmo");

    if (!is_model && !is_skin && !is_blp && !is_terrain) return false;

    // Specific creature filter
    if (!creature_name.empty()) {
        std::string creature_path = "creature/" + ToLower(creature_name);
        return lower.find(creature_path) != std::string::npos;
    }

    // Path prefix filter
    if (!path_prefix.empty()) {
        std::string prefix = ToLower(NormalizePath(path_prefix));
        return lower.find(prefix) != std::string::npos;
    }

    // All creatures
    if (all_creatures) {
        return lower.find("creature/") != std::string::npos;
    }

    return false;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 1;
    }

    const char* data_dir = nullptr;
    std::string output_dir = "assets";
    std::string creature_name;
    std::string path_prefix;
    std::string locale = "enUS";
    bool all_creatures = false;
    bool verbose = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            output_dir = argv[++i];
        } else if (strcmp(argv[i], "--creature") == 0 && i + 1 < argc) {
            creature_name = argv[++i];
        } else if (strcmp(argv[i], "--path") == 0 && i + 1 < argc) {
            path_prefix = argv[++i];
        } else if (strcmp(argv[i], "--all-creatures") == 0) {
            all_creatures = true;
        } else if (strcmp(argv[i], "--locale") == 0 && i + 1 < argc) {
            locale = argv[++i];
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
        } else if (!data_dir) {
            data_dir = argv[i];
        } else {
            printf("Unknown argument: %s\n", argv[i]);
            PrintUsage(argv[0]);
            return 1;
        }
    }

    if (!data_dir) {
        printf("Error: data_dir is required.\n");
        PrintUsage(argv[0]);
        return 1;
    }

    if (creature_name.empty() && path_prefix.empty() && !all_creatures) {
        printf("Error: specify --creature <name>, --path <prefix>, or --all-creatures\n");
        return 1;
    }

    // Collect and sort MPQ files
    auto mpqs = CollectMpqs(data_dir, locale);
    printf("Found %zu MPQ files.\n", mpqs.size());

    // Scan all MPQs for matching files. Later MPQs override earlier ones.
    std::map<std::string, fs::path> file_sources; // normalized_path -> mpq that has it

    for (const auto& mpq_path : mpqs) {
        MpqArchive mpq;
        if (!mpq.Open(mpq_path.string().c_str())) {
            printf("Warning: failed to open %s\n", mpq_path.string().c_str());
            continue;
        }

        if (verbose) {
            printf("Scanning: %s\n", mpq_path.filename().string().c_str());
        }

        auto files = mpq.GetListFile();
        for (const auto& f : files) {
            if (MatchesFilter(f, creature_name, path_prefix, all_creatures)) {
                file_sources[NormalizePath(f)] = mpq_path;
            }
        }

        mpq.Close();
    }

    printf("Found %zu matching files to extract.\n", file_sources.size());

    if (file_sources.empty()) {
        printf("No files matched the filter. Try a different creature name or path.\n");
        return 0;
    }

    // Group by MPQ to minimize re-opening archives
    std::map<std::string, std::vector<std::string>> mpq_to_files;
    for (const auto& [file_path, mpq_path] : file_sources) {
        mpq_to_files[mpq_path.string()].push_back(file_path);
    }

    // Extract files
    uint32_t extracted = 0;
    uint32_t failed = 0;

    for (const auto& [mpq_path, files] : mpq_to_files) {
        MpqArchive mpq;
        if (!mpq.Open(mpq_path.c_str())) {
            printf("Failed to open %s for extraction\n", mpq_path.c_str());
            failed += static_cast<uint32_t>(files.size());
            continue;
        }

        for (const auto& file_path : files) {
            // MPQ stores paths with backslashes
            std::string mpq_internal = file_path;
            std::replace(mpq_internal.begin(), mpq_internal.end(), '/', '\\');

            auto data = mpq.ExtractFile(mpq_internal.c_str());
            if (data.empty()) {
                if (verbose) printf("  Failed: %s\n", file_path.c_str());
                failed++;
                continue;
            }

            // Write to output directory
            fs::path out_path = fs::path(output_dir) / file_path;
            fs::create_directories(out_path.parent_path());

            FILE* out = fopen(out_path.string().c_str(), "wb");
            if (out) {
                fwrite(data.data(), 1, data.size(), out);
                fclose(out);
                extracted++;
                if (verbose) printf("  Extracted: %s (%zu bytes)\n", file_path.c_str(), data.size());
            } else {
                printf("  Failed to write: %s\n", out_path.string().c_str());
                failed++;
            }
        }

        mpq.Close();
    }

    printf("\nDone. Extracted: %u, Failed: %u\n", extracted, failed);
    printf("Output directory: %s\n", fs::absolute(output_dir).string().c_str());

    return 0;
}
