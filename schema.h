#ifndef SCHEMA_H_
#define SCHEMA_H_

#include <sstream>
#include <string>
#include <tuple>

#include "query.h"
#include "static_errors.h"

namespace const_query {

// Schema Definition

template <class ColumnEnum>
struct Table {
  static constexpr auto TableName
      = errors::SpecializeWithStaticFunction<Table<ColumnEnum>, std::string()>();
};

template<class ColumnEnum, ColumnEnum Name>
struct Column {
  using ColumnType = errors::SpecializeWithTypedef<Column<ColumnEnum, Name>>;

  static constexpr auto ColumnName
      = errors::SpecializeWithStaticFunction<Column<ColumnEnum, Name>, std::string(const std::string&)>();

  static constexpr auto ParseFromString
      = errors::SpecializeWithStaticFunction<Column<ColumnEnum, Name>, bool(const std::string&, ColumnType*)>();
};

namespace internal {

struct UnknownDatabase {
  UnknownDatabase() = delete;
  ~UnknownDatabase() = delete;
};

}  // namespace internal

// Database Identification

template<class ColumnEnum>
struct Database {
  using Id = internal::UnknownDatabase;
};

// Table Aliases

template <class ColumnEnum, int Number>
struct TableAlias {
  static std::string Define() {
    std::ostringstream output;
    output << Table<ColumnEnum>::TableName() << " AS " << Get();
    return output.str();
  }

  static std::string Get() {
    std::ostringstream output;
    output << Table<ColumnEnum>::TableName() << "_" << Number;
    return output.str();
  }
};

namespace internal {

// Collecting Column Type Information

template <class Tuple>
struct ColumnsToTypes {};

template <class ... Columns>
struct ColumnsToTypes<std::tuple<Columns...>> {
  using Types = std::tuple<typename Columns::ColumnType...>;
};

// Collecting Column Information from Queries

template<class Tuple, class ... Columns>
struct QueriesToColumns {};

template<class ColumnEnum, ColumnEnum ... Names, class ... Queries, class ... Columns>
struct QueriesToColumns<std::tuple<Query<ColumnEnum, Names...>, Queries...>, Columns...> {
  using Type = typename QueriesToColumns<std::tuple<Queries...>, Columns...,
                                         Column<ColumnEnum, Names>...>::Type;
};

template<class ... Columns>
struct QueriesToColumns<std::tuple<>, Columns...> {
  using Type = std::tuple<Columns...>;
};

}  // namespace internal
}  // namespace const_query

#endif  // SCHEMA_H_
