#include <iostream>
#include <string>
#include <vector>
#include <tuple>

#include "example.h"

#include "selector.h"

using example_schema::ChainTable;
using example_schema::DataTable;

class Record {
 public:
  using Selector =
      decltype(const_query::Select(
                 const_query::Query<ChainTable>()
                   .Get<ChainTable::KEY>()
                   .Get<ChainTable::NAME>(),
                 const_query::Query<ChainTable>(),
                 const_query::Query<DataTable>()
                   .Get<DataTable::DATA>())
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
  static constexpr Record::Selector selector;

  std::cout << selector.GetQuery() << std::endl;

  const auto column_names = selector.GetColumnNames();

  for (const auto raw_result :
        {
          std::vector<std::string>{ "1",   "One",    "x" },
          std::vector<std::string>{ "2",   "Two",    "y" },
          std::vector<std::string>{ "Bad", "Three",  "z" },
        }) {
    const auto converted = selector.ConvertRow(raw_result);
    if (!converted) {
      std::cerr << "conversion failed" << std::endl;
    } else {
      std::cout << Record(*converted) << std::endl;
    }
  }
}
