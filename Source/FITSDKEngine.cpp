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

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <thread>
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
  case FIT_SPORT_GENERIC:
    return Symbol("Generic");
  case FIT_SPORT_RUNNING:
    return Symbol("Running");
  case FIT_SPORT_CYCLING:
    return Symbol("Cycling");
  case FIT_SPORT_TRANSITION:
    return Symbol("Transition");
  case FIT_SPORT_FITNESS_EQUIPMENT:
    return Symbol("FitnessEquipment");
  case FIT_SPORT_SWIMMING:
    return Symbol("Swimming");
  case FIT_SPORT_BASKETBALL:
    return Symbol("Basketball");
  case FIT_SPORT_SOCCER:
    return Symbol("Soccer");
  case FIT_SPORT_TENNIS:
    return Symbol("Tennis");
  case FIT_SPORT_AMERICAN_FOOTBALL:
    return Symbol("AmericanFootball");
  case FIT_SPORT_TRAINING:
    return Symbol("Training");
  case FIT_SPORT_WALKING:
    return Symbol("Walking");
  case FIT_SPORT_CROSS_COUNTRY_SKIING:
    return Symbol("CrossCountrySkiing");
  case FIT_SPORT_ALPINE_SKIING:
    return Symbol("AlpineSkiing");
  case FIT_SPORT_SNOWBOARDING:
    return Symbol("Snowboarding");
  case FIT_SPORT_ROWING:
    return Symbol("Rowing");
  case FIT_SPORT_MOUNTAINEERING:
    return Symbol("Mountaineering");
  case FIT_SPORT_HIKING:
    return Symbol("Hiking");
  case FIT_SPORT_MULTISPORT:
    return Symbol("Multisport");
  case FIT_SPORT_PADDLING:
    return Symbol("Paddling");
  case FIT_SPORT_FLYING:
    return Symbol("Flying");
  case FIT_SPORT_E_BIKING:
    return Symbol("EBiking");
  case FIT_SPORT_MOTORCYCLING:
    return Symbol("Motorcycling");
  case FIT_SPORT_BOATING:
    return Symbol("Boating");
  case FIT_SPORT_DRIVING:
    return Symbol("Driving");
  case FIT_SPORT_GOLF:
    return Symbol("Golf");
  case FIT_SPORT_HANG_GLIDING:
    return Symbol("HangGliding");
  case FIT_SPORT_HORSEBACK_RIDING:
    return Symbol("HorsebackRiding");
  case FIT_SPORT_HUNTING:
    return Symbol("Hunting");
  case FIT_SPORT_FISHING:
    return Symbol("Fishing");
  case FIT_SPORT_INLINE_SKATING:
    return Symbol("InlineSkating");
  case FIT_SPORT_ROCK_CLIMBING:
    return Symbol("RockClimbing");
  case FIT_SPORT_SAILING:
    return Symbol("Sailing");
  case FIT_SPORT_ICE_SKATING:
    return Symbol("IceSkating");
  case FIT_SPORT_SKY_DIVING:
    return Symbol("SkyDiving");
  case FIT_SPORT_SNOWSHOEING:
    return Symbol("Snowshoeing");
  case FIT_SPORT_SNOWMOBILING:
    return Symbol("Snowmobiling");
  case FIT_SPORT_STAND_UP_PADDLEBOARDING:
    return Symbol("StandUpPaddleboarding");
  case FIT_SPORT_SURFING:
    return Symbol("Surfing");
  case FIT_SPORT_WAKEBOARDING:
    return Symbol("Wakeboarding");
  case FIT_SPORT_WATER_SKIING:
    return Symbol("WaterSkiing");
  case FIT_SPORT_KAYAKING:
    return Symbol("Kayaking");
  case FIT_SPORT_RAFTING:
    return Symbol("Rafting");
  case FIT_SPORT_WINDSURFING:
    return Symbol("Windsurfing");
  case FIT_SPORT_KITESURFING:
    return Symbol("Kitesurfing");
  case FIT_SPORT_TACTICAL:
    return Symbol("Tactical");
  case FIT_SPORT_JUMPMASTER:
    return Symbol("Jumpmaster");
  case FIT_SPORT_BOXING:
    return Symbol("Boxing");
  case FIT_SPORT_FLOOR_CLIMBING:
    return Symbol("FloorClimbing");
  case FIT_SPORT_BASEBALL:
    return Symbol("Baseball");
  case FIT_SPORT_DIVING:
    return Symbol("Diving");
  case FIT_SPORT_SHOOTING:
    return Symbol("Shooting");
  case FIT_SPORT_WINTER_SPORT:
    return Symbol("WinterSport");
  case FIT_SPORT_GRINDING:
    return Symbol("Grinding");
  case FIT_SPORT_HIIT:
    return Symbol("Hiit");
  case FIT_SPORT_VIDEO_GAMING:
    return Symbol("VideoGaming");
  case FIT_SPORT_RACKET:
    return Symbol("Racket");
  case FIT_SPORT_WHEELCHAIR_PUSH_WALK:
    return Symbol("WheelchairPushWalk");
  case FIT_SPORT_WHEELCHAIR_PUSH_RUN:
    return Symbol("WheelchairPushRun");
  case FIT_SPORT_MEDITATION:
    return Symbol("Meditation");
  case FIT_SPORT_PARA_SPORT:
    return Symbol("ParaSport");
  case FIT_SPORT_DISC_GOLF:
    return Symbol("DiscGolf");
  case FIT_SPORT_TEAM_SPORT:
    return Symbol("TeamSport");
  case FIT_SPORT_CRICKET:
    return Symbol("Cricket");
  case FIT_SPORT_RUGBY:
    return Symbol("Rugby");
  case FIT_SPORT_HOCKEY:
    return Symbol("Hockey");
  case FIT_SPORT_LACROSSE:
    return Symbol("Lacrosse");
  case FIT_SPORT_VOLLEYBALL:
    return Symbol("Volleyball");
  case FIT_SPORT_WATER_TUBING:
    return Symbol("WaterTubing");
  case FIT_SPORT_WAKESURFING:
    return Symbol("Wakesurfing");
  case FIT_SPORT_WATER_SPORT:
    return Symbol("WaterSport");
  case FIT_SPORT_ARCHERY:
    return Symbol("Archery");
  case FIT_SPORT_MIXED_MARTIAL_ARTS:
    return Symbol("MixedMartialArts");
  case FIT_SPORT_MOTOR_SPORTS:
    return Symbol("MotorSports");
  case FIT_SPORT_SNORKELING:
    return Symbol("Snorkeling");
  case FIT_SPORT_DANCE:
    return Symbol("Dance");
  case FIT_SPORT_JUMP_ROPE:
    return Symbol("JumpRope");
  case FIT_SPORT_POOL_APNEA:
    return Symbol("PoolApnea");
  case FIT_SPORT_MOBILITY:
    return Symbol("Mobility");
  case FIT_SPORT_GEOCACHING:
    return Symbol("Geocaching");
  case FIT_SPORT_CANOEING:
    return Symbol("Canoeing");
  case FIT_SPORT_ALL:
    return Symbol("All");
  default:
    return Symbol("Sport" + std::to_string(sport));
  }
}

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
    using VecType = std::vector<std::decay_t<V>>;
    if(!std::holds_alternative<VecType>(buffer)) {
      flush();
      buffer = VecType {};
    }
    std::get<VecType>(buffer).push_back(std::forward<V>(value));
    committedRows++;
  }

  void appendFrom(ColumnBuilder&& other) {
    other.flush();
    spans.reserve(spans.size() + other.spans.size());
    for(auto& s : other.spans)
      spans.push_back(std::move(s));
    committedRows += other.committedRows;
  }

  void appendNulls(size_t n) {
    if(n == 0)
      return;
    flush();
    spans.push_back(Span<Symbol>(std::vector<Symbol>(n, Symbol("NULL"))));
    committedRows += n;
  }

  ExpressionSpanArguments build() && {
    flush();
    return std::move(spans);
  }
};

