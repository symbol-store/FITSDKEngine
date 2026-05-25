// BOSS-independent FIT byte-skip parser.
//
// Decodes only the message types listed in ParseConfig::wanted_msgs, skipping
// all other data records at the byte level.  Within a wanted message, only
// the fields named in ParseConfig::wanted_fields (or all fields when that set
// is empty) are decoded; the remaining bytes inside the payload are not read.
//
// The hot-loop invariant: advancing past a non-wanted record costs one branch
// and one pointer-bump (payload_size is precomputed from the definition).
//
// Output: RawTable per message type, with RawColumn per field.  Values are raw
// (no scale/offset, no component/sub-field expansion).  Null slots use the
// NaN/empty-string sentinel; is_null[] tracks them explicitly so the caller
// can substitute any sentinel it prefers.

#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace fit_skip {

// One decoded column.  Values and nulls are stored in lockstep; the caller
// should call pad_to(table.row_count) before adding each value so that sparse
// messages produce NULL-padded columns of uniform length.
struct RawColumn {
  bool is_double = true; // set to false once a STRING value is first seen

  std::vector<double>      doubles;
  std::vector<std::string> strings;
  std::vector<bool>        is_null;
  size_t                   committed = 0; // rows pushed (doubles/strings + null rows)

  void set_double_type() noexcept { is_double = true; }
  void set_string_type() noexcept { is_double = false; }

  void push_double(double v) {
    doubles.push_back(v);
    is_null.push_back(false);
    ++committed;
  }
  void push_string(std::string v) {
    strings.push_back(std::move(v));
    is_null.push_back(false);
    ++committed;
  }
  void push_null() {
    if(is_double)
      doubles.push_back(0.0);
    else
      strings.push_back({});
    is_null.push_back(true);
    ++committed;
  }
  void pad_to(size_t target) {
    while(committed < target)
      push_null();
  }
};

struct RawTable {
  size_t                                    row_count = 0;
  std::unordered_map<std::string, RawColumn> columns;
};

// One pre-resolved field slot stored in the arena.
// column points directly into RawTable::columns; null means "skip this field".
struct FieldLayout {
  RawColumn* column;    // non-null => project this field
  uint8_t    size;      // byte count in the payload
  uint8_t    base_type; // FIT base type byte
  uint16_t   offset;    // byte offset from start of payload
};

// Per-local-message-type state rebuilt on every definition record.
struct LocalDef {
  uint16_t global_msg_num = 0;
  uint16_t payload_size   = 0;
  bool     big_endian     = false;
  bool     valid          = false; // false until first definition record seen
  bool     wanted         = false;

  // Pre-filtered to only the fields the query projects.
  // Points into the Arena owned by the parse() call; empty when !wanted.
  std::span<const FieldLayout> wanted_fields;

  // Timestamp field tracking (field def num 253).
  // Needed even for non-projected fields to expand compressed-timestamp records.
  bool     has_ts       = false;
  uint16_t ts_offset    = 0;       // byte offset of field 253 in payload
  RawColumn* ts_column  = nullptr; // non-null when field 253 is projected
};

// Fixed-size bump arena.  One allocation per definition record; old slices are
// abandoned when a local type is redefined (happens rarely in practice).
class Arena {
  static constexpr size_t kCapacity = 16 * 256;
  FieldLayout storage_[kCapacity];
  size_t      used_ = 0;

public:
  std::span<FieldLayout> alloc(size_t n) noexcept {
    n = std::min(n, kCapacity - used_);
    std::span<FieldLayout> s{storage_ + used_, n};
    used_ += n;
    return s;
  }
  void reset() noexcept { used_ = 0; }
};

struct ParseConfig {
  // Message names to decode.  Empty = decode all.
  std::unordered_set<std::string> wanted_msgs;
  // Per-message field names.  Empty entry for a message = decode all its fields.
  std::unordered_map<std::string, std::unordered_set<std::string>> wanted_fields;
};

struct ParseResult {
  std::unordered_map<std::string, RawTable> tables;
  std::string error;           // non-empty on failure
  uint64_t    bytes_decoded = 0;
  uint64_t    bytes_skipped = 0;
};

// Parse one FIT file image.  The buffer must remain valid for the duration of
// the call but may be released immediately after it returns.
ParseResult parse(const uint8_t* data, size_t size, const ParseConfig& cfg) noexcept;

} // namespace fit_skip
