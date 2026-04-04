#include "dbc_writer.h"
#include <cstring>
#include <cstdio>

DbcWriter::DbcWriter()
    : schema_(nullptr), record_size_(0) {
}

void DbcWriter::Init(const DbcSchema* schema) {
    schema_ = schema;
    records_.clear();
    strings_.clear();
    string_dedup_.clear();

    if (schema->packed) {
        record_size_ = GetSchemaRecordSize(schema);
    } else {
        record_size_ = schema->field_count * sizeof(uint32_t);
    }

    // Seed string pool with empty string at offset 0
    strings_.push_back("");
    string_dedup_[""] = 0;
}

bool DbcWriter::ImportFrom(const DbcFile& dbc, const DbcSchema* schema) {
    Init(schema);

    if (record_size_ != dbc.GetRecordSize()) {
        printf("DbcWriter::ImportFrom: record size mismatch (schema=%u, dbc=%u)\n",
               record_size_, dbc.GetRecordSize());
        return false;
    }

    // Import all records
    for (uint32_t r = 0; r < dbc.GetRecordCount(); r++) {
        const uint8_t* row = dbc.GetRecord(r);
        if (!row) {
            continue;
        }

        records_.emplace_back(row, row + record_size_);

        // For string fields, import the string values into our pool
        // and rewrite the offset in the record to point into our pool
        for (uint32_t f = 0; f < schema_->field_count; f++) {
            if (schema_->fields[f].type != DbcFieldType::String) {
                continue;
            }

            uint32_t byte_off = GetFieldOffset(f);
            uint32_t old_offset;
            memcpy(&old_offset, records_.back().data() + byte_off, sizeof(uint32_t));

            const char* str = dbc.GetString(old_offset);
            uint32_t new_offset = AddString(str);

            memcpy(records_.back().data() + byte_off, &new_offset, sizeof(uint32_t));
        }
    }

    return true;
}

uint32_t DbcWriter::AddRecord() {
    records_.emplace_back(record_size_, 0);
    return static_cast<uint32_t>(records_.size() - 1);
}

bool DbcWriter::RemoveRecord(uint32_t index) {
    if (index >= records_.size()) {
        return false;
    }
    records_.erase(records_.begin() + index);
    return true;
}

uint32_t DbcWriter::GetRecordCount() const {
    return static_cast<uint32_t>(records_.size());
}

uint32_t DbcWriter::GetFieldOffset(uint32_t field) const {
    if (!schema_ || field >= schema_->field_count) {
        return 0;
    }
    if (schema_->packed) {
        return DbcFile::GetFieldOffset(schema_, field);
    }
    return field * sizeof(uint32_t);
}

// --- Setters ---

void DbcWriter::SetUInt32(uint32_t record, uint32_t field, uint32_t value) {
    if (record >= records_.size() || !schema_ || field >= schema_->field_count) return;
    uint32_t off = GetFieldOffset(field);
    memcpy(records_[record].data() + off, &value, sizeof(uint32_t));
}

void DbcWriter::SetInt32(uint32_t record, uint32_t field, int32_t value) {
    if (record >= records_.size() || !schema_ || field >= schema_->field_count) return;
    uint32_t off = GetFieldOffset(field);
    memcpy(records_[record].data() + off, &value, sizeof(int32_t));
}

void DbcWriter::SetFloat(uint32_t record, uint32_t field, float value) {
    if (record >= records_.size() || !schema_ || field >= schema_->field_count) return;
    uint32_t off = GetFieldOffset(field);
    memcpy(records_[record].data() + off, &value, sizeof(float));
}

void DbcWriter::SetString(uint32_t record, uint32_t field, const char* value) {
    if (record >= records_.size() || !schema_ || field >= schema_->field_count) return;
    uint32_t string_offset = AddString(value ? value : "");
    uint32_t off = GetFieldOffset(field);
    memcpy(records_[record].data() + off, &string_offset, sizeof(uint32_t));
}

void DbcWriter::SetUInt8(uint32_t record, uint32_t field, uint8_t value) {
    if (record >= records_.size() || !schema_ || field >= schema_->field_count) return;
    uint32_t off = GetFieldOffset(field);
    records_[record][off] = value;
}

void DbcWriter::SetInt8(uint32_t record, uint32_t field, int8_t value) {
    SetUInt8(record, field, static_cast<uint8_t>(value));
}

void DbcWriter::SetUInt16(uint32_t record, uint32_t field, uint16_t value) {
    if (record >= records_.size() || !schema_ || field >= schema_->field_count) return;
    uint32_t off = GetFieldOffset(field);
    memcpy(records_[record].data() + off, &value, sizeof(uint16_t));
}

void DbcWriter::SetInt16(uint32_t record, uint32_t field, int16_t value) {
    SetUInt16(record, field, static_cast<uint16_t>(value));
}

// --- Getters ---

