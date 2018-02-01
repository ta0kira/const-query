#ifndef EXAMPLE_H_
#define EXAMPLE_H_

#include <iostream>
#include <sstream>
#include <string>

#include "column_base.h"
#include "query.h"
#include "schema.h"

class ExampleDatabase {
  ExampleDatabase() = delete;
  ~ExampleDatabase() = delete;
};

class ChainTable {
 public:
  enum class ColumnName {
    KEY,
    NAME,
    PARENT_KEY,
    DATA_KEY,
    COUNT_KEY,
  };

  static constexpr const_query::Query<ColumnName> EmptyQuery() {
    return const_query::Query<ColumnName>();
  }

  ChainTable() = delete;
  ~ChainTable() = delete;
};

namespace const_query {

template<>
struct Database<ChainTable::ColumnName> {
  using Id = ExampleDatabase;
};

template <>
struct Table<ChainTable::ColumnName> {
  static std::string TableName() { return "ChainTable"; }
};

template<>
struct Column<ChainTable::ColumnName, ChainTable::ColumnName::KEY>
    : public SimpleColumn<int> {
  static std::string ColumnName(const std::string& table) {
    return WithTable(table, "Key");
  }
};

template<>
struct Column<ChainTable::ColumnName, ChainTable::ColumnName::NAME>
    : public SimpleColumn<std::string> {
  static std::string ColumnName(const std::string& table) {
    return WithTable(table, "Name");
  }
};

template<>
struct Column<ChainTable::ColumnName, ChainTable::ColumnName::PARENT_KEY>
    : public SimpleColumn<int> {
  static std::string ColumnName(const std::string& table) {
    return WithTable(table, "ParentKey");
  }
};

template<>
struct Column<ChainTable::ColumnName, ChainTable::ColumnName::DATA_KEY>
    : public SimpleColumn<int> {
  static std::string ColumnName(const std::string& table) {
    return WithTable(table, "DataKey");
  }
};

template<>
struct Column<ChainTable::ColumnName, ChainTable::ColumnName::COUNT_KEY>
    : public CountColumn {
  static std::string ColumnName(const std::string& table) {
    return WithTable(table, "Key");
  }
};

}  // namespace const_query

class DataTable {
 public:
  enum class ColumnName {
    KEY,
    DATA,
  };

  static constexpr const_query::Query<ColumnName> EmptyQuery() {
    return const_query::Query<ColumnName>();
  }

  DataTable() = delete;
  ~DataTable() = delete;
};

namespace const_query {

template<>
struct Database<DataTable::ColumnName> {
  using Id = ExampleDatabase;
};

template <>
struct Table<DataTable::ColumnName> {
  static std::string TableName() { return "DataTable"; }
};

template<>
struct Column<DataTable::ColumnName, DataTable::ColumnName::KEY>
    : public SimpleColumn<int> {
  static std::string ColumnName(const std::string& table) {
    return WithTable(table, "Key");
  }
};

template<>
struct Column<DataTable::ColumnName, DataTable::ColumnName::DATA>
    : public SimpleColumn<std::string> {
  static std::string ColumnName(const std::string& table) {
    return WithTable(table, "Data");
  }
};

}  // namespace const_query

#endif  // EXAMPLE_H_
