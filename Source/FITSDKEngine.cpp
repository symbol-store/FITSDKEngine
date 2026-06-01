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
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <string>
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

  ExpressionSpanArguments build() && {
    flush();
    return std::move(spans);
  }
};

struct MessageTable {
  size_t rowCount = 0;
  std::map<std::string, ColumnBuilder> columns;
};

// Parsing options exposed as symbolic flags on the LoadFIT operator.
// Each flag is passed as a ComplexExpression: "flag_name"_(0_or_1).
// Omitting a flag leaves its default in effect.
struct ParseFlags {
  bool apply_scale_and_offset = true;     // use GetFLOAT64Value (scaled); false → GetRawValue
  bool expand_components = true;          // let Decode expand component fields
  bool expand_sub_fields = true;          // expose active sub-field as a separate column
  bool convert_datetimes_to_dates = true; // shift FIT timestamps to Unix epoch
  bool merge_heart_rates = false;         // interpolate HR from hr messages into target table
  bool enable_crc_check = true;           // validate file CRC; false → SkipHeader
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

static Expression evaluate(Expression&& e) {
  using sentinel::Any_;
  using sentinel::AnySequence_;
  using sentinel::Symbol_;
  return std::move(e) //
         <"LoadFIT"_(Any_, Symbol_, AnySequence_) >= Recurse(evaluate)>[](auto, auto dynamics,
                                                                          auto) -> Expression {
           auto const& path = std::get<std::string>(dynamics.at(0));
           auto const& msgType = std::get<Symbol>(dynamics.at(1)).getName();

           // Parse optional symbolic flags from dynamics[2+].
           // Each flag is expressed as flagName_(value) where value is int64_t 0/1.
           ParseFlags flags;
           for(size_t i = 2; i < dynamics.size(); ++i) {
             auto* flagExpr = std::get_if<ComplexExpression>(&dynamics[i]);
             if(!flagExpr)
               continue;
             auto [flagHead, flagStatics, flagArgs, flagSpans] = std::move(*flagExpr).decompose();
             bool value = true; // bare flag symbol with no argument defaults to true
             if(!flagArgs.empty()) {
               if(auto* iv = std::get_if<int64_t>(&flagArgs[0]))
                 value = (*iv != 0);
               else if(auto* dv = std::get_if<double>(&flagArgs[0]))
                 value = (*dv != 0.0);
             }
             auto const& flagName = flagHead.getName();
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

           auto filePaths = std::vector<std::filesystem::path> {};
           if(std::filesystem::is_directory(path)) {
             for(auto const& entry : std::filesystem::directory_iterator(path))
               if(entry.path().extension() == ".fit")
                 filePaths.push_back(entry.path());
             std::ranges::sort(filePaths);
           } else {
             filePaths.push_back(path);
           }

           struct : fit::MesgListener {
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
           } listener;
           listener.targetType = msgType;
           listener.flags = flags;

           for(auto const& filePath : filePaths) {
             auto file = std::fstream(filePath, std::ios::in | std::ios::binary);
             if(!file.is_open())
               return "LoadFIT::error: cannot open file: "s + filePath.string();
             try {
               fit::Decode decode;
               if(!flags.expand_components)
                 decode.SuppressComponentExpansion();
               if(!flags.enable_crc_check)
                 decode.SkipHeader();
               decode.Read(file, listener);
             } catch(fit::RuntimeException const& e) {
               return "LoadFIT::error: "s + e.what();
             } catch(...) {
               return "LoadFIT::error: unknown exception during decode"s;
             }
           }

           // Merge heart rate data: nearest-neighbour interpolation by timestamp.
           if(flags.merge_heart_rates && !listener.hrPoints.empty()) {
             auto tableIt = listener.tables.find(msgType);
             if(tableIt != listener.tables.end()) {
               auto& table = tableIt->second;
               std::ranges::sort(listener.hrPoints, {}, &HrPoint::timestamp);
               auto& hrColumn = table.columns["heart_rate"];
               for(size_t row = 0; row < table.rowCount; ++row) {
                 while(hrColumn.committedRows < row)
                   hrColumn.add(Symbol("NULL"));
                 double ts =
                     (row < listener.rowTimestamps.size()) ? listener.rowTimestamps[row] : -1.0;
                 if(ts < 0.0) {
                   hrColumn.add(Symbol("NULL"));
                   continue;
                 }
                 auto it = std::ranges::lower_bound(listener.hrPoints, ts, {}, &HrPoint::timestamp);
                 double bpm;
                 if(it == listener.hrPoints.end())
                   bpm = listener.hrPoints.back().bpm;
                 else if(it == listener.hrPoints.begin())
                   bpm = it->bpm;
                 else {
                   auto prev = std::prev(it);
                   bpm = (ts - prev->timestamp <= it->timestamp - ts) ? prev->bpm : it->bpm;
                 }
                 hrColumn.add(bpm);
               }
               while(hrColumn.committedRows < table.rowCount)
                 hrColumn.add(Symbol("NULL"));
             }
           }

           auto tableEntry = listener.tables.find(msgType);
           if(tableEntry == listener.tables.end())
             return "LoadFIT::error: message type not found: "s + msgType;

           auto columns = ExpressionArguments {};
           for(auto& [name, column] : tableEntry->second.columns)
             columns.emplace_back(
                 ComplexExpression(Symbol(name), {}, {}, std::move(column).build()));

           return ComplexExpression(Symbol("Table"), {}, std::move(columns), {});
         } <"LoadFIT"_(Any_, AnySequence_) >= Recurse(evaluate)>[](auto, auto dynamics,
                                                                   auto) -> Expression {
           // No message-type Symbol at position 1: emit a per-file summary table.
           auto const& path = std::get<std::string>(dynamics.at(0));

           ParseFlags flags;
           for(size_t i = 1; i < dynamics.size(); ++i) {
             auto* flagExpr = std::get_if<ComplexExpression>(&dynamics[i]);
             if(!flagExpr)
               continue;
             auto [flagHead, flagStatics, flagArgs, flagSpans] = std::move(*flagExpr).decompose();
             bool value = true;
             if(!flagArgs.empty()) {
               if(auto* iv = std::get_if<int64_t>(&flagArgs[0]))
                 value = (*iv != 0);
               else if(auto* dv = std::get_if<double>(&flagArgs[0]))
                 value = (*dv != 0.0);
             }
             auto const& flagName = flagHead.getName();
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

           auto filePaths = std::vector<std::filesystem::path> {};
           if(std::filesystem::is_directory(path)) {
             for(auto const& entry : std::filesystem::directory_iterator(path))
               if(entry.path().extension() == ".fit")
                 filePaths.push_back(entry.path());
             std::ranges::sort(filePaths);
           } else {
             filePaths.push_back(path);
           }

           struct : fit::MesgListener {
             ParseFlags flags;
             bool fileIdSeen = false;
             bool sessionSeen = false;
             std::optional<double> timeCreated;
             std::optional<double> startTime;
             std::optional<Symbol> sport;
             std::optional<double> totalElapsedTime;
             std::optional<double> totalDistance;
             std::optional<double> totalCalories;

             void reset() {
               fileIdSeen = false;
               sessionSeen = false;
               timeCreated.reset();
               startTime.reset();
               sport.reset();
               totalElapsedTime.reset();
               totalDistance.reset();
               totalCalories.reset();
             }

             std::optional<double> readDouble(fit::Mesg& mesg, char const* name) {
               auto* field = mesg.GetField(name);
               if(!field || !field->IsValueValid())
                 return std::nullopt;
               double value = flags.apply_scale_and_offset ? field->GetFLOAT64Value()
                                                           : field->GetRawValue();
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
             }
           } summaryListener;
           summaryListener.flags = flags;

           ColumnBuilder fileColumn;
           ColumnBuilder timeCreatedColumn;
           ColumnBuilder startTimeColumn;
           ColumnBuilder sportColumn;
           ColumnBuilder totalElapsedTimeColumn;
           ColumnBuilder totalDistanceColumn;
           ColumnBuilder totalCaloriesColumn;

           auto addOptionalDouble = [](ColumnBuilder& column,
                                       std::optional<double> value) {
             if(value)
               column.add(*value);
             else
               column.add(Symbol("NULL"));
           };

           for(auto const& filePath : filePaths) {
             auto file = std::fstream(filePath, std::ios::in | std::ios::binary);
             if(!file.is_open())
               return "LoadFIT::error: cannot open file: "s + filePath.string();
             summaryListener.reset();
             try {
               fit::Decode decode;
               if(!flags.expand_components)
                 decode.SuppressComponentExpansion();
               if(!flags.enable_crc_check)
                 decode.SkipHeader();
               decode.Read(file, summaryListener);
             } catch(fit::RuntimeException const& e) {
               return "LoadFIT::error: "s + e.what();
             } catch(...) {
               return "LoadFIT::error: unknown exception during decode"s;
             }

             fileColumn.add(filePath.filename().string());
             addOptionalDouble(timeCreatedColumn, summaryListener.timeCreated);
             addOptionalDouble(startTimeColumn, summaryListener.startTime);
             if(summaryListener.sport)
               sportColumn.add(*summaryListener.sport);
             else
               sportColumn.add(Symbol("NULL"));
             addOptionalDouble(totalElapsedTimeColumn, summaryListener.totalElapsedTime);
             addOptionalDouble(totalDistanceColumn, summaryListener.totalDistance);
             addOptionalDouble(totalCaloriesColumn, summaryListener.totalCalories);
           }

           auto columns = ExpressionArguments {};
           columns.emplace_back(
               ComplexExpression(Symbol("file"), {}, {}, std::move(fileColumn).build()));
           columns.emplace_back(ComplexExpression(Symbol("time_created"), {}, {},
                                                  std::move(timeCreatedColumn).build()));
           columns.emplace_back(ComplexExpression(Symbol("start_time"), {}, {},
                                                  std::move(startTimeColumn).build()));
           columns.emplace_back(
               ComplexExpression(Symbol("sport"), {}, {}, std::move(sportColumn).build()));
           columns.emplace_back(ComplexExpression(Symbol("total_elapsed_time"), {}, {},
                                                  std::move(totalElapsedTimeColumn).build()));
           columns.emplace_back(ComplexExpression(Symbol("total_distance"), {}, {},
                                                  std::move(totalDistanceColumn).build()));
           columns.emplace_back(ComplexExpression(Symbol("total_calories"), {}, {},
                                                  std::move(totalCaloriesColumn).build()));

           return ComplexExpression(Symbol("Table"), {}, std::move(columns), {});
                  } < "GetEngineDescription"_() >= [](auto, auto dynamics, auto) -> Expression {
           return R"(
**Loading FIT workout data:**
- `(LoadFIT "/path/to/dir")` - summary table, one row per `.fit` file; columns: `file`, `time_created`, `start_time`, `sport`, `total_elapsed_time` (seconds), `total_distance` (metres), `total_calories`
- `(LoadFIT "/path/to/file.fit" msgtype)` - single file with message type
- `(LoadFIT "/path/to/dir" msgtype)` - all files in directory with message type

Message types (Garmin FIT protocol spec columns):
- `session` - one row per workout; per-workout aggregates: `avg/max_heart_rate`, `avg/max_speed`, `avg/max_power`, `avg_cadence`, `total_calories`, `total_distance`, `total_elapsed_time`, `total_ascent`, `num_laps`, GPS bounding box, `training_load_peak`, `sport`, `timestamp`, etc.
- `record` - one row per second; time-series GPS/sensor data: `timestamp`, `position_lat/long`, `altitude`, `heart_rate`, `cadence`, `speed`, `power`, `distance`, etc. **Large - use only for single-file analysis.**
- `lap` - per-lap summaries
- `activity` - activity-level metadata

**Path conventions:** Paths must be absolute. `~` is not expanded (BOSS does not invoke shell expansion). Use `/Users/<name>/...` on macOS, `/home/<name>/...` on Linux.

             **Unicode in filenames:** Raw UTF-8 and JSON `\uXXXX` escapes are both accepted and equivalent - use whichever your client emits naturally. The real hazard is *invisible* Unicode: filenames produced by Apple devices commonly contain U+00A0 (non-breaking space) where a regular space appears to be - for instance, between "Apple" and "Watch" in Apple Watch export filenames. NBSP renders identically to a regular space everywhere, including in the `file` column returned by the directory-summary query, so it cannot be detected by sight. If `LoadFIT` reports `cannot open file` on a path that *visually* matches the directory listing, write the suspect gaps explicitly as `\u00a0` and retry. The same caution applies to U+200B (zero-width space), U+00AD (soft hyphen), and the Unicode dash variants. Discover the row with `(Slice (OrderBy (LoadFIT ".../dir") (List (Desc time_created))) 0 1)`.

**Key patterns:**
```
; Most recent N workouts
(Slice (OrderBy (LoadFIT ".../dir") (List (Desc time_created))) 0 5)

; Per-sport average of a derived metric (derive first, then aggregate)
(GroupBy
  (Project (LoadFIT ".../dir")
    (As (Divide total_calories (Divide total_elapsed_time 60.0)) cal_per_min)
    sport)
  (Mean cal_per_min)
  sport)

; Cache a large load for reuse across calls
(Name (LoadFIT ".../dir" session) workouts)
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
  return new BOSSExpression {.delegate = evaluate(std::move(e->delegate))};
};
