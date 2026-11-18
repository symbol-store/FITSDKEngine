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
#include <array>
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

// A table built row by row and emitted shape-wise partitioned (Mohr-Daurat et al., PVLDB
// 17(11)): rows are grouped by their shape, i.e. the atom type of each column, and every
// partition becomes exactly one span per column, so span k of every column covers the same
// rows. A missing value is a NULL symbol, i.e. just another atom type. Row order is not
// preserved.
class ShapeWiseTable {
public:
  using Value = std::variant<double, std::string, Symbol>;

  // Dense id of a column; columns are emitted in the order they are first registered.
  size_t column(std::string const& name) {
    auto [it, inserted] = columnIds.try_emplace(name, columnNames.size());
    if(inserted) {
      columnNames.push_back(name);
      row.emplace_back();
    }
    return it->second;
  }

  std::vector<std::string> const& columns() const { return columnNames; }
  bool empty() const { return partitions.empty(); }

  // Sets a cell of the current row (a double, std::string or Symbol). The first value set for
  // a column wins.
  template <typename T> void set(size_t columnId, T&& value) {
    if(std::holds_alternative<std::monostate>(row[columnId]))
      row[columnId] = std::forward<T>(value);
  }

  // Completes the current row; columns without a value are NULL.
  void endRow() {
    shape.clear();
    for(uint32_t c = 0; c < row.size(); ++c)
      if(row[c].index() != 0)
        shape.emplace_back(c, static_cast<uint8_t>(row[c].index() - 1)); // as Value index
    // Consecutive rows mostly share a shape; only look up the partition when it changes.
    if(!lastPartition || shape != lastShape) {
      lastShape = shape;
      lastPartition = &partitionFor(shape);
    }
    for(size_t j = 0; j < shape.size(); ++j) {
      auto& cell = row[shape[j].first];
      std::visit(
          [&](auto&& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr(!std::is_same_v<T, std::monostate>)
              std::get<std::vector<T>>(lastPartition->columns[j]).push_back(std::move(value));
          },
          std::move(cell));
      cell = std::monostate {};
    }
    lastPartition->rows++;
  }

  // Moves all rows of other into this table, matching columns by name.
  void append(ShapeWiseTable&& other) {
    std::vector<uint32_t> columnMap;
    for(auto const& name : other.columnNames)
      columnMap.push_back(static_cast<uint32_t>(column(name)));
    for(auto& [otherShape, otherPartition] : other.partitions) {
      // Translate the shape to this table's column ids, which may sort differently.
      std::vector<std::pair<ShapeEntry, size_t>> entries; // (entry, index in otherShape)
      entries.reserve(otherShape.size());
      for(size_t j = 0; j < otherShape.size(); ++j)
        entries.push_back({{columnMap[otherShape[j].first], otherShape[j].second}, j});
      std::ranges::sort(entries);
      Shape translated;
      translated.reserve(entries.size());
      for(auto const& [entry, _] : entries)
        translated.push_back(entry);
      auto& partition = partitionFor(translated);
      for(size_t k = 0; k < entries.size(); ++k) {
        auto& source = otherPartition.columns[entries[k].second];
        if(partition.rows == 0) { // New here: take the values over without copying.
          partition.columns[k] = std::move(source);
          continue;
        }
        std::visit(
            [&](auto& values) {
              auto& target = std::get<std::decay_t<decltype(values)>>(partition.columns[k]);
              target.insert(target.end(), std::make_move_iterator(values.begin()),
                            std::make_move_iterator(values.end()));
            },
            source);
      }
      partition.rows += otherPartition.rows;
    }
    other = ShapeWiseTable(); // Free the moved-from values now.
  }

