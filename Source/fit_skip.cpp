// FIT byte-skip parser — implementation.
//
// Profile lookups (message name, field name) are resolved once per definition
// record via the FIT SDK profile arrays.  All remaining work is byte-level.

#include "fit_skip.hpp"

#include "fit.hpp"         // FIT_MESG_NUM_INVALID, FIT_BASE_TYPE_* constants
#include "fit_profile.hpp" // fit::Profile::mesgs[], Profile::MESG / FIELD structs

#include <bit>
#include <cassert>
#include <cmath>
#include <cstring>
#include <limits>
#include <string_view>

namespace fit_skip {
namespace {

// ─── FIT format constants ────────────────────────────────────────────────────

static constexpr uint8_t kCompressedTsBit = 0x80;
static constexpr uint8_t kDefinitionBit   = 0x40;
static constexpr uint8_t kDevDataBit      = 0x20;

static constexpr uint8_t kBaseTypeString  = 0x07;
static constexpr uint8_t kBaseTypeMask    = 0x9F; // strips reserved bits
static constexpr uint8_t kTsFieldNum      = 253;

// FIT timestamp epoch offset to Unix epoch (seconds).
// Dec 31 1989 00:00:00 UTC = Unix 631065600.
static constexpr uint32_t kFitEpochOffset = 631065600u;

// ─── Profile name lookup ─────────────────────────────────────────────────────
// Called only at definition-bind time; linear scans are acceptable.

static std::string_view profile_msg_name(uint16_t global_msg_num) noexcept {
  const auto* m = fit::Profile::GetMesg(global_msg_num);
  return m ? std::string_view{m->name} : std::string_view{};
}

static std::string_view profile_field_name(uint16_t global_msg_num,
                                            uint8_t  field_def_num) noexcept {
  const auto* m = fit::Profile::GetMesg(global_msg_num);
  if(!m) return {};
  for(FIT_UINT16 j = 0; j < m->numFields; ++j)
    if(m->fields[j].num == field_def_num)
      return m->fields[j].name;
  return {};
}

// ─── Raw value decoder ───────────────────────────────────────────────────────
// Returns NaN for the type's "invalid" sentinel value.

static uint16_t read_u16_le(const uint8_t* p) noexcept {
  uint16_t v;
  std::memcpy(&v, p, 2);
  return v;
}
static uint32_t read_u32_le(const uint8_t* p) noexcept {
  uint32_t v;
  std::memcpy(&v, p, 4);
  return v;
}
static uint64_t read_u64_le(const uint8_t* p) noexcept {
  uint64_t v;
  std::memcpy(&v, p, 8);
  return v;
}
static uint16_t read_u16(const uint8_t* p, bool be) noexcept {
  uint16_t v = read_u16_le(p);
  return be ? std::byteswap(v) : v;
}
static uint32_t read_u32(const uint8_t* p, bool be) noexcept {
  uint32_t v = read_u32_le(p);
  return be ? std::byteswap(v) : v;
}
static uint64_t read_u64(const uint8_t* p, bool be) noexcept {
  uint64_t v = read_u64_le(p);
  return be ? std::byteswap(v) : v;
}

static constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();

static double decode_value(const uint8_t* ptr, uint8_t base_type, bool big_endian) noexcept {
  switch(base_type & kBaseTypeMask) {
  case 0x00: { // ENUM
    uint8_t v = *ptr;
    return (v == 0xFF) ? kNaN : static_cast<double>(v);
  }
  case 0x01: { // SINT8
    int8_t v = static_cast<int8_t>(*ptr);
    return (v == 0x7F) ? kNaN : static_cast<double>(v);
  }
  case 0x02: { // UINT8
    uint8_t v = *ptr;
    return (v == 0xFF) ? kNaN : static_cast<double>(v);
  }
  case 0x0A: { // UINT8Z
    uint8_t v = *ptr;
    return (v == 0x00) ? kNaN : static_cast<double>(v);
  }
  case 0x0D: { // BYTE
    return static_cast<double>(*ptr);
  }
  case 0x83: { // SINT16
    int16_t v = static_cast<int16_t>(read_u16(ptr, big_endian));
    return (v == 0x7FFF) ? kNaN : static_cast<double>(v);
  }
  case 0x84: { // UINT16
    uint16_t v = read_u16(ptr, big_endian);
    return (v == 0xFFFF) ? kNaN : static_cast<double>(v);
  }
  case 0x8B: { // UINT16Z
    uint16_t v = read_u16(ptr, big_endian);
    return (v == 0x0000) ? kNaN : static_cast<double>(v);
  }
  case 0x85: { // SINT32
    int32_t v = static_cast<int32_t>(read_u32(ptr, big_endian));
    return (v == 0x7FFFFFFF) ? kNaN : static_cast<double>(v);
  }
  case 0x86: { // UINT32
    uint32_t v = read_u32(ptr, big_endian);
    return (v == 0xFFFFFFFF) ? kNaN : static_cast<double>(v);
  }
  case 0x8C: { // UINT32Z
    uint32_t v = read_u32(ptr, big_endian);
    return (v == 0x00000000) ? kNaN : static_cast<double>(v);
  }
  case 0x88: { // FLOAT32
    uint32_t bits = read_u32(ptr, big_endian);
    if(bits == 0xFFFFFFFF)
      return kNaN;
    float f;
    std::memcpy(&f, &bits, 4);
    return static_cast<double>(f);
  }
  case 0x89: { // FLOAT64
    uint64_t bits = read_u64(ptr, big_endian);
    if(bits == 0xFFFFFFFFFFFFFFFFULL)
      return kNaN;
    double d;
    std::memcpy(&d, &bits, 8);
    return d;
  }
  case 0x8E: { // SINT64
    int64_t v = static_cast<int64_t>(read_u64(ptr, big_endian));
    return (v == 0x7FFFFFFFFFFFFFFFLL) ? kNaN : static_cast<double>(v);
  }
  case 0x8F: { // UINT64
    uint64_t v = read_u64(ptr, big_endian);
    return (v == 0xFFFFFFFFFFFFFFFFULL) ? kNaN : static_cast<double>(v);
  }
  case 0x90: { // UINT64Z
    uint64_t v = read_u64(ptr, big_endian);
    return (v == 0x0000000000000000ULL) ? kNaN : static_cast<double>(v);
  }
  default:
    return kNaN;
  }
}

// ─── String field decoder ────────────────────────────────────────────────────

static std::string decode_string(const uint8_t* ptr, uint8_t size) {
  // FIT strings are null-padded to their fixed field size.
  size_t len = 0;
  while(len < size && ptr[len] != 0x00)
    ++len;
  return std::string(reinterpret_cast<const char*>(ptr), len);
}

// ─── Definition record parser ────────────────────────────────────────────────

// Advances *cur past the definition body and fills in def.
// Returns false on truncation.
static bool parse_definition(LocalDef& def, const uint8_t*& cur, const uint8_t* end,
                              bool has_dev, const ParseConfig& cfg,
                              Arena& arena, RawTable& tbl) noexcept {
  if(cur + 5 > end)
    return false;

  // reserved (1), architecture (1), global_msg_num (2), num_fields (1)
  bool big_endian   = (cur[1] == 1);
  uint16_t msg_num  = big_endian ? (uint16_t(cur[2]) << 8 | cur[3])
                                 : (uint16_t(cur[3]) << 8 | cur[2]);
  uint8_t  n_fields = cur[4];
  cur += 5;

  if(cur + n_fields * 3 > end)
    return false;

  // Accumulate payload size from all fields (regular + developer).
  uint16_t payload_size = 0;

  // Collect (field_def_num, size, base_type, offset) for all regular fields.
  struct RawFieldDesc {
    uint8_t  field_def_num;
    uint8_t  size;
    uint8_t  base_type;
    uint16_t offset;
  };
  RawFieldDesc raw[255]; // max FIT fields per message
  for(uint8_t i = 0; i < n_fields; ++i) {
    raw[i].field_def_num = cur[0];
    raw[i].size          = cur[1];
    raw[i].base_type     = cur[2];
    raw[i].offset        = payload_size;
    payload_size += cur[1];
    cur += 3;
  }

  // Developer fields: consume their definitions, add to payload_size.
  uint16_t dev_payload = 0;
  if(has_dev) {
    if(cur >= end)
      return false;
    uint8_t n_dev = *cur++;
    if(cur + n_dev * 3 > end)
      return false;
    for(uint8_t i = 0; i < n_dev; ++i) {
      dev_payload += cur[1];
      cur += 3;
    }
  }

  // Commit definition metadata.
  def.global_msg_num = msg_num;
  def.payload_size   = payload_size + dev_payload;
  def.big_endian     = big_endian;
  def.valid          = true;
  def.has_ts         = false;
  def.ts_column      = nullptr;

  // Determine if this message is wanted.
  auto msg_name = profile_msg_name(msg_num);
  bool all_msgs = cfg.wanted_msgs.empty();
  def.wanted = all_msgs ||
               (!msg_name.empty() && cfg.wanted_msgs.count(std::string(msg_name)));

  def.wanted_fields = {};

  if(!def.wanted)
    return true; // nothing more to do for non-wanted types

  // Determine field projection for this message.
  std::string msg_name_str(msg_name.empty() ? ("msg_" + std::to_string(msg_num)) : msg_name);
  const std::unordered_set<std::string>* proj = nullptr;
  auto pit = cfg.wanted_fields.find(msg_name_str);
  if(pit != cfg.wanted_fields.end() && !pit->second.empty())
    proj = &pit->second;
  bool all_fields = (proj == nullptr);

  // Build wanted_fields in the arena.
  // First pass: count how many fields are wanted.
  uint8_t wanted_count = 0;
  for(uint8_t i = 0; i < n_fields; ++i) {
    uint8_t fnum = raw[i].field_def_num;
    auto fname   = profile_field_name(msg_num, fnum);
    bool is_ts   = (fnum == kTsFieldNum);
    std::string fname_str(fname.empty() ? ("field_" + std::to_string(fnum)) : fname);
    bool project = all_fields || is_ts || proj->count(fname_str);
    if(project)
      ++wanted_count;
  }

  auto slice = arena.alloc(wanted_count);
  uint8_t slot = 0;

  for(uint8_t i = 0; i < n_fields; ++i) {
    uint8_t  fnum     = raw[i].field_def_num;
    uint8_t  fsize    = raw[i].size;
    uint8_t  ftype    = raw[i].base_type;
    uint16_t foffset  = raw[i].offset;
    bool     is_ts    = (fnum == kTsFieldNum);
    auto     fname    = profile_field_name(msg_num, fnum);
    std::string fname_str(fname.empty() ? ("field_" + std::to_string(fnum)) : fname);
    bool project = all_fields || is_ts || (proj && proj->count(fname_str));

    if(!project)
      continue;

    // Resolve / create the output column.
    auto& col = tbl.columns[fname_str];
    if(ftype == kBaseTypeString)
      col.set_string_type();
    else
      col.set_double_type();

    // Track timestamp field for compressed-ts expansion.
    if(is_ts) {
      def.has_ts      = true;
      def.ts_offset   = foffset;
      // Only project into ts_column if it was explicitly requested or all_fields.
      bool ts_wanted = all_fields || (proj && proj->count(fname_str));
      def.ts_column   = ts_wanted ? &col : nullptr;
    }

    slice[slot++] = FieldLayout{
        .column    = &col,
        .size      = fsize,
        .base_type = ftype,
        .offset    = foffset,
    };
  }

  def.wanted_fields = slice.first(slot);
  return true;
}

// ─── Payload decoder ─────────────────────────────────────────────────────────

[[gnu::noinline]] static void decode_payload(const LocalDef& def, const uint8_t* payload,
                                             RawTable& tbl,
                                             uint32_t  ts_override,
                                             bool      use_ts_override) noexcept {
  // Inject timestamp if compressed-ts record.
  if(def.has_ts) {
    if(use_ts_override) {
      if(def.ts_column) {
        def.ts_column->pad_to(tbl.row_count);
        def.ts_column->push_double(static_cast<double>(ts_override));
      }
    }
    // When not using an override the timestamp comes through wanted_fields below.
  }

  for(const auto& fl : def.wanted_fields) {
    // Skip if this is the timestamp and we already injected it above.
    if(use_ts_override && fl.column == def.ts_column && def.has_ts)
      continue;

    if(!fl.column)
      continue;
    fl.column->pad_to(tbl.row_count);

    if((fl.base_type & kBaseTypeMask) == kBaseTypeString) {
      auto s = decode_string(payload + fl.offset, fl.size);
      if(s.empty())
        fl.column->push_null();
      else
        fl.column->push_string(std::move(s));
    } else {
      double v = decode_value(payload + fl.offset, fl.base_type, def.big_endian);
      if(std::isnan(v))
        fl.column->push_null();
      else
        fl.column->push_double(v);
    }
  }

  tbl.row_count++;
}

// ─── File header parser ──────────────────────────────────────────────────────

struct FileHeader {
  uint32_t data_size;  // bytes of records following the header
  uint8_t  hdr_size;   // 12 or 14
};

static bool parse_file_header(const uint8_t* data, size_t size,
                               FileHeader& out) noexcept {
  if(size < 12)
    return false;
  out.hdr_size = data[0];
  if(out.hdr_size < 12 || size < out.hdr_size)
    return false;
  // ".FIT" magic at bytes 8..11
  if(data[8] != '.' || data[9] != 'F' || data[10] != 'I' || data[11] != 'T')
    return false;
  std::memcpy(&out.data_size, data + 4, 4); // always little-endian
  return true;
}

} // namespace

// ─── Public entry point ───────────────────────────────────────────────────────

ParseResult parse(const uint8_t* data, size_t size, const ParseConfig& cfg) noexcept {
  ParseResult result;
  if(!data || size < 12) {
    result.error = "file too short to be a valid FIT file";
    return result;
  }

  // State shared across the hot loop.
  LocalDef defs[16]{};
  Arena    arena;
  uint32_t base_ts = 0;

  const uint8_t* const file_start = data;
  const uint8_t*       cur        = data;
  const uint8_t* const file_end   = data + size;

  // Support chained FIT files: keep decoding while we find valid headers.
  while(cur < file_end) {
    FileHeader hdr;
    if(!parse_file_header(cur, static_cast<size_t>(file_end - cur), hdr)) {
      if(cur == file_start)
        result.error = "invalid FIT file header";
      break; // end of chained files or truncation
    }

    const uint8_t* rec_begin = cur + hdr.hdr_size;
    const uint8_t* rec_end   = rec_begin + hdr.data_size;
    if(rec_end > file_end)
      rec_end = file_end; // truncated file; decode what we have

    // Reset arena and local-def state for each chained segment.
    arena.reset();
    for(auto& d : defs)
      d = LocalDef{};

    cur = rec_begin;

    // ── Hot loop ──────────────────────────────────────────────────────────────
    while(cur < rec_end) {
      uint8_t h = *cur++;

      if(h & kCompressedTsBit) {
        // Compressed-timestamp data record: 2-bit local type, 5-bit time offset.
        uint8_t   local_type  = (h >> 5) & 0x3;
        uint8_t   time_offset = h & 0x1F;
        LocalDef& d           = defs[local_type];

        if(!d.valid) {
          // Corrupt: data before definition.
          result.error = "data record before definition";
          goto done;
        }

        // Expand 5-bit offset to full timestamp.
        uint32_t ts = (base_ts & ~0x1Fu) | time_offset;
        if(ts < base_ts)
          ts += 32;
        base_ts = ts;

        if(d.wanted) {
          if(cur + d.payload_size > rec_end)
            break; // truncated
          auto& tbl = result.tables[std::string(profile_msg_name(d.global_msg_num).empty()
                                                    ? "msg_" + std::to_string(d.global_msg_num)
                                                    : profile_msg_name(d.global_msg_num))];
          result.bytes_decoded += d.payload_size;
          decode_payload(d, cur, tbl, ts, /*use_ts_override=*/true);
        } else {
          result.bytes_skipped += d.payload_size;
        }
        cur += d.payload_size;

      } else if(h & kDefinitionBit) {
        // Definition record.
        uint8_t local_type = h & 0x0F;
        bool    has_dev    = (h & kDevDataBit) != 0;

        // Resolve the message table name upfront using the pre-parse peek.
        // parse_definition reads further into cur, so do it unconditionally.
        auto& d = defs[local_type];

        // To bind the wanted_fields we need the RawTable; determine the msg
        // name before calling parse_definition by peeking at bytes 2..3.
        if(cur + 5 > rec_end) {
          result.error = "truncated definition record";
          goto done;
        }
        bool     be_peek      = (cur[1] == 1);
        uint16_t msg_num_peek = be_peek ? (uint16_t(cur[2]) << 8 | cur[3])
                                        : (uint16_t(cur[3]) << 8 | cur[2]);
        auto     mn           = profile_msg_name(msg_num_peek);
        std::string msg_name_str(mn.empty() ? "msg_" + std::to_string(msg_num_peek) : mn);

        auto& tbl = result.tables[msg_name_str];
        if(!parse_definition(d, cur, rec_end, has_dev, cfg, arena, tbl)) {
          result.error = "truncated definition record body";
          goto done;
        }

      } else {
        // Normal data record.
        uint8_t   local_type = h & 0x0F;
        LocalDef& d          = defs[local_type];

        if(!d.valid) {
          result.error = "data record before definition";
          goto done;
        }

        if(cur + d.payload_size > rec_end)
          break; // truncated; stop cleanly

        if(d.wanted) {
          auto msg_name = profile_msg_name(d.global_msg_num);
          auto& tbl     = result.tables[msg_name.empty()
                                            ? "msg_" + std::to_string(d.global_msg_num)
                                            : std::string(msg_name)];
          result.bytes_decoded += d.payload_size;
          decode_payload(d, cur, tbl, 0, /*use_ts_override=*/false);

          // Update base timestamp for future compressed-ts expansion.
          if(d.has_ts) {
            uint32_t ts =
                static_cast<uint32_t>(read_u32(cur + d.ts_offset, d.big_endian));
            if(ts != 0xFFFFFFFF)
              base_ts = ts;
          }
        } else {
          result.bytes_skipped += d.payload_size;
          // Still update base_ts for local types 0..3 (the only ones that can
          // appear as compressed-timestamp records).
          if(local_type <= 3 && d.has_ts) {
            uint32_t ts =
                static_cast<uint32_t>(read_u32(cur + d.ts_offset, d.big_endian));
            if(ts != 0xFFFFFFFF)
              base_ts = ts;
          }
        }
        cur += d.payload_size;
      }
    }
    // ── End hot loop ──────────────────────────────────────────────────────────

    // Advance past the trailing 2-byte file CRC (if present).
    cur = rec_end + 2;
  }

done:
  // Pad all columns in every table to their table's row_count.
  for(auto& [name, tbl] : result.tables)
    for(auto& [col_name, col] : tbl.columns)
      col.pad_to(tbl.row_count);

  return result;
}

} // namespace fit_skip
