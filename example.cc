#include <initializer_list>
#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <tuple>

#include "example.h"

#include "filter.h"
#include "selector.h"

template<class RecordType>
std::list<std::unique_ptr<RecordType>> PretendToExecuteQuery(
    const std::vector<std::vector<std::string>>& raw_results,
    const const_query::Filter<typename RecordType::Selector>& filter =
        const_query::EmptyFilter<typename RecordType::Selector>()) {
  static constexpr typename RecordType::Selector SELECTOR;
  std::cerr << "Faking query: " << filter.GetQuery() << std::endl;
  std::list<std::unique_ptr<RecordType>> results;
  for (const std::vector<std::string>& raw_result : raw_results) {
    const auto converted = SELECTOR.ConvertRow(raw_result);
    if (!converted) {
      std::cerr << "conversion failed" << std::endl;
    } else {
      results.emplace_back(new RecordType(*converted));
    }
  }
  return results;
}

using example_schema::ChainTable;
using example_schema::DataTable;

class Record {
 public:
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

  Record(const Selector::ColumnTypes& source)
      : key_(std::get<0>(source)),
        name_(std::get<1>(source)),
        data_(std::get<2>(source)) {}

  std::ostream& Print(std::ostream& out) const {
    return out << key_ << ") " << name_ << ": " << data_;
  }

 private:
  const int key_;
  const std::string name_;
  const std::string data_;
};

std::ostream& operator <<(std::ostream& out, const Record& record) {
  return record.Print(out);
}

int main() {
  static constexpr const_query::FilterBuilder<Record::Selector> B;
  const auto filter =
      B.And(
        B.Equals<0, ChainTable::KEY,
                 1, ChainTable::PARENT_KEY>(),
        B.Equals<0, ChainTable::KEY,
                 2, DataTable::KEY>());

  const auto records = PretendToExecuteQuery<Record>({
        std::vector<std::string>{ "1",   "One",    "x" },
        std::vector<std::string>{ "2",   "Two",    "y" },
        std::vector<std::string>{ "Bad", "Three",  "z" },
      }, filter);

  for (const auto& record : records) {
    std::cout << *record << std::endl;
  }
}