struct MessageTable {
  size_t rowCount = 0;
  std::map<std::string, ColumnBuilder> columns;
};

// Parsing options exposed as symbolic flags on the Load operator.
// Each flag is passed as a ComplexExpression: "flag_name"_(0_or_1).
// Omitting a flag leaves its default in effect.
struct ParseFlags {
  bool apply_scale_and_offset = true;     // use GetFLOAT64Value (scaled); false → GetRawValue
  bool expand_components = true;          // let Decode expand component fields
  bool expand_sub_fields = true;          // expose active sub-field as a separate column
  bool convert_datetimes_to_dates = true; // shift FIT timestamps to Unix epoch
  bool merge_heart_rates = false;         // interpolate HR from hr messages into target table
  bool enable_crc_check = true;           // validate file CRC; false → SkipHeader
  std::optional<double> timestamp_min;    // skip files whose record range ends before this
  std::optional<double> timestamp_max;    // skip files whose record range starts after this
};

// Seconds between the FIT epoch (Dec 31 1989 00:00 UTC) and the Unix epoch.
static constexpr double kFitEpochOffset = 631065600.0;

// FIT timestamps below this value are relative (time-of-day), not absolute.
static constexpr double kFitDateTimeMin = static_cast<double>(0x10000000u);

static bool isDateTimeField(std::string_view name) {
  return name == "timestamp" || name == "local_timestamp";
}

struct HrPoint {
  double timestamp;
  double bpm;
};

} // namespace

static std::optional<double> tryAsDouble(Expression const& e) {
  return std::visit(
      [](auto const& v) -> std::optional<double> {
        if constexpr(std::is_arithmetic_v<std::decay_t<decltype(v)>>)
          return static_cast<double>(v);
        else
          return std::nullopt;
      },
      e);
}