  // Adds the double column `name`, computed as fn(source value) in every partition where the
  // double column `source` is set, and NULL elsewhere (keeping partitions shape-homogeneous).
  template <typename F>
  void deriveColumn(std::string const& name, std::string const& source, F const& fn) {
    auto derivedId = static_cast<uint32_t>(column(name));
    auto sourceIt = columnIds.find(source);
    if(sourceIt == columnIds.end())
      return;
    ShapeEntry const sourceEntry {static_cast<uint32_t>(sourceIt->second), doubleIndex};
    decltype(partitions) derived;
    while(!partitions.empty()) {
      auto node = partitions.extract(partitions.begin());
      auto& key = node.key();
      auto& partition = node.mapped();
      auto sourcePos = std::ranges::find(key, sourceEntry);
      if(sourcePos != key.end()) {
        auto const& sourceValues =
            std::get<std::vector<double>>(partition.columns[sourcePos - key.begin()]);
        std::vector<double> values;
        values.reserve(sourceValues.size());
        std::ranges::transform(sourceValues, std::back_inserter(values), fn);
        ShapeEntry const entry {derivedId, doubleIndex};
        auto pos = std::ranges::lower_bound(key, entry) - key.begin();
        key.insert(key.begin() + pos, entry);
        partition.columns.insert(partition.columns.begin() + pos, std::move(values));
      }
      derived.insert(std::move(node));
    }
    partitions = std::move(derived);
    lastPartition = nullptr;
  }

  // One span per column per partition.
  ExpressionArguments build() && {
    std::vector<ExpressionSpanArguments> spans(columnNames.size());
    for(auto& [key, partition] : partitions) {
      size_t j = 0;
      for(size_t c = 0; c < columnNames.size(); ++c) {
        if(j < key.size() && key[j].first == c) {
          std::visit(
              [&](auto& values) {
                using T = typename std::decay_t<decltype(values)>::value_type;
                spans[c].push_back(Span<T>(std::move(values)));
              },
              partition.columns[j++]);
        } else {
          spans[c].push_back(Span<Symbol>(std::vector<Symbol>(partition.rows, Symbol("NULL"))));
        }
      }
    }
    ExpressionArguments result;
    for(size_t c = 0; c < columnNames.size(); ++c)
      result.emplace_back(ComplexExpression(Symbol(columnNames[c]), {}, {}, std::move(spans[c])));
    return result;
  }

private:
  using Cell = std::variant<std::monostate, double, std::string, Symbol>; // monostate: unset
  using ColumnValues = boss::utilities::rewrap_variant_arguments<std::vector, Value>::type;
  using ShapeEntry = std::pair<uint32_t, uint8_t>; // (column id, Value index)
  using Shape = std::vector<ShapeEntry>;           // Non-NULL cells, sorted by column id.

  static constexpr uint8_t doubleIndex = 0; // Value index of double
  static_assert(std::is_same_v<std::variant_alternative_t<doubleIndex, Value>, double>);

  struct Partition {
    size_t rows = 0;
    std::vector<ColumnValues> columns; // One per shape entry, in shape order.
  };

  static ColumnValues emptyColumn(uint8_t valueIndex) {
    return [valueIndex]<size_t... I>(std::index_sequence<I...>) {
      ColumnValues column;
      ((I == valueIndex ? (void)column.emplace<I>() : void()), ...);
      return column;
    }(std::make_index_sequence<std::variant_size_v<Value>>());
  }

  Partition& partitionFor(Shape const& key) {
    auto [it, inserted] = partitions.try_emplace(key);
    if(inserted)
      for(auto const& [_, valueIndex] : key)
        it->second.columns.push_back(emptyColumn(valueIndex));
    return it->second;
  }

  std::unordered_map<std::string, size_t> columnIds;
  std::vector<std::string> columnNames;
  std::map<Shape, Partition> partitions;

