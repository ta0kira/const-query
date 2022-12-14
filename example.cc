#include <initializer_list>
#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <tuple>

#include "example.h"

#include "filter.h"
#include "record.h"
#include "selector.h"

// Fakes sending a query to a DBMS. The `raw_results` don't have strong typing,
// like most results from SQL-like systems.
//
// The query itself is encoded in the type of `Select`, which is presumed to be
// a `const_query::Selector<Queries, Joins>`. In the absence of a filter,
// `const_query::EmptyFilter` is able to call `Selector::GetQuery`, which will
// build the query by pattern-matching the template parameters.
template<class T, class Select>
std::list<T> PretendToExecuteQuery(
    const std::vector<std::vector<std::string>>& raw_results,
    const const_query::RecordBuilder<T, Select>& builder,
    const const_query::Filter<Select>& filter =
        const_query::EmptyFilter<Select>()) {
  static constexpr Select SELECTOR;
  std::cerr << "Faking query: " << filter.GetQuery() << std::endl;
  std::list<T> results;
  for (const std::vector<std::string>& raw_result : raw_results) {
    const auto converted = SELECTOR.ConvertRow(raw_result);
    if (!converted) {
      std::cerr << "conversion failed" << std::endl;
    } else {
      results.push_back(builder.CreateFrom(*converted));
    }
  }
  return results;
}

using example_schema::ChainTable;
using example_schema::DataTable;

// An example record object that is meant to be built using a specific query.
// The query is ecoded in the `Selector` typedef. This is enough information to
// build the entire SELECT + JOIN statement, without any WHERE clauses.
//
// `ExampleRecord` doesn't use the query information directly; it uses it to
// define a `const_query::RecordBuilder`, which defines how to query for record
// data and how to turn individual records into `ExampleRecord` instances. This
// puts the query definition and field mappings all in one place, but without
// any sort of business logic.
class ExampleRecord {
 public:
  std::ostream& Print(std::ostream& out) const {
    return out << key_ << ") " << name_ << ": " << data_;
  }

  using Selector =
      decltype(const_query::Select(
                 const_query::Query<ChainTable>()
                   .Get<ChainTable::KEY>()          // 0
                   .Get<ChainTable::NAME>(),        // 1
                 const_query::Query<ChainTable>(),
                 const_query::Query<DataTable>()
                   .Get<DataTable::DATA>())         // 2
                 .JoinNextOn<0, ChainTable::PARENT_KEY,
                                ChainTable::KEY>()
                 .JoinNextOn<1, ChainTable::DATA_KEY,
                                DataTable::KEY>());

  using FilterBuilder = const_query::FilterBuilder<Selector>;

  // This class encodes the query in the second `const_query::RecordBuilder`
  // template parameter, and defines a builder function for turning the results
  // of a query into `ExampleRecord`. This is sufficient information to query
  // and translate *all* records, but filter can be build from `FilterBuilder`
  // and passed separately to `PretendToExecuteQuery` by the caller.
  class Builder
      : public const_query::RecordBuilder<std::unique_ptr<const ExampleRecord>, Selector> {
   public:
    constexpr Builder() = default;

    std::unique_ptr<const ExampleRecord>
    CreateFrom(const ColumnTypes& source) const final {
      return std::unique_ptr<const ExampleRecord>(
          new ExampleRecord(std::get<0>(source),
                            std::get<1>(source),
                            std::get<2>(source)));
    }
  };

 private:
  ExampleRecord(int key, std::string name, std::string data)
      : key_(key), name_(std::move(name)), data_(std::move(data)) {}

  const int key_;
  const std::string name_;
  const std::string data_;
};

std::ostream& operator <<(std::ostream& out, const ExampleRecord& record) {
  return record.Print(out);
}

int main() {
  static constexpr ExampleRecord::FilterBuilder B;
  const auto filter =
      B.And(
        B.Equals<0, ChainTable::KEY,
                 1, ChainTable::PARENT_KEY>(),
        B.Equals<0, ChainTable::KEY,
                 2, DataTable::KEY>());

  const auto records = PretendToExecuteQuery({
        std::vector<std::string>{ "1",   "One",    "x" },
        std::vector<std::string>{ "2",   "Two",    "y" },
        std::vector<std::string>{ "Bad", "Three",  "z" },
      }, ExampleRecord::Builder(), filter);

  for (const auto& record : records) {
    std::cout << *record << std::endl;
  }
}