uint32_t DbcWriter::GetUInt32(uint32_t record, uint32_t field) const {
    if (record >= records_.size() || !schema_ || field >= schema_->field_count) return 0;
    uint32_t off = GetFieldOffset(field);
    uint32_t val;
    memcpy(&val, records_[record].data() + off, sizeof(uint32_t));
    return val;
}

int32_t DbcWriter::GetInt32(uint32_t record, uint32_t field) const {
    return static_cast<int32_t>(GetUInt32(record, field));
}

float DbcWriter::GetFloat(uint32_t record, uint32_t field) const {
    if (record >= records_.size() || !schema_ || field >= schema_->field_count) return 0.0f;
    uint32_t off = GetFieldOffset(field);
    float val;
    memcpy(&val, records_[record].data() + off, sizeof(float));
    return val;
}

const char* DbcWriter::GetString(uint32_t record, uint32_t field) const {
    uint32_t offset = GetUInt32(record, field);
    // Walk the string pool to find the string at this offset
    uint32_t current_offset = 0;
    for (const auto& s : strings_) {
        if (current_offset == offset) {
            return s.c_str();
        }
        current_offset += static_cast<uint32_t>(s.size()) + 1;
    }
    return "";
}

uint8_t DbcWriter::GetUInt8(uint32_t record, uint32_t field) const {
    if (record >= records_.size() || !schema_ || field >= schema_->field_count) return 0;
    uint32_t off = GetFieldOffset(field);
    return records_[record][off];
}

int8_t DbcWriter::GetInt8(uint32_t record, uint32_t field) const {
    return static_cast<int8_t>(GetUInt8(record, field));
}

uint16_t DbcWriter::GetUInt16(uint32_t record, uint32_t field) const {
    if (record >= records_.size() || !schema_ || field >= schema_->field_count) return 0;
    uint32_t off = GetFieldOffset(field);
    uint16_t val;
    memcpy(&val, records_[record].data() + off, sizeof(uint16_t));
    return val;
}

int16_t DbcWriter::GetInt16(uint32_t record, uint32_t field) const {
    return static_cast<int16_t>(GetUInt16(record, field));
}

// --- String pool ---

uint32_t DbcWriter::AddString(const char* str) {
    std::string s(str ? str : "");
    auto it = string_dedup_.find(s);
    if (it != string_dedup_.end()) {
        return it->second;
    }

    // Compute offset: sum of all existing strings + null terminators
    uint32_t offset = 0;
    for (const auto& existing : strings_) {
        offset += static_cast<uint32_t>(existing.size()) + 1;
    }

    string_dedup_[s] = offset;
    strings_.push_back(s);
    return offset;
}

std::pair<std::vector<uint8_t>, std::unordered_map<uint32_t, uint32_t>>
DbcWriter::BuildStringBlock() const {
    std::vector<uint8_t> block;
    std::unordered_map<uint32_t, uint32_t> offset_map;

    uint32_t offset = 0;
    for (const auto& s : strings_) {
        offset_map[offset] = offset;  // identity map (offsets are already correct)
        block.insert(block.end(), s.begin(), s.end());
        block.push_back(0);
        offset += static_cast<uint32_t>(s.size()) + 1;
    }

    return { block, offset_map };
}

// --- Serialization ---

std::vector<uint8_t> DbcWriter::Serialize() const {
    if (!schema_ || records_.empty()) {
        return {};
    }

    auto [string_block, _] = BuildStringBlock();

    // Compute DBC header field_count (always in 4-byte columns for the header)
    uint32_t header_field_count = record_size_ / sizeof(uint32_t);

    dbc_header header;
    header.magic = DBC_MAGIC;
    header.record_count = static_cast<uint32_t>(records_.size());
    header.field_count = header_field_count;
    header.record_size = record_size_;
    header.string_block_size = static_cast<uint32_t>(string_block.size());

    std::vector<uint8_t> output;
    output.reserve(sizeof(dbc_header) + records_.size() * record_size_ + string_block.size());

    // Header
    const uint8_t* hp = reinterpret_cast<const uint8_t*>(&header);
    output.insert(output.end(), hp, hp + sizeof(dbc_header));

    // Records
    for (const auto& rec : records_) {
        output.insert(output.end(), rec.begin(), rec.end());
    }

    // String block
    output.insert(output.end(), string_block.begin(), string_block.end());

    return output;
}

bool DbcWriter::Save(const std::filesystem::path& path) const {
    std::vector<uint8_t> data = Serialize();
    if (data.empty()) {
        return false;
    }

    std::filesystem::create_directories(path.parent_path());

    FILE* f = fopen(path.string().c_str(), "wb");
    if (!f) {
        printf("DbcWriter::Save: failed to open %s\n", path.string().c_str());
        return false;
    }

    size_t written = fwrite(data.data(), 1, data.size(), f);
    fclose(f);

    return written == data.size();
}
