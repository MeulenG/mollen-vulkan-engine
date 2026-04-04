#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <algorithm>

#include "mpq_archive.h"
#include "dbc_file.h"
#include "dbc_writer.h"
#include "schema_registry.h"
#include "dbc_generator.h"
#include "db_config.h"
#include "psql_connector.h"
#include "dbc_db_import.h"

namespace fs = std::filesystem;

static void PrintUsage(const char* program) {
    printf("Usage: %s <data_dir> [options] [dbc_name]\n", program);
    printf("  data_dir     Path to WoW Data directory containing MPQ files\n");
    printf("  --expansion  Target expansion (default: wotlk)\n");
    printf("  --locale     Locale subdirectory to include (default: enUS)\n");
    printf("  --generate   Generate C++ headers to output directory\n");
    printf("  --export     Export raw .dbc files to output directory\n");
    printf("  --database   Import DBC data into PostgreSQL (path to .toml config)\n");
    printf("  -v           Verbose output (per-file details)\n");
    printf("  dbc_name     Optional: extract only this DBC (e.g. Spell)\n");
}

static std::string GetDbcName(const std::string& path) {
    size_t slash = path.find_last_of("\\/");
    std::string filename = path;
    if (slash != std::string::npos) {
        filename = path.substr(slash + 1);
    }

    size_t dot = filename.find_last_of('.');
    if (dot != std::string::npos) {
        filename = filename.substr(0, dot);
    }
    return filename;
}

static bool IsPadding(const char* name) {
    return name && name[0] == '_' && strncmp(name, "_pad", 4) == 0;
}

static void PrintFieldValue(const DbcFile& dbc, const DbcSchema* schema,
                             uint32_t record, uint32_t field) {
    const char* name = schema->fields[field].name;
    if (schema->packed) {
        uint32_t offset = DbcFile::GetFieldOffset(schema, field);
        switch (schema->fields[field].type) {
        case DbcFieldType::UInt8:
            printf(", %s=%u", name, dbc.GetUInt8At(record, offset));
            break;
        case DbcFieldType::Int8:
            printf(", %s=%d", name, dbc.GetInt8At(record, offset));
            break;
        case DbcFieldType::UInt16:
            printf(", %s=%u", name, dbc.GetUInt16At(record, offset));
            break;
        case DbcFieldType::Int16:
            printf(", %s=%d", name, dbc.GetInt16At(record, offset));
            break;
        case DbcFieldType::UInt32:
            printf(", %s=%u", name, dbc.GetUInt32At(record, offset));
            break;
        case DbcFieldType::Int32:
            printf(", %s=%d", name, dbc.GetInt32At(record, offset));
            break;
        case DbcFieldType::Float:
            printf(", %s=%.2f", name, dbc.GetFloatAt(record, offset));
            break;
        case DbcFieldType::String:
            printf(", %s=\"%s\"", name, dbc.GetStringAt(record, offset));
            break;
        }
    } else {
        switch (schema->fields[field].type) {
        case DbcFieldType::String:
            printf(", %s=\"%s\"", name, dbc.GetStringField(record, field));
            break;
        case DbcFieldType::Float:
            printf(", %s=%.2f", name, dbc.GetFloat(record, field));
            break;
        case DbcFieldType::Int32:
            printf(", %s=%d", name, dbc.GetInt32(record, field));
            break;
        default:
            printf(", %s=%u", name, dbc.GetUInt32(record, field));
            break;
        }
    }
}

static void DumpDbcDetailed(const DbcFile& dbc, const DbcSchema* schema) {
    printf("  Records: %u, Fields: %u, Record Size: %u bytes\n",
           dbc.GetRecordCount(), dbc.GetFieldCount(), dbc.GetRecordSize());

    if (!schema) {
        printf("  (no schema registered)\n");
        return;
    }

    printf("  Schema: %s%s\n", schema->dbc_name, schema->packed ? " (packed)" : "");

    if (schema->packed) {
        uint32_t expected = GetSchemaRecordSize(schema);
        if (expected != dbc.GetRecordSize()) {
            printf("  WARNING: schema record size %u, DBC record size %u\n",
                   expected, dbc.GetRecordSize());
            return;
        }
    } else {
        if (schema->field_count != dbc.GetFieldCount()) {
            printf("  WARNING: schema has %u fields, DBC has %u fields\n",
                   schema->field_count, dbc.GetFieldCount());
            return;
        }
    }

    uint32_t preview_count = 3;
    if (dbc.GetRecordCount() < preview_count) {
        preview_count = dbc.GetRecordCount();
    }

    for (uint32_t r = 0; r < preview_count; r++) {
        if (schema->packed) {
            uint32_t offset = DbcFile::GetFieldOffset(schema, 0);
            switch (schema->fields[0].type) {
            case DbcFieldType::UInt8:
                printf("  Row %u: %s=%u", r, schema->fields[0].name, dbc.GetUInt8At(r, offset));
                break;
            default:
                printf("  Row %u: %s=%u", r, schema->fields[0].name, dbc.GetUInt32At(r, offset));
                break;
            }
        } else {
            printf("  Row %u: ID=%u", r, dbc.GetUInt32(r, 0));
        }

        uint32_t shown = 0;
        for (uint32_t f = 1; f < schema->field_count && shown < 4; f++) {
            if (IsPadding(schema->fields[f].name)) {
                continue;
            }
            PrintFieldValue(dbc, schema, r, f);
            shown++;
        }
        printf("\n");
    }

    if (dbc.GetRecordCount() > preview_count) {
        printf("  ... (%u more rows)\n", dbc.GetRecordCount() - preview_count);
    }
}