  // Current row (one cell per column) and the shape and partition of the previous row.
  std::vector<Cell> row;
  Shape shape;
  Shape lastShape;
  Partition* lastPartition = nullptr;
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

// A FIT date_time value as the engine emits it: shifted to the Unix epoch when requested,
// except for relative values, which stay as they are.
static double convertDateTime(double value, ParseFlags const& flags) {
  return flags.convert_datetimes_to_dates && value >= kFitDateTimeMin ? value + kFitEpochOffset
                                                                      : value;
}

struct HrPoint {
  double timestamp;
  double bpm;
};

// Message filter for Decode::Read restricting decoding to the named message types, so the SDK
// skips everything else (mostly `record` messages) without decoding it. A name unknown to the
// profile yields an empty filter (decode everything), so the listener sees exactly what it
// would without the filter.
static std::vector<FIT_UINT16> mesgFilterFor(std::initializer_list<std::string> names) {
  std::vector<FIT_UINT16> mesgNums;
  for(auto const& name : names) {
    auto const* profile = fit::Profile::GetMesg(name);
    if(!profile)
      return {};
    mesgNums.push_back(profile->num);
  }
  return mesgNums;
}

// The messages read by the directory summary and the timestamp pre-filter.
static std::vector<FIT_UINT16> const& summaryMesgFilter() {
  static auto const filter = mesgFilterFor({"file_id", "session"});
  return filter;
}

// Decode one file into `listener` with the parse flags applied. Listeners with a `decode`
// member (to Pause() once they have what they need) get the decoder. Returns the Load error
// message on failure.
template <typename Listener>
static std::optional<std::string> decodeFile(std::filesystem::path const& filePath,
                                             Listener& listener, ParseFlags const& flags,
                                             std::vector<FIT_UINT16> const& mesgFilter) {
  auto file = std::fstream(filePath, std::ios::in | std::ios::binary);
  if(!file.is_open())
    return "Load::error: cannot open file: "s + filePath.string();
  try {
    fit::Decode decode;
    if constexpr(requires { listener.decode; })
      listener.decode = &decode;
    if(!flags.expand_components)
      decode.SuppressComponentExpansion();
    if(!flags.enable_crc_check)
      decode.SkipHeader();
    decode.Read(file, listener, mesgFilter);
  } catch(fit::RuntimeException const& e) {
    return "Load::error: "s + e.what();
  } catch(...) {
    return "Load::error: unknown exception during decode"s;
  }
  return std::nullopt;
}

// Number of threads parallelFor uses for n items; size per-worker state with it.
static size_t workerCount(size_t n) {
  return std::min<size_t>(std::max(1u, std::thread::hardware_concurrency()), n);
}

// Runs fn(worker, i) for every i in [0, n) on workerCount(n) threads, where worker is the
// index of the calling thread. fn must not throw.
template <typename F> static void parallelFor(size_t n, F const& fn) {
  size_t numWorkers = workerCount(n);
  std::atomic<size_t> next {0};
  std::vector<std::thread> workers;
  workers.reserve(numWorkers);
  for(size_t w = 0; w < numWorkers; ++w)
    workers.emplace_back([&, w]() {
      size_t i;
      while((i = next.fetch_add(1, std::memory_order_relaxed)) < n)
        fn(w, i);
    });
  for(auto& t : workers)
    t.join();
}

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
      auto const& name = entry.path().filename().string();
      if(!name.empty() && name.front() == '.')
        continue;
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

  if(decodeFile(filePath, listener, flags, summaryMesgFilter()))
    return std::nullopt;

  if(!listener.startTime)
    return std::nullopt;
  double recordStart = convertDateTime(*listener.startTime, flags);
  double recordEnd = recordStart + listener.totalElapsedTime.value_or(0.0);
  return FileTimeRange {recordStart, recordEnd};
}

