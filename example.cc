#include <iostream>
#include <string>
#include <vector>
#include <tuple>

#include "example.h"

#include "selector.h"

using example_schema::ChainTable;
using example_schema::DataTable;

int main() {
  static constexpr auto selector = const_query::Select(
    const_query::Query<ChainTable>()
      .Get<ChainTable::COUNT_KEY>()
      .Get<ChainTable::NAME>(),
    const_query::Query<ChainTable>(),
    const_query::Query<DataTable>()
      .Get<DataTable::DATA>())
    .JoinNextOn<0, ChainTable::PARENT_KEY,
                   ChainTable::KEY>()
    .JoinNextOn<1, ChainTable::DATA_KEY,
                   DataTable::KEY>();

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
      std::cout << column_names[0] << ": " << std::get<0>(*converted) << std::endl;
      std::cout << column_names[1] << ": " << std::get<1>(*converted) << std::endl;
      std::cout << column_names[2] << ": " << std::get<2>(*converted) << std::endl;
    }
  }
}
