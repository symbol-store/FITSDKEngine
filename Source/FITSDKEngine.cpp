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
             std::sort(filePaths.begin(), filePaths.end());
           } else {
             filePaths.push_back(path);
           }

           struct : fit::MesgListener {
             std::unordered_map<
                 std::string,
                 std::map<std::string, std::variant<std::vector<double>, std::vector<std::string>>>>
                 tables;
             void OnMesg(fit::Mesg& mesg) override {
               auto& columns = tables[mesg.GetName()];
               for(FIT_UINT16 i = 0; i < (FIT_UINT16)mesg.GetNumFields(); i++) {
                 auto* field = mesg.GetFieldByIndex(i);
                 if(!field || !field->IsValid() || !field->IsValueValid())
                   continue;
                 switch(field->GetType()) {
                 case FIT_BASE_TYPE_STRING: {
                   auto& column = columns[field->GetName()];
                   if(std::holds_alternative<std::vector<double>>(column))
                     column = std::vector<std::string> {};
                   auto const& wstr = field->GetSTRINGValue();
                   std::get<std::vector<std::string>>(column).emplace_back(wstr.begin(),
                                                                           wstr.end());
                   break;
                 }
                 case FIT_BASE_TYPE_ENDIAN_FLAG:
                 case FIT_BASE_TYPE_RESERVED:
                 case FIT_BASE_TYPE_NUM_MASK:
                   break;
                 default:
                   // GetFLOAT64Value applies scale and offset as defined by the FIT profile
                   std::get<std::vector<double>>(columns[field->GetName()])
                       .emplace_back(field->GetFLOAT64Value());
                 }
               }
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
           for(auto& [name, column] : tableEntry->second)
             columns.emplace_back(std::visit(
                 [&name](auto& values) -> Expression {
                   return ComplexExpression(
                       Symbol(name), {}, {},
                       ExpressionSpanArguments(
                           Span<typename std::decay_t<decltype(values)>::value_type>(
                               std::move(values))));
                 },
                 column));

           return ComplexExpression(Symbol("Table"), {}, std::move(columns), {});
         } < Any_ >= Recurse(evaluate);
};

extern "C" BOSSExpression* evaluate(BOSSExpression* e) {
  return new BOSSExpression {.delegate = evaluate(std::move(e->delegate))};
};
