#ifndef EXAMPLE_H_
#define EXAMPLE_H_

#include <iostream>
#include <sstream>
#include <string>

#include "column_base.h"
#include "query.h"
#include "schema.h"

namespace example_schema {

enum class ChainTable {
  KEY,
  NAME,
  PARENT_KEY,
  DATA_KEY,
  COUNT_KEY,
};

enum class DataTable {
  KEY,
  DATA,
};

class ExampleDatabase {
  ExampleDatabase() = delete;
  ~ExampleDatabase() = delete;
};

}  // namespace example_schema

namespace const_query {

template<>
struct Database<::example_schema::ChainTable> {
  using Id = ::example_schema::ExampleDatabase;
};

template <>
struct Table<::example_schema::ChainTable> {
  static std::string TableName() { return "ChainTable"; }
};

template<>
struct Column<::example_schema::ChainTable, ::example_schema::ChainTable::KEY>
    : public SimpleColumn<int> {
  static std::string ColumnName(const std::string& table) {
    return WithTable(table, "Key");
  }
};

template<>
struct Column<::example_schema::ChainTable, ::example_schema::ChainTable::NAME>
    : public SimpleColumn<std::string> {
  static std::string ColumnName(const std::string& table) {
    return WithTable(table, "Name");
  }
};

template<>
struct Column<::example_schema::ChainTable, ::example_schema::ChainTable::PARENT_KEY>
    : public SimpleColumn<int> {
  static std::string ColumnName(const std::string& table) {
    return WithTable(table, "ParentKey");
  }
};

template<>
struct Column<::example_schema::ChainTable, ::example_schema::ChainTable::DATA_KEY>
    : public SimpleColumn<int> {
  static std::string ColumnName(const std::string& table) {
    return WithTable(table, "DataKey");
  }
};

template<>
struct Column<::example_schema::ChainTable, ::example_schema::ChainTable::COUNT_KEY>
    : public CountColumn {
  static std::string ColumnName(const std::string& table) {
    return WithTable(table, "Key");
  }
};

template<>
struct Database<::example_schema::DataTable> {
  using Id = ::example_schema::ExampleDatabase;
};

template <>
struct Table<::example_schema::DataTable> {
  static std::string TableName() { return "DataTable"; }
};

template<>
struct Column<::example_schema::DataTable, ::example_schema::DataTable::KEY>
    : public SimpleColumn<int> {
  static std::string ColumnName(const std::string& table) {
    return WithTable(table, "Key");
  }
};

template<>
struct Column<::example_schema::DataTable, ::example_schema::DataTable::DATA>
    : public SimpleColumn<std::string> {
  static std::string ColumnName(const std::string& table) {
    return WithTable(table, "Data");
  }
};

}  // namespace const_query

#endif  // EXAMPLE_H_