static ParseFlags parseFlagsFrom(ExpressionArguments& dynamics, size_t startIndex) {
  ParseFlags flags;
  for(size_t i = startIndex; i < dynamics.size(); ++i) {
    auto* flagExpr = std::get_if<ComplexExpression>(&dynamics[i]);
    if(!flagExpr)
      continue;
    auto [flagHead, flagStatics, flagArgs, flagSpans] = std::move(*flagExpr).decompose();
    auto const& flagName = flagHead.getName();

    if(flagName == "timestamp_min" || flagName == "timestamp_max") {
      auto value = flagArgs.empty() ? std::nullopt : tryAsDouble(flagArgs[0]);
      if(!value)
        continue;
      if(flagName == "timestamp_min")
        flags.timestamp_min = *value;
      else
        flags.timestamp_max = *value;
      continue;
    }

    bool value = true;
    if(!flagArgs.empty()) {
      if(auto const v = tryAsDouble(flagArgs[0]))
        value = (*v != 0.0);
    }
    if(flagName == "apply_scale_and_offset")
      flags.apply_scale_and_offset = value;
    else if(flagName == "expand_components")
      flags.expand_components = value;
    else if(flagName == "expand_sub_fields")
      flags.expand_sub_fields = value;
    else if(flagName == "convert_datetimes_to_dates")
      flags.convert_datetimes_to_dates = value;
    else if(flagName == "merge_heart_rates")
      flags.merge_heart_rates = value;
    else if(flagName == "enable_crc_check")
      flags.enable_crc_check = value;
  }
  return flags;
}

struct TimestampBounds {
  std::optional<double> minimum;
  std::optional<double> maximum;
};

static TimestampBounds extractTimestampBounds(Expression const& predicate) {
  TimestampBounds bounds;
  auto const* ce = std::get_if<ComplexExpression>(&predicate);
  if(!ce)
    return bounds;
  auto const& head = ce->getHead().getName();
  auto const& dynamics = ce->getDynamicArguments();

  if(head == "And") {
    for(auto const& child : dynamics) {
      auto childBounds = extractTimestampBounds(child);
      if(childBounds.minimum)
        bounds.minimum =
            bounds.minimum ? std::max(*bounds.minimum, *childBounds.minimum) : *childBounds.minimum;
      if(childBounds.maximum)
        bounds.maximum =
            bounds.maximum ? std::min(*bounds.maximum, *childBounds.maximum) : *childBounds.maximum;
    }
    return bounds;
  }

  if(dynamics.size() != 2)
    return bounds;

  // Normalise so timestamp is always on the left: if it's on the right, swap and flip.
  auto const* leftSym = std::get_if<Symbol>(&dynamics[0]);
  bool reversed = false;
  if(!(leftSym && leftSym->getName() == "timestamp")) {
    auto const* rightSym = std::get_if<Symbol>(&dynamics[1]);
    if(!(rightSym && rightSym->getName() == "timestamp"))
      return bounds;
    reversed = true;
  }
  auto const& literalSide = dynamics[reversed ? 0 : 1];
  auto value = tryAsDouble(literalSide);
  if(!value)
    return bounds;

  auto setsMin = (head == "GreaterEqual" || head == "Greater");
  auto setsMax = (head == "LessEqual" || head == "Less");
  if(reversed)
    std::swap(setsMin, setsMax);
  if(setsMin)
    bounds.minimum = *value;
  else if(setsMax)
    bounds.maximum = *value;
  else if(head == "Equal") {
    bounds.minimum = *value;
    bounds.maximum = *value;
  }
  return bounds;
}

// Walk the table-source tree (descending through Project, OrderBy, Slice, etc. via their
// first dynamic arg) and append (timestamp_min N) / (timestamp_max M) flags to the Load.
// Bails on Project rewrites that rebind the `timestamp` column via (As ... timestamp).
static Expression injectBoundsIntoLoad(Expression&& expr, TimestampBounds const& bounds) {
  if(!bounds.minimum && !bounds.maximum)
    return std::move(expr);
  auto* ce = std::get_if<ComplexExpression>(&expr);
  if(!ce)
    return std::move(expr);
  auto headName = ce->getHead().getName();

  if(headName == "Load") {
    auto [loadHead, loadStatics, loadArgs, loadSpans] = std::move(*ce).decompose();
    if(bounds.minimum)
      loadArgs.emplace_back("timestamp_min"_(*bounds.minimum));
    if(bounds.maximum)
      loadArgs.emplace_back("timestamp_max"_(*bounds.maximum));
    return ComplexExpression(std::move(loadHead), std::move(loadStatics), std::move(loadArgs),
                             std::move(loadSpans));
  }

  if(headName == "Project") {
    auto const& dynamics = ce->getDynamicArguments();
    for(size_t i = 1; i < dynamics.size(); ++i) {
      auto const* asExpr = std::get_if<ComplexExpression>(&dynamics[i]);
      if(!asExpr || asExpr->getHead().getName() != "As")
        continue;
      auto const& asArgs = asExpr->getDynamicArguments();
      if(asArgs.empty())
        continue;
      auto const* aliasSym = std::get_if<Symbol>(&asArgs.back());
      if(aliasSym && aliasSym->getName() == "timestamp")
        return std::move(expr);
    }
  }

  auto [opHead, opStatics, opArgs, opSpans] = std::move(*ce).decompose();
  if(!opArgs.empty())
    opArgs[0] = injectBoundsIntoLoad(std::move(opArgs[0]), bounds);
  return ComplexExpression(std::move(opHead), std::move(opStatics), std::move(opArgs),
                           std::move(opSpans));
}