static Expansion ParseExpansion(const char* str) {
    if (strcmp(str, "vanilla") == 0) return Expansion::Vanilla;
    if (strcmp(str, "tbc") == 0) return Expansion::TBC;
    if (strcmp(str, "wotlk") == 0) return Expansion::WotLK;
    return Expansion::WotLK;
}

static const char* ExpansionName(Expansion exp) {
    switch (exp) {
    case Expansion::Vanilla: return "vanilla";
    case Expansion::TBC: return "tbc";
    case Expansion::WotLK: return "wotlk";
    }
    return "unknown";
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
        if (ext == ".mpq") {
            mpqs.push_back(entry.path());
        }
    }

    fs::path locale_dir = data_dir / locale;
    if (fs::is_directory(locale_dir)) {
        for (const auto& entry : fs::directory_iterator(locale_dir)) {
            if (!entry.is_regular_file()) continue;
            auto ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".mpq") {
                mpqs.push_back(entry.path());
            }
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

struct DbcResult {
    std::string name;
    uint32_t records;
    uint32_t fields;
    uint32_t record_size;
    bool has_schema;
    bool packed;
    bool generated;
    bool exported;
    bool imported;
    std::string warning;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 1;
    }

    const char* data_dir = nullptr;
    const char* target_dbc = nullptr;
    const char* locale = "enUS";
    const char* generate_dir = nullptr;
    const char* export_dir = nullptr;
    const char* db_config_path = nullptr;
    bool verbose = false;
    Expansion expansion = Expansion::WotLK;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--expansion") == 0) {
            if (i + 1 < argc) {
                expansion = ParseExpansion(argv[++i]);
            } else {
                printf("Error: --expansion requires a value (vanilla, tbc, wotlk)\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--locale") == 0) {
            if (i + 1 < argc) {
                locale = argv[++i];
            } else {
                printf("Error: --locale requires a value (e.g. enUS)\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--generate") == 0) {
            if (i + 1 < argc) {
                generate_dir = argv[++i];
            } else {
                printf("Error: --generate requires an output directory\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--export") == 0) {
            if (i + 1 < argc) {
                export_dir = argv[++i];
            } else {
                printf("Error: --export requires an output directory\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--database") == 0) {
            if (i + 1 < argc) {
                db_config_path = argv[++i];
            } else {
                printf("Error: --database requires a config file path\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = true;
        } else if (!data_dir) {
            data_dir = argv[i];
        } else {
            target_dbc = argv[i];
        }
    }

    if (!data_dir) {
        PrintUsage(argv[0]);
        return 1;
    }

    if (!fs::is_directory(data_dir)) {
        printf("Not a directory: %s\n", data_dir);
        return 1;
    }

    printf(R"(
               .__  .__                                                             .______.
  _____   ____ |  | |  |   ____   ____           __  _  ________  _  __           __| _/\_ |__   ____
 /     \ /  _ \|  | |  | _/ __ \ /    \   ______ \ \/ \/ /  _ \ \/ \/ /  ______  / __ |  | __ \_/ ___\
|  Y Y  (  <_> )  |_|  |_\  ___/|   |  \ /_____/  \     (  <_> )     /  /_____/ / /_/ |  | \_\ \  \___
|__|_|  /\____/|____/____/\___  >___|  /           \/\_/ \____/ \/\_/           \____ |  |___  /\___  >
      \/                      \/     \/                                              \/      \/     \/
)");

    printf("Data directory: %s (expansion: %s, locale: %s)\n",
           data_dir, ExpansionName(expansion), locale);

    std::vector<fs::path> mpq_paths = CollectMpqs(data_dir, locale);
    if (mpq_paths.empty()) {
        printf("No MPQ files found.\n");
        return 1;
    }

    if (verbose) {
        printf("Found %zu MPQ files:\n", mpq_paths.size());
        for (const auto& p : mpq_paths) {
            printf("  %s\n", p.filename().string().c_str());
        }
        printf("\n");
    } else {
        printf("Loaded %zu MPQ files\n", mpq_paths.size());
    }

    std::map<std::string, std::vector<uint8_t>> dbc_data;

    for (const auto& mpq_path : mpq_paths) {
        MpqArchive mpq;
        if (!mpq.Open(mpq_path.string().c_str())) {
            if (verbose) {
                printf("Failed to open: %s\n", mpq_path.filename().string().c_str());
            }
            continue;
        }

        std::vector<std::string> files = mpq.GetListFile();
        for (const std::string& f : files) {
            if (f.size() <= 4) continue;
            std::string ext = f.substr(f.size() - 4);
            if (ext != ".dbc" && ext != ".DBC") continue;

            std::string name = GetDbcName(f);
            if (target_dbc && name != target_dbc) continue;

            std::vector<uint8_t> raw = mpq.ExtractFile(f.c_str());
            if (!raw.empty()) {
                dbc_data[name] = std::move(raw);
            }
        }

        mpq.Close();
    }

    printf("Resolved %zu DBC files.\n", dbc_data.size());

    // Connect to database if requested
    psql_connector db;
    if (db_config_path) {
        DbConfig cfg;
        if (!LoadDbConfig(db_config_path, cfg)) {
            printf("Failed to load database config: %s\n", db_config_path);
            return 1;
        }
        if (!db.Connect(cfg)) {
            printf("Failed to connect to database.\n");
            return 1;
        }
        printf("Connected to database: %s\n", cfg.dbname.c_str());
    }

    bool single_target = (target_dbc != nullptr);

    if (single_target) {
        for (const auto& [name, raw] : dbc_data) {
            printf("\n[%s]\n", name.c_str());
            DbcFile dbc;
            if (!dbc.Load(raw.data(), (uint32_t)raw.size())) {
                printf("  Failed to parse DBC header.\n");
                continue;
            }
            const DbcSchema* schema = GetSchema(expansion, name.c_str());

            if (export_dir) {
                if (schema) {
                    DbcWriter writer;
                    if (writer.ImportFrom(dbc, schema)) {
                        fs::path out_path = fs::path(export_dir) / (name + ".dbc");
                        fs::create_directories(export_dir);
                        if (writer.Save(out_path)) {
                            printf("  Exported %s (%u records)\n",
                                   out_path.string().c_str(), writer.GetRecordCount());
                        } else {
                            printf("  Failed to save %s\n", out_path.string().c_str());
                        }
                    } else {
                        printf("  Failed to import (record size mismatch)\n");
                    }
                } else {
                    fs::path out_path = fs::path(export_dir) / (name + ".dbc");
                    fs::create_directories(export_dir);
                    FILE* fout = fopen(out_path.string().c_str(), "wb");
                    if (fout) {
                        fwrite(raw.data(), 1, raw.size(), fout);
                        fclose(fout);
                        printf("  Exported raw %s (%zu bytes)\n",
                               out_path.string().c_str(), raw.size());
                    }
                }
            } else if (generate_dir) {
                GenerateHeader(dbc, schema, generate_dir);
            } else if (db.IsConnected()) {
                if (schema) {
                    if (DbCreateTable(db, schema) && DbImportDbc(db, dbc, schema, false)) {
                        printf("  Imported %u records into database\n", dbc.GetRecordCount());
                    }
                } else {
                    printf("  No schema available, skipping database import\n");
                }
            } else {
                DumpDbcDetailed(dbc, schema);
            }
        }
        printf("\n");
        return 0;
    }

    std::vector<DbcResult> results;
    std::vector<std::string> warnings;

    for (const auto& [name, raw] : dbc_data) {
        DbcFile dbc;
        if (!dbc.Load(raw.data(), (uint32_t)raw.size())) {
            warnings.push_back(name + ": failed to parse DBC header");
            continue;
        }

        const DbcSchema* schema = GetSchema(expansion, name.c_str());

        DbcResult res;
        res.name = name;
        res.records = dbc.GetRecordCount();
        res.fields = dbc.GetFieldCount();
        res.record_size = dbc.GetRecordSize();
        res.has_schema = (schema != nullptr);
        res.packed = (schema && schema->packed);
        res.generated = false;
        res.exported = false;
        res.imported = false;

        // Check for schema mismatch
        if (schema) {
            if (schema->packed) {
                uint32_t expected = GetSchemaRecordSize(schema);
                if (expected != dbc.GetRecordSize()) {
                    char buf[256];
                    snprintf(buf, sizeof(buf), "%-35s schema=%uB, dbc=%uB",
                             name.c_str(), expected, dbc.GetRecordSize());
                    res.warning = buf;
                }
            } else {
                if (schema->field_count != dbc.GetFieldCount()) {
                    char buf[256];
                    snprintf(buf, sizeof(buf), "%-35s schema=%u fields, dbc=%u fields",
                             name.c_str(), schema->field_count, dbc.GetFieldCount());
                    res.warning = buf;
                }
            }
        }

        // Process based on mode
        if (export_dir && res.warning.empty()) {
            if (schema) {
                DbcWriter writer;
                if (writer.ImportFrom(dbc, schema)) {
                    fs::path out_path = fs::path(export_dir) / (name + ".dbc");
                    fs::create_directories(export_dir);
                    res.exported = writer.Save(out_path);
                }
            } else {
                // No schema - raw export
                fs::path out_path = fs::path(export_dir) / (name + ".dbc");
                fs::create_directories(export_dir);
                FILE* fout = fopen(out_path.string().c_str(), "wb");
                if (fout) {
                    fwrite(raw.data(), 1, raw.size(), fout);
                    fclose(fout);
                    res.exported = true;
                }
            }
        } else if (generate_dir && res.warning.empty()) {
            res.generated = GenerateHeader(dbc, schema, generate_dir, !verbose);
        } else if (db.IsConnected() && schema && res.warning.empty()) {
            if (DbCreateTable(db, schema)) {
                res.imported = DbImportDbc(db, dbc, schema, !verbose);
            }
        }

        results.push_back(res);
    }

    printf("\n");

    if (generate_dir) {
        uint32_t gen_ok = 0, gen_skip = 0;
        for (const auto& r : results) {
            if (r.generated) gen_ok++;
            else gen_skip++;
        }
        printf("Generated %u headers, %u skipped\n", gen_ok, gen_skip);
        if (generate_dir) {
            printf("Output: %s\n", generate_dir);
        }
    } else if (export_dir) {
        uint32_t exp_ok = 0, exp_skip = 0;
        for (const auto& r : results) {
            if (r.exported) exp_ok++;
            else exp_skip++;
        }
        printf("Exported %u DBC files, %u skipped\n", exp_ok, exp_skip);
        if (export_dir) {
            printf("Output: %s\n", export_dir);
        }
    } else if (db.IsConnected()) {
        uint32_t imp_ok = 0, imp_skip = 0;
        for (const auto& r : results) {
            if (r.imported) imp_ok++;
            else imp_skip++;
        }
        printf("Imported %u DBC tables, %u skipped\n", imp_ok, imp_skip);
        db.Disconnect();
    } else {
        printf("  %-35s %8s %6s %8s  %s\n", "Name", "Records", "Fields", "RecSize", "Schema");
        printf("  %-35s %8s %6s %8s  %s\n",
               "-----------------------------------", "--------", "------", "--------", "------");

        for (const auto& r : results) {
            const char* schema_status;
            if (!r.has_schema)           schema_status = "-";
            else if (!r.warning.empty()) schema_status = "MISMATCH";
            else if (r.packed)           schema_status = "packed";
            else                         schema_status = "ok";

            printf("  %-35s %8u %6u %8u  %s\n",
                   r.name.c_str(), r.records, r.fields, r.record_size, schema_status);
        }

        uint32_t with_schema = 0, without_schema = 0, packed_count = 0, mismatch_count = 0;
        for (const auto& r : results) {
            if (r.has_schema) with_schema++;
            else without_schema++;
            if (r.packed) packed_count++;
            if (!r.warning.empty()) mismatch_count++;
        }
        printf("\n  %zu total, %u with schema, %u without, %u packed",
               results.size(), with_schema, without_schema, packed_count);
        if (mismatch_count > 0) {
            printf(", %u mismatched", mismatch_count);
        }
        printf("\n");
    }

    bool has_warnings = false;
    for (const auto& r : results) {
        if (!r.warning.empty()) {
            if (!has_warnings) {
                printf("\nWarnings:\n");
                has_warnings = true;
            }
            printf("  %s\n", r.warning.c_str());
        }
    }
    for (const auto& w : warnings) {
        if (!has_warnings) {
            printf("\nWarnings:\n");
            has_warnings = true;
        }
        printf("  %s\n", w.c_str());
    }

    printf("\n");
    return 0;
}
