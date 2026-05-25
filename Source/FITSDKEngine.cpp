#include <BOSS.hpp>
#include <Expression.hpp>
#include <ExpressionUtilities.hpp>
#include <Utilities.hpp>

#include "fit_decode.hpp"
#include "fit_hr_mesg.hpp"
#include "fit_mesg.hpp"
#include "fit_mesg_listener.hpp"
#include "fit_profile.hpp"
#include "fit_runtime_exception.hpp"

#include "fit_skip.hpp"

#include <algorithm>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <limits>
#include <map>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>
#include <variant>
#include <vector>

using std::string_literals::operator""s;
using boss::utilities::operator""_;
using boss::ComplexExpression;
using boss::Expression;
using boss::ExpressionArguments;
using boss::Span;
using boss::Symbol;
using boss::expressions::ExpressionSpanArguments;
using namespace boss::utilities::experimental;

namespace {

static_assert(FIT_SPORT_COUNT == 82, "FIT SDK added new sport values — update fitSportName");

static Symbol fitSportName(int sport) {
  switch(sport) {
  case FIT_SPORT_GENERIC:               return Symbol("Generic");
  case FIT_SPORT_RUNNING:               return Symbol("Running");
  case FIT_SPORT_CYCLING:               return Symbol("Cycling");
  case FIT_SPORT_TRANSITION:            return Symbol("Transition");
  case FIT_SPORT_FITNESS_EQUIPMENT:     return Symbol("FitnessEquipment");
  case FIT_SPORT_SWIMMING:              return Symbol("Swimming");
  case FIT_SPORT_BASKETBALL:            return Symbol("Basketball");
  case FIT_SPORT_SOCCER:                return Symbol("Soccer");
  case FIT_SPORT_TENNIS:                return Symbol("Tennis");
  case FIT_SPORT_AMERICAN_FOOTBALL:     return Symbol("AmericanFootball");
  case FIT_SPORT_TRAINING:              return Symbol("Training");
  case FIT_SPORT_WALKING:               return Symbol("Walking");
  case FIT_SPORT_CROSS_COUNTRY_SKIING:  return Symbol("CrossCountrySkiing");
  case FIT_SPORT_ALPINE_SKIING:         return Symbol("AlpineSkiing");
  case FIT_SPORT_SNOWBOARDING:          return Symbol("Snowboarding");
  case FIT_SPORT_ROWING:                return Symbol("Rowing");
  case FIT_SPORT_MOUNTAINEERING:        return Symbol("Mountaineering");
  case FIT_SPORT_HIKING:                return Symbol("Hiking");
  case FIT_SPORT_MULTISPORT:            return Symbol("Multisport");
  case FIT_SPORT_PADDLING:              return Symbol("Paddling");
  case FIT_SPORT_FLYING:                return Symbol("Flying");
  case FIT_SPORT_E_BIKING:              return Symbol("EBiking");
  case FIT_SPORT_MOTORCYCLING:          return Symbol("Motorcycling");
  case FIT_SPORT_BOATING:               return Symbol("Boating");
  case FIT_SPORT_DRIVING:               return Symbol("Driving");
  case FIT_SPORT_GOLF:                  return Symbol("Golf");
  case FIT_SPORT_HANG_GLIDING:          return Symbol("HangGliding");
  case FIT_SPORT_HORSEBACK_RIDING:      return Symbol("HorsebackRiding");
  case FIT_SPORT_HUNTING:               return Symbol("Hunting");
  case FIT_SPORT_FISHING:               return Symbol("Fishing");
  case FIT_SPORT_INLINE_SKATING:        return Symbol("InlineSkating");
  case FIT_SPORT_ROCK_CLIMBING:         return Symbol("RockClimbing");
  case FIT_SPORT_SAILING:               return Symbol("Sailing");
  case FIT_SPORT_ICE_SKATING:           return Symbol("IceSkating");
  case FIT_SPORT_SKY_DIVING:            return Symbol("SkyDiving");
  case FIT_SPORT_SNOWSHOEING:           return Symbol("Snowshoeing");
  case FIT_SPORT_SNOWMOBILING:          return Symbol("Snowmobiling");
  case FIT_SPORT_STAND_UP_PADDLEBOARDING: return Symbol("StandUpPaddleboarding");
  case FIT_SPORT_SURFING:               return Symbol("Surfing");
  case FIT_SPORT_WAKEBOARDING:          return Symbol("Wakeboarding");
  case FIT_SPORT_WATER_SKIING:          return Symbol("WaterSkiing");
  case FIT_SPORT_KAYAKING:              return Symbol("Kayaking");
  case FIT_SPORT_RAFTING:               return Symbol("Rafting");
  case FIT_SPORT_WINDSURFING:           return Symbol("Windsurfing");
  case FIT_SPORT_KITESURFING:           return Symbol("Kitesurfing");
  case FIT_SPORT_TACTICAL:              return Symbol("Tactical");
  case FIT_SPORT_JUMPMASTER:            return Symbol("Jumpmaster");
  case FIT_SPORT_BOXING:                return Symbol("Boxing");
  case FIT_SPORT_FLOOR_CLIMBING:        return Symbol("FloorClimbing");
  case FIT_SPORT_BASEBALL:              return Symbol("Baseball");
  case FIT_SPORT_DIVING:                return Symbol("Diving");
  case FIT_SPORT_SHOOTING:              return Symbol("Shooting");
  case FIT_SPORT_WINTER_SPORT:          return Symbol("WinterSport");
  case FIT_SPORT_GRINDING:              return Symbol("Grinding");
  case FIT_SPORT_HIIT:                  return Symbol("Hiit");
  case FIT_SPORT_VIDEO_GAMING:          return Symbol("VideoGaming");
  case FIT_SPORT_RACKET:                return Symbol("Racket");
  case FIT_SPORT_WHEELCHAIR_PUSH_WALK:  return Symbol("WheelchairPushWalk");
  case FIT_SPORT_WHEELCHAIR_PUSH_RUN:   return Symbol("WheelchairPushRun");
  case FIT_SPORT_MEDITATION:            return Symbol("Meditation");
  case FIT_SPORT_PARA_SPORT:            return Symbol("ParaSport");
  case FIT_SPORT_DISC_GOLF:             return Symbol("DiscGolf");
  case FIT_SPORT_TEAM_SPORT:            return Symbol("TeamSport");
  case FIT_SPORT_CRICKET:               return Symbol("Cricket");
  case FIT_SPORT_RUGBY:                 return Symbol("Rugby");
  case FIT_SPORT_HOCKEY:                return Symbol("Hockey");
  case FIT_SPORT_LACROSSE:              return Symbol("Lacrosse");
  case FIT_SPORT_VOLLEYBALL:            return Symbol("Volleyball");
  case FIT_SPORT_WATER_TUBING:          return Symbol("WaterTubing");
  case FIT_SPORT_WAKESURFING:           return Symbol("Wakesurfing");
  case FIT_SPORT_WATER_SPORT:           return Symbol("WaterSport");
  case FIT_SPORT_ARCHERY:               return Symbol("Archery");
  case FIT_SPORT_MIXED_MARTIAL_ARTS:    return Symbol("MixedMartialArts");
  case FIT_SPORT_MOTOR_SPORTS:          return Symbol("MotorSports");
  case FIT_SPORT_SNORKELING:            return Symbol("Snorkeling");
  case FIT_SPORT_DANCE:                 return Symbol("Dance");
  case FIT_SPORT_JUMP_ROPE:             return Symbol("JumpRope");
  case FIT_SPORT_POOL_APNEA:            return Symbol("PoolApnea");
  case FIT_SPORT_MOBILITY:              return Symbol("Mobility");
  case FIT_SPORT_GEOCACHING:            return Symbol("Geocaching");
  case FIT_SPORT_CANOEING:              return Symbol("Canoeing");
  case FIT_SPORT_ALL:                   return Symbol("All");
  default: return Symbol("Sport" + std::to_string(sport));
  }
}

// ── Column builder ─────────────────────────────────────────────────────────────

struct ColumnBuilder {
  ExpressionSpanArguments spans;
  std::variant<std::vector<double>, std::vector<std::string>, std::vector<Symbol>> buffer;
  size_t committedRows = 0;

