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