// Files with unreadable session metadata are kept (conservative).
static std::vector<std::filesystem::path>
filterFilesByTimeRange(std::vector<std::filesystem::path>&& files, ParseFlags const& flags) {
  if(!flags.timestamp_min && !flags.timestamp_max)
    return std::move(files);
  std::vector<std::optional<FileTimeRange>> ranges(files.size());
  parallelFor(files.size(), [&](size_t, size_t i) { ranges[i] = readFileTimeRange(files[i], flags); });
  std::vector<std::filesystem::path> filtered;
  filtered.reserve(files.size());
  for(size_t i = 0; i < files.size(); ++i) {
    auto& path = files[i];
    auto const& range = ranges[i];
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

// --- Filename correction -----------------------------------------------------
// load is frequently called with paths that are *visually* correct but byte-wise
// wrong, because some intermediary (a JSON layer, a copy-paste, an Apple export)
// substituted a look-alike Unicode character for an ASCII one. The classic case
// is U+00A0 (non-breaking space) where a plain space is expected, but zero-width
// characters and the Unicode dash variants cause the same class of failure.
//
// To recover, we canonicalise both the requested filename and the names actually
// on disk: invisible code points are dropped, the Unicode space and dash variants
// collapse onto ASCII ' ' and '-'. If a directory entry canonicalises to the same
// string as the (missing) requested name, that entry is the intended target.

static void appendUtf8(std::string& out, char32_t cp) {
  if(cp <= 0x7F) {
    out.push_back(static_cast<char>(cp));
  } else if(cp <= 0x7FF) {
    out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
    out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
  } else if(cp <= 0xFFFF) {
    out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
    out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
    out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
  } else {
    out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
    out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
    out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
    out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
  }
}

// Code points that render as whitespace but are not ASCII space.
static bool isUnicodeSpace(char32_t cp) {
  switch(cp) {
  case 0x00A0: // no-break space
  case 0x1680: // ogham space mark
  case 0x2000: case 0x2001: case 0x2002: case 0x2003: case 0x2004:
  case 0x2005: case 0x2006: case 0x2007: case 0x2008: case 0x2009:
  case 0x200A: // en/em/thin/hair spaces
  case 0x202F: // narrow no-break space
  case 0x205F: // medium mathematical space
  case 0x3000: // ideographic space
    return true;
  default:
    return false;
  }
}

// Code points that render as a hyphen/dash but are not ASCII '-'.
static bool isUnicodeDash(char32_t cp) {
  switch(cp) {
  case 0x2010: case 0x2011: case 0x2012: case 0x2013: case 0x2014:
  case 0x2015: // hyphen .. horizontal bar
  case 0x2212: // minus sign
  case 0xFE58: case 0xFE63: // small em/hyphen-minus
  case 0xFF0D: // fullwidth hyphen-minus
    return true;
  default:
    return false;
  }
}

// Zero-width / formatting code points that should simply vanish.
static bool isInvisible(char32_t cp) {
  switch(cp) {
  case 0x00AD: // soft hyphen
  case 0x200B: // zero-width space
  case 0x200C: // zero-width non-joiner
  case 0x200D: // zero-width joiner
  case 0x2060: // word joiner
  case 0xFEFF: // zero-width no-break space / BOM
    return true;
  default:
    return false;
  }
}

static std::string canonicaliseFilename(std::string const& s) {
  std::string out;
  out.reserve(s.size());
  size_t i = 0, n = s.size();
  while(i < n) {
    unsigned char c = static_cast<unsigned char>(s[i]);
    char32_t cp = 0;
    size_t len = 1;
    if(c < 0x80) {
      cp = c;
      len = 1;
    } else if((c >> 5) == 0x6) {
      cp = c & 0x1F;
      len = 2;
    } else if((c >> 4) == 0xE) {
      cp = c & 0x0F;
      len = 3;
    } else if((c >> 3) == 0x1E) {
      cp = c & 0x07;
      len = 4;
    } else {
      // Invalid lead byte — pass through verbatim.
      out.push_back(s[i++]);
      continue;
    }
    if(i + len > n) {
      out.push_back(s[i++]);
      continue;
    }
    bool valid = true;
    for(size_t k = 1; k < len; ++k) {
      unsigned char cc = static_cast<unsigned char>(s[i + k]);
      if((cc >> 6) != 0x2) {
        valid = false;
        break;
      }
      cp = (cp << 6) | (cc & 0x3F);
    }
    if(!valid) {
      out.push_back(s[i++]);
      continue;
    }
    i += len;
    if(isInvisible(cp))
      continue;
    if(isUnicodeSpace(cp))
      out.push_back(' ');
    else if(isUnicodeDash(cp))
      out.push_back('-');
    else
      appendUtf8(out, cp);
  }
  return out;
}

// If `requested` does not exist but a sibling entry canonicalises to the same
// name, return that sibling's path — the filename the caller almost certainly
// meant. Returns nullopt when the path already exists or no match is found.
static std::optional<std::filesystem::path>
findFilenameCorrection(std::filesystem::path const& requested) {
  std::error_code ec;
  if(std::filesystem::exists(requested, ec))
    return std::nullopt;
  auto parent = requested.parent_path();
  if(parent.empty())
    parent = std::filesystem::path(".");
  if(!std::filesystem::is_directory(parent, ec))
    return std::nullopt;
  auto target = canonicaliseFilename(requested.filename().string());
  if(target.empty())
    return std::nullopt;
  for(auto const& entry : std::filesystem::directory_iterator(parent, ec)) {
    if(canonicaliseFilename(entry.path().filename().string()) == target)
      return entry.path();
  }
  return std::nullopt;
}

// Build (DidYouMean (Load <corrected> <rest...>)) if the load's path argument
// names a missing file with a canonical match on disk. The returned expression
// is a suggestion: it is *not* evaluated, leaving the caller to confirm.
static std::optional<Expression> correctLoadFilename(ExpressionArguments& dynamics) {
  if(dynamics.empty())
    return std::nullopt;
  auto const* pathArg = std::get_if<std::string>(&dynamics[0]);
  if(!pathArg)
    return std::nullopt;
  auto corrected = findFilenameCorrection(std::filesystem::path(*pathArg));
  if(!corrected)
    return std::nullopt;
  dynamics[0] = corrected->string();
  return "DidYouMean"_("Load"_(std::move(dynamics)));
}

static Expression evaluate(Expression&& e) {
  using sentinel::Any_;
  using sentinel::AnySequence_;
  using sentinel::Symbol_;
  return std::move(e) //
         <"Load"_(Any_, Symbol_, AnySequence_) >= Recurse(evaluate)>[](auto, auto dynamics,
                                                                       auto) -> Expression {
           if(auto suggestion = correctLoadFilename(dynamics))
             return std::move(*suggestion);
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

             // Name-derived per-field facts, resolved once per field number (only messages of
             // the target type reach the column code, so field numbers map to fixed names).
             struct CachedColumn {
               ColumnBuilder* column = nullptr;
               bool isSport = false;
               bool isDateTime = false;
             };
             std::array<CachedColumn, 256> mainColumns {};
             std::map<std::pair<FIT_UINT8, FIT_UINT16>, ColumnBuilder*> subFieldColumns;
             MessageTable* table = nullptr;
             std::optional<FIT_UINT16> targetNum; // nullopt: unknown to the profile, match by name

             void setTarget(std::string_view type) {
               targetType = type;
               table = &tables[std::string(type)];
               if(auto const* profile = fit::Profile::GetMesg(std::string(type)))
                 targetNum = profile->num;
             }

             bool isTarget(fit::Mesg& mesg) const {
               return targetNum ? mesg.GetNum() == *targetNum : mesg.GetName() == targetType;
             }

             CachedColumn& mainColumnFor(fit::Field* field) {
               auto& cached = mainColumns[field->GetNum()];
               if(!cached.column) {
                 auto name = field->GetName();
                 cached.column = &table->columns[name];
                 cached.isSport = name == "sport";
                 cached.isDateTime = isDateTimeField(name);
               }
               return cached;
             }

             ColumnBuilder& subFieldColumnFor(fit::Field* field, FIT_UINT16 subFieldIndex,
                                              fit::Profile::SUBFIELD const* subFieldProfile) {
               auto& column = subFieldColumns[{field->GetNum(), subFieldIndex}];
               if(!column)
                 column = &table->columns[std::string(subFieldProfile->name)];
               return *column;
             }

             void addNumericValue(ColumnBuilder& column, fit::FieldBase* field,
                                  FIT_UINT16 subFieldIndex = FIT_SUBFIELD_INDEX_MAIN_FIELD) {
               if(flags.apply_scale_and_offset)
                 column.add(field->GetFLOAT64Value(0, subFieldIndex));
               else
                 column.add(field->GetRawValue());
             }

             void OnMesg(fit::Mesg& mesg) override {
               // Collect HR messages when merging heart rates.
               if(flags.merge_heart_rates && mesg.GetNum() == FIT_MESG_NUM_HR) {
                 auto* tsField = mesg.GetField("timestamp");
                 auto* bpmField = mesg.GetField("filtered_bpm");
                 if(tsField && tsField->IsValueValid() && bpmField && bpmField->IsValueValid())
                   hrPoints.push_back({tsField->GetFLOAT64Value(), bpmField->GetFLOAT64Value()});
                 return;
               }

               if(!isTarget(mesg))
                 return;

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

                 auto& cached = mainColumnFor(field);
                 auto& column = *cached.column;
                 while(column.committedRows < table->rowCount)
                   column.add(Symbol("NULL"));

                 if(cached.isSport) {
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
                     if(flags.convert_datetimes_to_dates && cached.isDateTime &&
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
                       auto& sfColumn = subFieldColumnFor(field, activeSubField, subFieldProfile);
                       while(sfColumn.committedRows < table->rowCount)
                         sfColumn.add(Symbol("NULL"));
                       addNumericValue(sfColumn, field, activeSubField);
                     }
                   }
                 }
               }

               table->rowCount++;
               for(auto& [_, column] : table->columns)
                 if(column.committedRows < table->rowCount)
                   column.add(Symbol("NULL"));
             }
           };

           auto const mesgFilter = flags.merge_heart_rates
                                       ? mesgFilterFor({std::string(msgType), "hr"})
                                       : mesgFilterFor({std::string(msgType)});

           // One listener per worker; rows are merged in worker order below.
           std::vector<FitDataListener> listeners(workerCount(filePaths.size()));
           for(auto& l : listeners) {
             l.setTarget(msgType);
             l.flags = flags;
           }
           std::vector<std::optional<std::string>> fileErrors(filePaths.size());
           parallelFor(filePaths.size(), [&](size_t worker, size_t i) {
             fileErrors[i] = decodeFile(filePaths[i], listeners[worker], flags, mesgFilter);
           });

           for(auto& e : fileErrors)
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
           if(auto suggestion = correctLoadFilename(dynamics))
             return std::move(*suggestion);
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
               if(isDateTimeField(name))
                 value = convertDateTime(value, flags);
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

           ShapeWiseTable summary;
           auto fileColumn = summary.column("file");
           auto timeCreatedColumn = summary.column("time_created");
           auto startTimeColumn = summary.column("start_time");
           auto sportColumn = summary.column("sport");
           auto totalElapsedTimeColumn = summary.column("total_elapsed_time");
           auto totalDistanceColumn = summary.column("total_distance");
           auto totalCaloriesColumn = summary.column("total_calories");

           auto setOptional = [&summary](size_t column, auto const& value) {
             if(value)
               summary.set(column, *value);
           };

           // Decode in parallel into per-file slots; the first error is reported in file order.
           std::vector<SummaryListener> summaries(filePaths.size());
           std::vector<std::optional<std::string>> errors(filePaths.size());
           parallelFor(filePaths.size(), [&](size_t, size_t i) {
             summaries[i].flags = flags;
             errors[i] = decodeFile(filePaths[i], summaries[i], flags, summaryMesgFilter());
           });

           for(size_t i = 0; i < filePaths.size(); ++i) {
             if(errors[i])
               return std::move(*errors[i]);
             auto const& filePath = filePaths[i];
             auto const& summaryListener = summaries[i];

             summary.set(fileColumn, filePath.filename().string());
             setOptional(timeCreatedColumn, summaryListener.timeCreated);
             setOptional(startTimeColumn, summaryListener.startTime);
             setOptional(sportColumn, summaryListener.sport);
             setOptional(totalElapsedTimeColumn, summaryListener.totalElapsedTime);
             setOptional(totalDistanceColumn, summaryListener.totalDistance);
             setOptional(totalCaloriesColumn, summaryListener.totalCalories);
             summary.endRow();
           }

           auto table = "Table"_(std::move(summary).build());
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

**Auto-correction:** When `Load` is given a single file path that does not exist but whose name matches a file on disk *after* collapsing these look-alike code points (Unicode spaces \u2192 space, Unicode dashes \u2192 `-`, zero-width characters dropped), the engine does not error. Instead it returns the same query with the on-disk filename substituted, wrapped as `(DidYouMean (Load "/corrected/path.fit" ...))`. This is a suggestion, not a result: re-issue the inner `Load` to actually read the file.

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