  void flush() {
    std::visit(
        [this](auto& vec) {
          if(!vec.empty())
            spans.push_back(Span<typename std::decay_t<decltype(vec)>::value_type>(std::move(vec)));
        },
        buffer);
  }

  template <typename V> void add(V&& value) {
    using Vec = std::vector<std::decay_t<V>>;
    if(!std::holds_alternative<Vec>(buffer)) {
      flush();
      buffer = Vec{};
    }
    std::get<Vec>(buffer).push_back(std::forward<V>(value));
    committedRows++;
  }

  ExpressionSpanArguments build() && {
    flush();
    return std::move(spans);
  }
};

// ── Message table ──────────────────────────────────────────────────────────────

struct MessageTable {
  size_t rowCount = 0;
  std::map<std::string, ColumnBuilder> columns;

  // Returns the named column padded with NULLs to the current row boundary.
  ColumnBuilder& padded(const std::string& name) {
    auto& col = columns[name];
    while(col.committedRows < rowCount)
      col.add(Symbol("NULL"));
    return col;
  }

  // Advances to the next row, padding any columns that weren't written this row.
  void finishRow() {
    ++rowCount;
    for(auto& [_, col] : columns)
      if(col.committedRows < rowCount)
        col.add(Symbol("NULL"));
  }
};

// ── Parsing flags ──────────────────────────────────────────────────────────────

// Options exposed as symbolic flags on LoadFIT: e.g. apply_scale_and_offset_(0).
struct ParseFlags {
  bool apply_scale_and_offset    = true;
  bool expand_components         = true;
  bool expand_sub_fields         = true;
  bool convert_datetimes_to_dates = true;
  bool merge_heart_rates         = false;
  bool enable_crc_check          = true;
};

// ── Constants ──────────────────────────────────────────────────────────────────

// Seconds between FIT epoch (Dec 31 1989 00:00 UTC) and Unix epoch.
static constexpr double kFitEpochOffset = 631065600.0;

// FIT timestamps below this value are relative (time-of-day), not absolute dates.
static constexpr double kFitDateTimeMin = static_cast<double>(0x10000000u);

static bool isDateTimeField(std::string_view name) {
  return name == "timestamp" || name == "local_timestamp";
}

struct HrPoint {
  double timestamp;
  double bpm;
};

// ── Expression construction helpers ───────────────────────────────────────────

static ComplexExpression makeColumn(std::string name, ColumnBuilder builder) {
  return ComplexExpression(Symbol(std::move(name)), {}, {}, std::move(builder).build());
}

static ComplexExpression makeTable(ExpressionArguments columns) {
  return ComplexExpression(Symbol("Table"), {}, std::move(columns), {});
}

// ── File collection ────────────────────────────────────────────────────────────

static std::vector<std::filesystem::path> collectFitFiles(const std::string& path) {
  std::vector<std::filesystem::path> paths;
  if(std::filesystem::is_directory(path)) {
    for(auto const& entry : std::filesystem::directory_iterator(path))
      if(entry.path().extension() == ".fit")
        paths.push_back(entry.path());
    std::ranges::sort(paths);
  } else {
    paths.push_back(path);
  }
  return paths;
}

// ── Memory-mapped read-only file view ─────────────────────────────────────────

struct MmapView {
  const uint8_t* data = nullptr;
  size_t size = 0;
  std::string error;