static bool containsFilter(Expression const& e) {
  auto const* ce = std::get_if<ComplexExpression>(&e);
  if(!ce)
    return false;
  if(ce->getHead().getName() == "Filter")
    return true;
  for(auto const& child : ce->getDynamicArguments())
    if(containsFilter(child))
      return true;
  return false;
}

static Expression applyTimestampPushdown(Expression&& e) {
  using sentinel::Any_;
  return std::move(e) //
         <"Filter"_(Any_, Any_) >=
          Recurse(applyTimestampPushdown)>[](auto, auto dynamics,
                                             auto) -> Expression {
           auto bounds = extractTimestampBounds(dynamics[1]);
           if(bounds.minimum || bounds.maximum)
             dynamics[0] = injectBoundsIntoLoad(std::move(dynamics[0]), bounds);
           return "Filter"_(std::move(dynamics));
         } //
                                                          < Any_ >= Recurse(applyTimestampPushdown);
}

struct LoadPlan {
  std::vector<std::filesystem::path> fitFiles;
  bool hasOtherFiles = false;
};

static LoadPlan planLoad(std::string const& path) {
  LoadPlan plan;
  std::error_code ec;
  std::filesystem::path p(path);
  if(std::filesystem::is_directory(p, ec)) {
    for(auto const& entry : std::filesystem::directory_iterator(p, ec)) {
      if(entry.path().extension() == ".fit")
        plan.fitFiles.push_back(entry.path());
      else
        plan.hasOtherFiles = true;
    }
    std::ranges::sort(plan.fitFiles);
    return plan;
  }
  if(p.extension() == ".fit")
    plan.fitFiles.push_back(p);
  return plan;
}

static Expression wrapForMixedDirectory(std::string path, Expression fitTable) {
  return "Union"_("Load"_(std::move(path)), std::move(fitTable));
}

struct FileTimeRange {
  double startTime; // record-timestamp lower bound in the same space as emitted timestamps
  double endTime;   // record-timestamp upper bound
};

// Read a FIT file's session message to estimate its record-timestamp range, then Pause()
// the decoder so we don't pay for the full scan. Returns nullopt if the file can't be
// opened or has no session metadata.
static std::optional<FileTimeRange> readFileTimeRange(std::filesystem::path const& filePath,
                                                      ParseFlags const& flags) {
  auto file = std::fstream(filePath, std::ios::in | std::ios::binary);
  if(!file.is_open())
    return std::nullopt;

  struct : fit::MesgListener {
    fit::Decode* decode = nullptr;
    bool fileIdSeen = false;
    bool sessionSeen = false;
    std::optional<double> startTime;
    std::optional<double> totalElapsedTime;
    ParseFlags flags;
    void OnMesg(fit::Mesg& mesg) override {
      if(mesg.GetName() == "file_id" && !fileIdSeen) {
        fileIdSeen = true;
      } else if(mesg.GetName() == "session" && !sessionSeen) {
        auto* st = mesg.GetField("start_time");
        if(st && st->IsValueValid())
          startTime = flags.apply_scale_and_offset ? st->GetFLOAT64Value() : st->GetRawValue();
        auto* tet = mesg.GetField("total_elapsed_time");
        if(tet && tet->IsValueValid())
          totalElapsedTime =
              flags.apply_scale_and_offset ? tet->GetFLOAT64Value() : tet->GetRawValue();
        sessionSeen = true;
      }
      if(fileIdSeen && sessionSeen)
        decode->Pause();
    }
  } listener;
  listener.flags = flags;

  try {
    fit::Decode decode;
    listener.decode = &decode;
    if(!flags.expand_components)
      decode.SuppressComponentExpansion();
    if(!flags.enable_crc_check)
      decode.SkipHeader();
    decode.Read(file, listener);
  } catch(...) {
    return std::nullopt;
  }

  if(!listener.startTime)
    return std::nullopt;
  double recordStart = *listener.startTime;
  if(flags.convert_datetimes_to_dates && recordStart >= kFitDateTimeMin)
    recordStart += kFitEpochOffset;
  double recordEnd = recordStart + listener.totalElapsedTime.value_or(0.0);
  return FileTimeRange {recordStart, recordEnd};
}

