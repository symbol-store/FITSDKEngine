#include <BOSS.hpp>
#include <Expression.hpp>
#include <ExpressionUtilities.hpp>
#include <Utilities.hpp>

#include "fit_decode.hpp"
#include "fit_mesg.hpp"
#include "fit_mesg_listener.hpp"
#include "fit_runtime_exception.hpp"

#include <filesystem>
#include <fstream>
#include <map>
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

} // namespace

static Expression evaluate(Expression&& e) {
  using sentinel::Any_;
  using sentinel::Symbol_;
  return std::move(e) //
         <"LoadFIT"_(Any_, Symbol_) >= Recurse(evaluate)>[](auto, auto dynamics,
                                                            auto) -> Expression {
           auto const& path = std::get<std::string>(dynamics.at(0));
           auto const& msgType = std::get<Symbol>(dynamics.at(1)).getName();

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
             std::unordered_map<std::string, MessageTable> tables;
             void OnMesg(fit::Mesg& mesg) override {
               if(mesg.GetName() != targetType)
                 return;
               auto& table = tables[mesg.GetName()];
               for(FIT_UINT16 i = 0; i < (FIT_UINT16)mesg.GetNumFields(); i++) {
                 auto* field = mesg.GetFieldByIndex(i);
                 if(!field || !field->IsValid() || !field->IsValueValid())
                   continue;
                 auto& column = table.columns[field->GetName()];
                 while(column.committedRows < table.rowCount)
                   column.add(Symbol("NULL"));
                 if(field->GetName() == "sport")
                   column.add(fitSportName(static_cast<int>(field->GetFLOAT64Value())));
                 else {
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
                   default:
                     // GetFLOAT64Value applies scale and offset as defined by the FIT profile
                     column.add(field->GetFLOAT64Value());
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

           for(auto const& filePath : filePaths) {
             auto file = std::fstream(filePath, std::ios::in | std::ios::binary);
             if(!file.is_open())
               return "LoadFIT::error: cannot open file: "s + filePath.string();
             try {
               fit::Decode().Read(file, listener);
             } catch(fit::RuntimeException const& e) {
               return "LoadFIT::error: "s + e.what();
             } catch(...) {
               return "LoadFIT::error: unknown exception during decode"s;
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
         } < Any_ >= Recurse(evaluate);
};

extern "C" BOSSExpression* evaluate(BOSSExpression* e) {
  return new BOSSExpression {.delegate = evaluate(std::move(e->delegate))};
};