  explicit MmapView(const std::filesystem::path& path) {
    int fd = ::open(path.c_str(), O_RDONLY);
    if(fd < 0) { error = "cannot open file: "s + path.string(); return; }
    struct stat st{};
    if(::fstat(fd, &st) < 0) {
      ::close(fd);
      error = "cannot stat file: "s + path.string();
      return;
    }
    if(st.st_size == 0) { ::close(fd); return; }
    void* mapping = ::mmap(nullptr, static_cast<size_t>(st.st_size), PROT_READ, MAP_PRIVATE, fd, 0);
    ::close(fd);
    if(mapping == MAP_FAILED) { error = "mmap failed for: "s + path.string(); return; }
    data = static_cast<const uint8_t*>(mapping);
    size = static_cast<size_t>(st.st_size);
  }

  ~MmapView() {
    if(data) ::munmap(const_cast<uint8_t*>(data), size);
  }

  MmapView(const MmapView&) = delete;
  MmapView& operator=(const MmapView&) = delete;

  bool ok()    const { return error.empty(); }
  bool empty() const { return size == 0; }
};

// ── Flag parsing ───────────────────────────────────────────────────────────────

// Reads LoadFIT symbolic flags from dynamics[from..end]. Each flag is a
// ComplexExpression flagName_(0_or_1); bare flag with no argument defaults to true.
static ParseFlags parseFlags(ExpressionArguments& dynamics, size_t from) {
  ParseFlags flags;
  for(size_t i = from; i < dynamics.size(); ++i) {
    auto* flagExpr = std::get_if<ComplexExpression>(&dynamics[i]);
    if(!flagExpr) continue;
    auto [head, statics, args, spans] = std::move(*flagExpr).decompose();
    bool enabled = true;
    if(!args.empty()) {
      if(auto* iv = std::get_if<int64_t>(&args[0]))  enabled = (*iv != 0);
      else if(auto* dv = std::get_if<double>(&args[0])) enabled = (*dv != 0.0);
    }
    auto const& name = head.getName();
    if(name == "apply_scale_and_offset")          flags.apply_scale_and_offset = enabled;
    else if(name == "expand_components")           flags.expand_components = enabled;
    else if(name == "expand_sub_fields")           flags.expand_sub_fields = enabled;
    else if(name == "convert_datetimes_to_dates")  flags.convert_datetimes_to_dates = enabled;
    else if(name == "merge_heart_rates")           flags.merge_heart_rates = enabled;
    else if(name == "enable_crc_check")            flags.enable_crc_check = enabled;
  }
  return flags;
}

// ── fit_skip config parsing ────────────────────────────────────────────────────

// Builds a ParseConfig from dynamics[from..end]. Looks for a List_ of field
// name symbols or strings to use as the column projection for msgType.
static fit_skip::ParseConfig parseFitSkipConfig(ExpressionArguments& dynamics,
                                                 const std::string& msgType, size_t from) {
  fit_skip::ParseConfig cfg;
  cfg.wanted_msgs.insert(msgType);
  for(size_t i = from; i < dynamics.size(); ++i) {
    auto* listExpr = std::get_if<ComplexExpression>(&dynamics[i]);
    if(!listExpr || listExpr->getHead().getName() != "List") continue;
    auto [head, statics, args, spans] = std::move(*listExpr).decompose();
    auto& fieldSet = cfg.wanted_fields[msgType];
    for(auto& arg : args) {
      if(auto* sym = std::get_if<Symbol>(&arg))        fieldSet.insert(sym->getName());
      else if(auto* str = std::get_if<std::string>(&arg)) fieldSet.insert(*str);
    }
    break;
  }
  return cfg;
}

// ── Metadata helpers ───────────────────────────────────────────────────────────

static double firstRawVal(const fit_skip::RawTable& tbl, const std::string& colName) {
  auto it = tbl.columns.find(colName);
  if(it == tbl.columns.end() || tbl.row_count == 0)
    return std::numeric_limits<double>::quiet_NaN();
  auto const& col = it->second;
  if(!col.is_double || col.doubles.empty() || col.is_null[0])
    return std::numeric_limits<double>::quiet_NaN();
  return col.doubles[0];
}

static void addScaled(ColumnBuilder& col, double value, double scale = 1.0) {
  std::isnan(value) ? col.add(Symbol("NULL")) : col.add(value * scale);
}

static void addFitTimestamp(ColumnBuilder& col, double value) {
  (std::isnan(value) || value < kFitDateTimeMin) ? col.add(Symbol("NULL"))
                                                  : col.add(value + kFitEpochOffset);
}

// ── RawTable helpers ───────────────────────────────────────────────────────────

static void mergeRawTable(fit_skip::RawTable& dst, const fit_skip::RawTable& src) {
  for(auto const& [name, srcCol] : src.columns) {
    auto& dstCol = dst.columns[name];
    dstCol.pad_to(dst.row_count);
    if(srcCol.is_double) {
      dstCol.set_double_type();
      for(size_t r = 0; r < src.row_count; ++r)
        srcCol.is_null[r] ? dstCol.push_null() : dstCol.push_double(srcCol.doubles[r]);
    } else {
      dstCol.set_string_type();
      for(size_t r = 0; r < src.row_count; ++r)
        srcCol.is_null[r] ? dstCol.push_null() : dstCol.push_string(srcCol.strings[r]);
    }
  }
  dst.row_count += src.row_count;
}

static Expression rawTableToBoss(fit_skip::RawTable& tbl) {
  for(auto& [_, col] : tbl.columns)
    col.pad_to(tbl.row_count);

  auto columns = ExpressionArguments{};
  for(auto& [name, col] : tbl.columns) {
    ColumnBuilder builder;
    if(col.is_double) {
      for(size_t r = 0; r < tbl.row_count; ++r)
        col.is_null[r] ? builder.add(Symbol("NULL")) : builder.add(col.doubles[r]);
    } else {
      for(size_t r = 0; r < tbl.row_count; ++r)
        col.is_null[r] ? builder.add(Symbol("NULL")) : builder.add(col.strings[r]);
    }
    columns.emplace_back(makeColumn(name, std::move(builder)));
  }
  return makeTable(std::move(columns));
}

// ── Metadata table builder ─────────────────────────────────────────────────────

// Scans one file or every .fit file in a directory and returns a Table with
// one row per file containing activity metadata from file_id + session messages.
static Expression buildMetadataTable(const std::string& path) {
  fit_skip::ParseConfig cfg;
  cfg.wanted_msgs.insert("file_id");
  cfg.wanted_msgs.insert("session");

  ColumnBuilder file, timeCreated, startTime, sport, elapsedSecs, distanceMeters, calories;

  for(auto const& filePath : collectFitFiles(path)) {
    MmapView view(filePath);
    if(!view.ok())    return "LoadFIT::error: "s + view.error;
    if(view.empty())  continue;

    auto result = fit_skip::parse(view.data, view.size, cfg);
    if(!result.error.empty()) return "LoadFIT::error: "s + result.error;

    double timeCreatedVal = std::numeric_limits<double>::quiet_NaN();
    if(auto it = result.tables.find("file_id"); it != result.tables.end())
      timeCreatedVal = firstRawVal(it->second, "time_created");

    double sportVal    = std::numeric_limits<double>::quiet_NaN();
    double startTimeVal = std::numeric_limits<double>::quiet_NaN();
    double elapsedVal  = std::numeric_limits<double>::quiet_NaN();
    double distanceVal = std::numeric_limits<double>::quiet_NaN();
    double caloriesVal = std::numeric_limits<double>::quiet_NaN();
    if(auto it = result.tables.find("session"); it != result.tables.end()) {
      auto const& session = it->second;
      sportVal     = firstRawVal(session, "sport");
      startTimeVal = firstRawVal(session, "start_time");
      elapsedVal   = firstRawVal(session, "total_elapsed_time");
      distanceVal  = firstRawVal(session, "total_distance");
      caloriesVal  = firstRawVal(session, "total_calories");
    }

    file.add(filePath.string());
    addFitTimestamp(timeCreated, timeCreatedVal);
    addFitTimestamp(startTime, startTimeVal);
    std::isnan(sportVal) ? sport.add(Symbol("NULL"))
                         : sport.add(fitSportName(static_cast<int>(sportVal)));
    addScaled(elapsedSecs,    elapsedVal,  1.0 / 1000.0); // ms → s
    addScaled(distanceMeters, distanceVal, 1.0 / 100.0);  // cm → m
    addScaled(calories,       caloriesVal);
  }

  auto columns = ExpressionArguments{};
  columns.emplace_back(makeColumn("file",               std::move(file)));
  columns.emplace_back(makeColumn("time_created",       std::move(timeCreated)));
  columns.emplace_back(makeColumn("start_time",         std::move(startTime)));
  columns.emplace_back(makeColumn("sport",              std::move(sport)));
  columns.emplace_back(makeColumn("total_elapsed_time", std::move(elapsedSecs)));
  columns.emplace_back(makeColumn("total_distance",     std::move(distanceMeters)));
  columns.emplace_back(makeColumn("total_calories",     std::move(calories)));
  return makeTable(std::move(columns));
}

// ── Operator dispatch ──────────────────────────────────────────────────────────

static Expression evaluate(Expression&& e) {
  using sentinel::Any_;
  using sentinel::Symbol_;
  using sentinel::AnySequence_;
  return std::move(e)
    <"LoadFIT"_(Any_) >= Recurse(evaluate)>
    [](auto, auto dynamics, auto) -> Expression {
      return buildMetadataTable(std::get<std::string>(dynamics.at(0)));
    }
    <"LoadFIT"_(Any_, Symbol_, AnySequence_) >= Recurse(evaluate)>
    [](auto, auto dynamics, auto) -> Expression {
      auto const& path    = std::get<std::string>(dynamics.at(0));
      auto const& msgType = std::get<Symbol>(dynamics.at(1)).getName();
      auto flags = parseFlags(dynamics, 2);

      struct FitListener : fit::MesgListener {
        std::string_view targetType;
        ParseFlags flags;
        std::unordered_map<std::string, MessageTable> tables;
        std::vector<HrPoint> hrPoints;
        std::vector<double> rowTimestamps;

        void addNumericValue(ColumnBuilder& col, fit::FieldBase* field,
                             FIT_UINT16 subFieldIndex = FIT_SUBFIELD_INDEX_MAIN_FIELD) {
          col.add(flags.apply_scale_and_offset ? field->GetFLOAT64Value(0, subFieldIndex)
                                               : field->GetRawValue());
        }

        void OnMesg(fit::Mesg& mesg) override {
          if(flags.merge_heart_rates && mesg.GetName() == "hr") {
            auto* tsField  = mesg.GetField("timestamp");
            auto* bpmField = mesg.GetField("filtered_bpm");
            if(tsField && tsField->IsValueValid() && bpmField && bpmField->IsValueValid())
              hrPoints.push_back({tsField->GetFLOAT64Value(), bpmField->GetFLOAT64Value()});
            return;
          }
          if(mesg.GetName() != targetType) return;

          auto& table = tables[mesg.GetName()];

          if(flags.merge_heart_rates) {
            auto* tsField = mesg.GetField("timestamp");
            rowTimestamps.push_back(
                (tsField && tsField->IsValueValid()) ? tsField->GetFLOAT64Value() : -1.0);
          }

          for(FIT_UINT16 i = 0; i < (FIT_UINT16)mesg.GetNumFields(); i++) {
            auto* field = mesg.GetFieldByIndex(i);
            if(!field || !field->IsValid() || !field->IsValueValid()) continue;

            auto& col = table.padded(field->GetName());
            if(field->GetName() == "sport") {
              col.add(fitSportName(static_cast<int>(field->GetFLOAT64Value())));
            } else {
              switch(field->GetType()) {
              case FIT_BASE_TYPE_STRING: {
                auto const& wstr = field->GetSTRINGValue();
                col.add(std::string(wstr.begin(), wstr.end()));
                break;
              }
              case FIT_BASE_TYPE_ENDIAN_FLAG:
              case FIT_BASE_TYPE_RESERVED:
              case FIT_BASE_TYPE_NUM_MASK:
                break;
              default: {
                double rawVal = flags.apply_scale_and_offset ? field->GetFLOAT64Value()
                                                             : field->GetRawValue();
                if(flags.convert_datetimes_to_dates && isDateTimeField(field->GetName()) &&
                   rawVal >= kFitDateTimeMin)
                  col.add(rawVal + kFitEpochOffset);
                else
                  col.add(rawVal);
                break;
              }
              }
            }

            if(flags.expand_sub_fields && field->GetNumSubFields() > 0) {
              FIT_UINT16 activeSubField = mesg.GetActiveSubFieldIndexByFieldIndex(i);
              if(activeSubField != FIT_SUBFIELD_INDEX_MAIN_FIELD) {
                auto const* subFieldProfile = field->GetSubField(activeSubField);
                if(subFieldProfile && field->IsValueValid(0, activeSubField))
                  addNumericValue(table.padded(subFieldProfile->name), field, activeSubField);
              }
            }
          }
          table.finishRow();
        }
      } listener;
      listener.targetType = msgType;
      listener.flags = flags;

      for(auto const& filePath : collectFitFiles(path)) {
        auto file = std::fstream(filePath, std::ios::in | std::ios::binary);
        if(!file.is_open()) return "LoadFIT::error: cannot open file: "s + filePath.string();
        try {
          fit::Decode decode;
          if(!flags.expand_components) decode.SuppressComponentExpansion();
          if(!flags.enable_crc_check)  decode.SkipHeader();
          decode.Read(file, listener);
        } catch(fit::RuntimeException const& e) { return "LoadFIT::error: "s + e.what(); }
        catch(...)                               { return "LoadFIT::error: unknown exception during decode"s; }
      }

      if(flags.merge_heart_rates && !listener.hrPoints.empty()) {
        if(auto tableIt = listener.tables.find(msgType); tableIt != listener.tables.end()) {
          auto& table = tableIt->second;
          std::ranges::sort(listener.hrPoints, {}, &HrPoint::timestamp);
          auto& hrCol = table.padded("heart_rate");
          for(size_t row = 0; row < table.rowCount; ++row) {
            while(hrCol.committedRows < row)
              hrCol.add(Symbol("NULL"));
            double ts = (row < listener.rowTimestamps.size()) ? listener.rowTimestamps[row] : -1.0;
            if(ts < 0.0) { hrCol.add(Symbol("NULL")); continue; }
            auto it = std::ranges::lower_bound(listener.hrPoints, ts, {}, &HrPoint::timestamp);
            double bpm;
            if(it == listener.hrPoints.end())        bpm = listener.hrPoints.back().bpm;
            else if(it == listener.hrPoints.begin()) bpm = it->bpm;
            else {
              auto prev = std::prev(it);
              bpm = (ts - prev->timestamp <= it->timestamp - ts) ? prev->bpm : it->bpm;
            }
            hrCol.add(bpm);
          }
          while(hrCol.committedRows < table.rowCount)
            hrCol.add(Symbol("NULL"));
        }
      }

      auto tableIt = listener.tables.find(msgType);
      if(tableIt == listener.tables.end())
        return "LoadFIT::error: message type not found: "s + msgType;

      auto columns = ExpressionArguments{};
      for(auto& [name, col] : tableIt->second.columns)
        columns.emplace_back(makeColumn(name, std::move(col)));
      return makeTable(std::move(columns));
    }
    <"LoadFITFast"_(Any_) >= Recurse(evaluate)>
    [](auto, auto dynamics, auto) -> Expression {
      return buildMetadataTable(std::get<std::string>(dynamics.at(0)));
    }
    <"LoadFITFast"_(Any_, Symbol_, AnySequence_) >= Recurse(evaluate)>
    [](auto, auto dynamics, auto) -> Expression {
      auto const& path    = std::get<std::string>(dynamics.at(0));
      auto const& msgType = std::get<Symbol>(dynamics.at(1)).getName();
      auto cfg = parseFitSkipConfig(dynamics, msgType, 2);

      fit_skip::RawTable merged;
      for(auto const& filePath : collectFitFiles(path)) {
        MmapView view(filePath);
        if(!view.ok())   return "LoadFITFast::error: "s + view.error;
        if(view.empty()) continue;

        auto result = fit_skip::parse(view.data, view.size, cfg);
        if(!result.error.empty()) return "LoadFITFast::error: "s + result.error;

        if(auto it = result.tables.find(msgType); it != result.tables.end())
          mergeRawTable(merged, it->second);
      }

      if(merged.row_count == 0)
        return "LoadFITFast::error: message type not found: "s + msgType;

      return rawTableToBoss(merged);
    }
    < Any_ >= Recurse(evaluate);
}

} // namespace

extern "C" BOSSExpression* evaluate(BOSSExpression* e) {
  return new BOSSExpression{.delegate = evaluate(std::move(e->delegate))};
}
