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
             std::unordered_map<std::string, MessageTable> tables;
             void OnMesg(fit::Mesg& mesg) override {
               auto& table = tables[mesg.GetName()];
               for(FIT_UINT16 i = 0; i < (FIT_UINT16)mesg.GetNumFields(); i++) {
                 auto* field = mesg.GetFieldByIndex(i);
                 if(!field || !field->IsValid() || !field->IsValueValid())
                   continue;
                 auto& column = table.columns[field->GetName()];
                 while(column.committedRows < table.rowCount)
                   column.add(Symbol("NULL"));
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
               table.rowCount++;
               for(auto& [_, column] : table.columns)
                 if(column.committedRows < table.rowCount)
                   column.add(Symbol("NULL"));
             }
           } listener;

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