// Files with unreadable session metadata are kept (conservative).
static std::vector<std::filesystem::path>
filterFilesByTimeRange(std::vector<std::filesystem::path>&& files, ParseFlags const& flags) {
  if(!flags.timestamp_min && !flags.timestamp_max)
    return std::move(files);
  std::vector<std::filesystem::path> filtered;
  filtered.reserve(files.size());
  for(auto& path : files) {
    auto range = readFileTimeRange(path, flags);
    if(!range) {
      filtered.push_back(std::move(path));
      continue;
    }
    if(flags.timestamp_max && range->startTime > *flags.timestamp_max)
      continue;
    if(flags.timestamp_min && range->endTime < *flags.timestamp_min)
      continue;
    filtered.push_back(std::move(path));
  }
  return filtered;
}

static Expression evaluate(Expression&& e) {
  using sentinel::Any_;
  using sentinel::AnySequence_;
  using sentinel::Symbol_;
  return std::move(e) //
         <"Load"_(Any_, Symbol_, AnySequence_) >= Recurse(evaluate)>[](auto, auto dynamics,
                                                                       auto) -> Expression {
           auto const& path = std::get<std::string>(dynamics.at(0));
           auto plan = planLoad(path);
           if(plan.fitFiles.empty())
             return "Load"_(std::move(dynamics));

           auto const& msgType = std::get<Symbol>(dynamics.at(1)).getName();
           auto flags = parseFlagsFrom(dynamics, 2);
           auto filePaths = std::move(plan.fitFiles);
           filePaths = filterFilesByTimeRange(std::move(filePaths), flags);

           struct FitDataListener : fit::MesgListener {
             std::string_view targetType;
             ParseFlags flags;
             std::unordered_map<std::string, MessageTable> tables;
             std::vector<HrPoint> hrPoints;
             std::vector<double> rowTimestamps;

             void addNumericValue(ColumnBuilder& column, fit::FieldBase* field,
                                  FIT_UINT16 subFieldIndex = FIT_SUBFIELD_INDEX_MAIN_FIELD) {
               if(flags.apply_scale_and_offset)
                 column.add(field->GetFLOAT64Value(0, subFieldIndex));
               else
                 column.add(field->GetRawValue());
             }

             void OnMesg(fit::Mesg& mesg) override {
               // Collect HR messages when merging heart rates.
               if(flags.merge_heart_rates && mesg.GetName() == "hr") {
                 auto* tsField = mesg.GetField("timestamp");
                 auto* bpmField = mesg.GetField("filtered_bpm");
                 if(tsField && tsField->IsValueValid() && bpmField && bpmField->IsValueValid())
                   hrPoints.push_back({tsField->GetFLOAT64Value(), bpmField->GetFLOAT64Value()});
                 return;
               }

               if(mesg.GetName() != targetType)
                 return;

               auto& table = tables[mesg.GetName()];

               // Stash this row's timestamp for HR interpolation later.
               if(flags.merge_heart_rates) {
                 auto* tsField = mesg.GetField("timestamp");
                 rowTimestamps.push_back(
                     (tsField && tsField->IsValueValid()) ? tsField->GetFLOAT64Value() : -1.0);
               }

               for(FIT_UINT16 i = 0; i < (FIT_UINT16)mesg.GetNumFields(); i++) {
                 auto* field = mesg.GetFieldByIndex(i);
                 if(!field || !field->IsValid() || !field->IsValueValid())
                   continue;

                 auto& column = table.columns[field->GetName()];
                 while(column.committedRows < table.rowCount)
                   column.add(Symbol("NULL"));

                 if(field->GetName() == "sport") {
                   column.add(fitSportName(static_cast<int>(field->GetFLOAT64Value())));
                 } else {
                   switch(field->GetType()) {
                   case FIT_BASE_TYPE_STRING: {
                     auto const& wstr = field->GetSTRINGValue();
                     column.add(std::string(wstr.begin(), wstr.end()));
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
                       column.add(rawVal + kFitEpochOffset);
                     else
                       column.add(rawVal);
                     break;
                   }
                   }
                 }

                 // Expose the active sub-field as its own column when requested.
                 if(flags.expand_sub_fields && field->GetNumSubFields() > 0) {
                   FIT_UINT16 activeSubField = mesg.GetActiveSubFieldIndexByFieldIndex(i);
                   if(activeSubField != FIT_SUBFIELD_INDEX_MAIN_FIELD) {
                     auto const* subFieldProfile = field->GetSubField(activeSubField);
                     if(subFieldProfile && field->IsValueValid(0, activeSubField)) {
                       std::string subFieldName(subFieldProfile->name);
                       auto& sfColumn = table.columns[subFieldName];
                       while(sfColumn.committedRows < table.rowCount)
                         sfColumn.add(Symbol("NULL"));
                       addNumericValue(sfColumn, field, activeSubField);
                     }
                   }
                 }
               }

               table.rowCount++;
               for(auto& [_, column] : table.columns)
                 if(column.committedRows < table.rowCount)
                   column.add(Symbol("NULL"));
             }
           };

           unsigned int hwc = std::thread::hardware_concurrency();
           if(hwc == 0)
             hwc = 1;
           unsigned int numWorkers =
               std::min<unsigned int>(hwc, static_cast<unsigned int>(filePaths.size()));

           std::vector<FitDataListener> listeners(numWorkers);
           for(auto& l : listeners) {
             l.targetType = msgType;
             l.flags = flags;
           }
           std::vector<std::optional<std::string>> workerErrors(numWorkers);

           {
             std::atomic<size_t> nextFileIdx {0};
             std::vector<std::thread> workers;
             workers.reserve(numWorkers);
             for(unsigned int w = 0; w < numWorkers; ++w) {
               workers.emplace_back([&, w]() {
                 auto& listener = listeners[w];
                 size_t i;
                 while((i = nextFileIdx.fetch_add(1, std::memory_order_relaxed)) <
                       filePaths.size()) {
                   auto const& filePath = filePaths[i];
                   auto file = std::fstream(filePath, std::ios::in | std::ios::binary);
                   if(!file.is_open()) {
                     workerErrors[w] = "Load::error: cannot open file: "s + filePath.string();
                     return;
                   }
                   try {
                     fit::Decode decode;
                     if(!flags.expand_components)
                       decode.SuppressComponentExpansion();
                     if(!flags.enable_crc_check)
                       decode.SkipHeader();
                     decode.Read(file, listener);
                   } catch(fit::RuntimeException const& e) {
                     workerErrors[w] = "Load::error: "s + e.what();
                     return;
                   } catch(...) {
                     workerErrors[w] = "Load::error: unknown exception during decode"s;
                     return;
                   }
                 }
               });
             }
             for(auto& t : workers)
               t.join();
           }

           for(auto& e : workerErrors)
             if(e)
               return *e;

           // std::map keeps the column order alphabetical, matching single-listener output.
           std::map<std::string, ColumnBuilder> mergedColumns;
           size_t mergedRowCount = 0;
           auto msgTypeKey = std::string(msgType);
           for(auto& l : listeners) {
             auto it = l.tables.find(msgTypeKey);
             if(it == l.tables.end())
               continue;
             mergedRowCount += it->second.rowCount;
             for(auto const& [name, _] : it->second.columns) {
               // Skip native heart_rate when interpolating — HR merge will write the column.
               if(flags.merge_heart_rates && name == "heart_rate")
                 continue;
               mergedColumns.try_emplace(name);
             }
           }
           for(auto& l : listeners) {
             auto it = l.tables.find(msgTypeKey);
             if(it == l.tables.end())
               continue;
             size_t lRows = it->second.rowCount;
             for(auto& [name, target] : mergedColumns) {
               auto colIt = it->second.columns.find(name);
               if(colIt != it->second.columns.end())
                 target.appendFrom(std::move(colIt->second));
               else
                 target.appendNulls(lRows);
             }
           }

           if(mergedRowCount == 0) {
             if(filePaths.empty()) {
               auto emptyTable = "Table"_(ExpressionArguments {});
               if(plan.hasOtherFiles)
                 return wrapForMixedDirectory(std::string(path), std::move(emptyTable));
               return std::move(emptyTable);
             }
             return "Load::error: message type not found: "s + msgTypeKey;
           }

           // Merge heart rate data: nearest-neighbour interpolation by timestamp.
           // rowTimestamps stays row-aligned with mergedColumns only because each
           // listener appends its rows contiguously in the same worker-iteration order
           // used here — keep those two loops in lockstep.
           if(flags.merge_heart_rates) {
             size_t hrPointTotal = 0, rowTsTotal = 0;
             for(auto& l : listeners) {
               hrPointTotal += l.hrPoints.size();
               rowTsTotal += l.rowTimestamps.size();
             }
             std::vector<HrPoint> mergedHrPoints;
             std::vector<double> mergedRowTimestamps;
             mergedHrPoints.reserve(hrPointTotal);
             mergedRowTimestamps.reserve(rowTsTotal);
             for(auto& l : listeners) {
               mergedHrPoints.insert(mergedHrPoints.end(), l.hrPoints.begin(), l.hrPoints.end());
               mergedRowTimestamps.insert(mergedRowTimestamps.end(), l.rowTimestamps.begin(),
                                          l.rowTimestamps.end());
             }
             if(!mergedHrPoints.empty()) {
               std::ranges::sort(mergedHrPoints, {}, &HrPoint::timestamp);
               ColumnBuilder hrColumn;
               for(size_t row = 0; row < mergedRowCount; ++row) {
                 double ts = (row < mergedRowTimestamps.size()) ? mergedRowTimestamps[row] : -1.0;
                 if(ts < 0.0) {
                   hrColumn.add(Symbol("NULL"));
                   continue;
                 }
                 auto it = std::ranges::lower_bound(mergedHrPoints, ts, {}, &HrPoint::timestamp);
                 double bpm;
                 if(it == mergedHrPoints.end())
                   bpm = mergedHrPoints.back().bpm;
                 else if(it == mergedHrPoints.begin())
                   bpm = it->bpm;
                 else {
                   auto prev = std::prev(it);
                   bpm = (ts - prev->timestamp <= it->timestamp - ts) ? prev->bpm : it->bpm;
                 }
                 hrColumn.add(bpm);
               }
               mergedColumns["heart_rate"] = std::move(hrColumn);
             }
           }


           auto columns = ExpressionArguments {};
           for(auto& [name, builder] : mergedColumns)
             columns.emplace_back(
                 ComplexExpression(Symbol(name), {}, {}, std::move(builder).build()));

           auto table = "Table"_(std::move(columns));
           if(plan.hasOtherFiles)
             return wrapForMixedDirectory(std::string(path), std::move(table));
           return std::move(table);
         } < "Load"_(Any_, AnySequence_) >= Recurse(evaluate) > [](auto, auto dynamics,
                                                                   auto) -> Expression {
           auto const& path = std::get<std::string>(dynamics.at(0));
           auto plan = planLoad(path);
           if(plan.fitFiles.empty())
             return "Load"_(std::move(dynamics));

           auto flags = parseFlagsFrom(dynamics, 1);
           auto filePaths = std::move(plan.fitFiles);
           filePaths = filterFilesByTimeRange(std::move(filePaths), flags);

           struct SummaryListener : fit::MesgListener {
             ParseFlags flags;
             fit::Decode* decode = nullptr;
             bool fileIdSeen = false;
             bool sessionSeen = false;
             std::optional<double> timeCreated;
             std::optional<double> startTime;
             std::optional<Symbol> sport;
             std::optional<double> totalElapsedTime;
             std::optional<double> totalDistance;
             std::optional<double> totalCalories;

             std::optional<double> readDouble(fit::Mesg& mesg, char const* name) {
               auto* field = mesg.GetField(name);
               if(!field || !field->IsValueValid())
                 return std::nullopt;
               double value =
                   flags.apply_scale_and_offset ? field->GetFLOAT64Value() : field->GetRawValue();
               if(flags.convert_datetimes_to_dates && isDateTimeField(name) &&
                  value >= kFitDateTimeMin)
                 value += kFitEpochOffset;
               return value;
             }

             void OnMesg(fit::Mesg& mesg) override {
               if(mesg.GetName() == "file_id" && !fileIdSeen) {
                 timeCreated = readDouble(mesg, "time_created");
                 fileIdSeen = true;
               } else if(mesg.GetName() == "session" && !sessionSeen) {
                 startTime = readDouble(mesg, "start_time");
                 auto* sportField = mesg.GetField("sport");
                 if(sportField && sportField->IsValueValid())
                   sport = fitSportName(static_cast<int>(sportField->GetFLOAT64Value()));
                 totalElapsedTime = readDouble(mesg, "total_elapsed_time");
                 totalDistance = readDouble(mesg, "total_distance");
                 totalCalories = readDouble(mesg, "total_calories");
                 sessionSeen = true;
               }
               if(fileIdSeen && sessionSeen)
                 decode->Pause();
             }
           };

           ColumnBuilder fileColumn;
           ColumnBuilder timeCreatedColumn;
           ColumnBuilder startTimeColumn;
           ColumnBuilder sportColumn;
           ColumnBuilder totalElapsedTimeColumn;
           ColumnBuilder totalDistanceColumn;
           ColumnBuilder totalCaloriesColumn;

           auto addOptionalDouble = [](ColumnBuilder& column, std::optional<double> value) {
             if(value)
               column.add(*value);
             else
               column.add(Symbol("NULL"));
           };
           auto addOptionalSymbol = [](ColumnBuilder& column, std::optional<Symbol> value) {
             if(value)
               column.add(*value);
             else
               column.add(Symbol("NULL"));
           };

           for(auto const& filePath : filePaths) {
             auto file = std::fstream(filePath, std::ios::in | std::ios::binary);
             if(!file.is_open())
               return "Load::error: cannot open file: "s + filePath.string();
             SummaryListener summaryListener;
             summaryListener.flags = flags;
             try {
               fit::Decode decode;
               summaryListener.decode = &decode;
               if(!flags.expand_components)
                 decode.SuppressComponentExpansion();
               if(!flags.enable_crc_check)
                 decode.SkipHeader();
               decode.Read(file, summaryListener);
             } catch(fit::RuntimeException const& e) {
               return "Load::error: "s + e.what();
             } catch(...) {
               return "Load::error: unknown exception during decode"s;
             }

             fileColumn.add(filePath.filename().string());
             addOptionalDouble(timeCreatedColumn, summaryListener.timeCreated);
             addOptionalDouble(startTimeColumn, summaryListener.startTime);
             addOptionalSymbol(sportColumn, summaryListener.sport);
             addOptionalDouble(totalElapsedTimeColumn, summaryListener.totalElapsedTime);
             addOptionalDouble(totalDistanceColumn, summaryListener.totalDistance);
             addOptionalDouble(totalCaloriesColumn, summaryListener.totalCalories);
           }

           auto columns = ExpressionArguments {};
           columns.emplace_back(
               ComplexExpression(Symbol("file"), {}, {}, std::move(fileColumn).build()));
           columns.emplace_back(ComplexExpression(Symbol("time_created"), {}, {},
                                                  std::move(timeCreatedColumn).build()));
           columns.emplace_back(
               ComplexExpression(Symbol("start_time"), {}, {}, std::move(startTimeColumn).build()));
           columns.emplace_back(
               ComplexExpression(Symbol("sport"), {}, {}, std::move(sportColumn).build()));
           columns.emplace_back(ComplexExpression(Symbol("total_elapsed_time"), {}, {},
                                                  std::move(totalElapsedTimeColumn).build()));
           columns.emplace_back(ComplexExpression(Symbol("total_distance"), {}, {},
                                                  std::move(totalDistanceColumn).build()));
           columns.emplace_back(ComplexExpression(Symbol("total_calories"), {}, {},
                                                  std::move(totalCaloriesColumn).build()));

           auto table = "Table"_(std::move(columns));
           if(plan.hasOtherFiles)
             return wrapForMixedDirectory(std::string(path), std::move(table));
           return std::move(table);
         } < "GetEngineDescription"_() >= [](auto, auto dynamics, auto) -> Expression {
           return R"(
**Loading FIT workout data:**
- `(Load "/path/to/dir")` - summary table, one row per `.fit` file; columns: `file`, `time_created`, `start_time`, `sport`, `total_elapsed_time` (seconds), `total_distance` (metres), `total_calories`
- `(Load "/path/to/file.fit" msgtype)` - single file with message type
- `(Load "/path/to/dir" msgtype)` - all files in directory with message type

**Mixed-content directories:** If the directory also contains non-`.fit` files, the FIT engine loads the `.fit` subset and emits `(Union (Load "/path/to/dir") (Table ...))` so another engine can pick up the rest. A single non-`.fit` path or a directory with no `.fit` files is returned unchanged as `(Load ...)` for another engine to handle.

Message types (Garmin FIT protocol spec columns):
- `session` - one row per workout; per-workout aggregates: `avg/max_heart_rate`, `avg/max_speed`, `avg/max_power`, `avg_cadence`, `total_calories`, `total_distance`, `total_elapsed_time`, `total_ascent`, `num_laps`, GPS bounding box, `training_load_peak`, `sport`, `timestamp`, etc.
- `record` - one row per second; time-series GPS/sensor data: `timestamp`, `position_lat/long`, `altitude`, `heart_rate`, `cadence`, `speed`, `power`, `distance`, etc. **Large - use only for single-file analysis.**
- `lap` - per-lap summaries
- `activity` - activity-level metadata

**Path conventions:** Paths must be absolute. `~` is not expanded (BOSS does not invoke shell expansion). Use `/Users/<name>/...` on macOS, `/home/<name>/...` on Linux.

             **Unicode in filenames:** Raw UTF-8 and JSON `\uXXXX` escapes are both accepted and equivalent - use whichever your client emits naturally. The real hazard is *invisible* Unicode: filenames produced by Apple devices commonly contain U+00A0 (non-breaking space) where a regular space appears to be - for instance, between "Apple" and "Watch" in Apple Watch export filenames. NBSP renders identically to a regular space everywhere, including in the `file` column returned by the directory-summary query, so it cannot be detected by sight. If `Load` reports `cannot open file` on a path that *visually* matches the directory listing, write the suspect gaps explicitly as `\u00a0` and retry. The same caution applies to U+200B (zero-width space), U+00AD (soft hyphen), and the Unicode dash variants. Discover the row with `(Slice (OrderBy (Load ".../dir") (List (Desc time_created))) 0 1)`.

**Key patterns:**
```
; Most recent N workouts
(Slice (OrderBy (Load ".../dir") (List (Desc time_created))) 0 5)

; Per-sport average of a derived metric (derive first, then aggregate)
(GroupBy
  (Project (Load ".../dir")
    (As (Divide total_calories (Divide total_elapsed_time 60.0)) cal_per_min)
    sport)
  (Mean cal_per_min)
  sport)

; Cache a large load for reuse across calls
(Name (Load ".../dir" session) workouts)
(GroupBy (ByName workouts) (Mean total_calories) sport)
```

**Avoid returning unaggregated full-directory loads** — they exceed the result size limit. Always wrap in `GroupBy`, `Slice`, or `Filter` before returning.
)";
         } //
                                                                               < Any_ >=
                                                                               Recurse(evaluate) //
      ;
};

extern "C" BOSSExpression* evaluate(BOSSExpression* e) {
  auto delegate = std::move(e->delegate);
  if(containsFilter(delegate))
    delegate = applyTimestampPushdown(std::move(delegate));
  return new BOSSExpression {.delegate = evaluate(std::move(delegate))};
};
