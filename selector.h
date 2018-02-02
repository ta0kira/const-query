#ifndef SELECTOR_H_
#define SELECTOR_H_

#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "collection.h"
#include "query.h"
#include "schema.h"
#include "static_errors.h"

namespace const_query {
namespace internal {

// Join Representation and Collection

template<int L, class ColumnEnumL, ColumnEnumL ColumnL,
         int R, class ColumnEnumR, ColumnEnumR ColumnR>
struct QueryJoiner {
  static std::string ToString() {
    std::ostringstream output;
    output << Column<ColumnEnumL, ColumnL>::ColumnName(TableAlias<ColumnEnumL, L>::Get()) << " = "
           << Column<ColumnEnumR, ColumnR>::ColumnName(TableAlias<ColumnEnumR, R>::Get());
    return output.str();
  }
};

template<class Queries, class Joins, bool Complete>
class JoinedSelector {
  JoinedSelector() = delete;
  ~JoinedSelector() = delete;
};

template<class Queries, class Joins>
class JoinedSelector<Queries, Joins, true> {
 private:
  using Columns = typename QueriesToColumns<Queries>::Type;
  constexpr JoinedSelector() = default;
  ~JoinedSelector() = default;
  template<class Queries2, class Joins2> friend class Selector;

 public:
  using ColumnTypes = typename ColumnsToTypes<Columns>::Types;

  std::string GetQuery() const {
    const auto columns_statement = GetColumns();
    const auto tables_statement = GetTables();
    std::ostringstream output;
    output << "SELECT " << columns_statement << " FROM " << tables_statement;
    return output.str();
  }

  std::vector<std::string> GetColumnNames() const {
    std::vector<std::string> column_names;
    CollectTableColumns<0, Queries>::To(&column_names);
    return column_names;
  }

  std::unique_ptr<ColumnTypes> ConvertRow(const std::vector<std::string>& values) const {
    auto row = std::unique_ptr<ColumnTypes>(new ColumnTypes);
    if (ConvertAll<Columns, std::tuple_size<Columns>::value>::At(values, row.get())) {
      return row;
    } else {
      return nullptr;
    }
  }

  static constexpr auto JoinNextOn = errors::YouAlreadyHaveEnoughJoins();

 private:
  std::string GetColumns() const {
    std::ostringstream output;
    std::vector<std::string> column_names = GetColumnNames();
    for (int i = 0; i < column_names.size(); ++i) {
      if (i > 0) {
        output << ", ";
      }
      output << column_names[i];
    }
    return output.str();
  }

  std::string GetTables() const {
    std::ostringstream output;
    std::vector<std::string> table_names;
    CollectTableNames<0, Queries>::To(&table_names);
    std::vector<std::string> joins;
    CollectJoins<Joins>::To(&joins);
    output << table_names[0];
    for (int i = 1; i < table_names.size(); ++i) {
      output << " JOIN " << table_names[i] << " ON (" << joins[i-1] << ")";
    }
    return output.str();
  }
};

}  // namespace internal

template<class Queries, class Joins = std::tuple<>>
struct Selector : public internal::JoinedSelector<Queries, Joins,
                                                  std::tuple_size<Queries>::value ==
                                                  std::tuple_size<Joins>::value+1> {};

template<>
struct Selector<std::tuple<>, std::tuple<>> {
  Selector() = delete;
  ~Selector() = delete;
};

namespace internal {

template<class Queries, class Joins, bool Valid, int Index>
struct ValidateJoin {};

template<class Queries, class Joins, int Index>
struct ValidateJoin<Queries, Joins, true, Index> {
  using Type = Selector<Queries, Joins>;
};

template<class Queries, class Joins, int Index>
struct ValidateJoin<Queries, Joins, false, Index> {
  using Type = errors::YouMustJoinWithAnIndexLessThan<Index>;
};

template <class Tuple, class Join>
struct AppendJoin {};

template <class Join, class ... Joins>
struct AppendJoin<std::tuple<Joins...>, Join> {
  using Type = std::tuple<Joins..., Join>;
};

template <int Index, class Tuple>
struct GetTableType {
  using Type = typename std::tuple_element<Index, Tuple>::type::TableType;
};

template <class Queries, class Joins>
class JoinedSelector<Queries, Joins, false> {
 private:
  static constexpr int NextJoin = std::tuple_size<Joins>::value+1;
  static constexpr int RemainingJoins = std::tuple_size<Queries>::value-NextJoin;
  constexpr JoinedSelector() = default;
  ~JoinedSelector() = default;
  template<class Queries2, class Joins2> friend class Selector;

 public:
  template<int L, typename GetTableType<L, Queries>::Type ColumnL,
           typename GetTableType<NextJoin, Queries>::Type ColumnR>
  constexpr typename ValidateJoin<Queries,
                                  typename AppendJoin<Joins,
                                                      QueryJoiner<L,        typename GetTableType<L, Queries>::Type,        ColumnL,
                                                                  NextJoin, typename GetTableType<NextJoin, Queries>::Type, ColumnR>>::Type,
                                  L < NextJoin, NextJoin>::Type JoinNextOn() {
    return typename ValidateJoin<Queries,
                                 typename AppendJoin<Joins,
                                                     QueryJoiner<L,        typename GetTableType<L, Queries>::Type,        ColumnL,
                                                                 NextJoin, typename GetTableType<NextJoin, Queries>::Type, ColumnR>>::Type,
                                 L < NextJoin, NextJoin>::Type();
  }

  static constexpr auto GetQuery = errors::YouNeedThisManyMoreJoins<RemainingJoins>();
  static constexpr auto ConvertRow = errors::YouNeedThisManyMoreJoins<RemainingJoins>();
};

template <class TheType, class Tuple>
struct CheckSameDatabase {};

template <class TheType>
struct CheckSameDatabase<TheType, std::tuple<>> {};

template <class TheType, class Table>
struct CheckSameDatabase<TheType, std::tuple<Table>> {
  using Type = TheType;
};

template <class TheType, class Table1, class Database1, class Table2, class Database2>
struct OnSuccess {
  using Type = errors::TheseTablesAreNotInTheSameDatabase<Table1, Database1, Table2, Database2>;
};

template <class TheType, class Table1, class Database, class Table2>
struct OnSuccess<TheType, Table1, Database, Table2, Database> {
  using Type = TheType;
};

template <class TheType, class Query1, class Query2, class ... Queries>
struct CheckSameDatabase<TheType, std::tuple<Query1, Query2, Queries...>> {
  using Type = typename OnSuccess<typename CheckSameDatabase<TheType, std::tuple<Query2, Queries...>>::Type,
                                  typename Query1::TableType, typename Database<typename Query1::TableType>::Id,
                                  typename Query2::TableType, typename Database<typename Query2::TableType>::Id>::Type;
};

template<class ... Queries>
struct NonEmptySelector {
  using Type = typename CheckSameDatabase<Selector<std::tuple<Queries...>>, std::tuple<Queries...>>::Type;
};

template<>
struct NonEmptySelector<> {
  using Type = errors::YouMayNotHaveAnEmptySelector;
};

}  // namespace internal

template<class ... Queries>
constexpr typename internal::NonEmptySelector<Queries...>::Type Select(Queries ... queries) {
  return typename internal::NonEmptySelector<Queries...>::Type();
}

}  // namespace const_query

#endif  // SELECTOR_H_
