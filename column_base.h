#ifndef COLUMN_BASE_H_
#define COLUMN_BASE_H_

namespace const_query {

template<class ValueType>
class SimpleColumn {
 public:
  // Used internally for meta-programming.
  using ColumnType = ValueType;

  static bool ParseFromString(const std::string& value, ColumnType* output) {
    std::istringstream stream(value);
    if ((stream >> *output) && stream.eof()) {
      return true;
    } else {
      std::cerr << "failed to parse \"" << value << "\"" << std::endl;
      return false;
    }
  }

 protected:
  static std::string WithTable(const std::string& table, const std::string& name) {
    if (table.empty()) {
      return name;
    } else {
      std::ostringstream output;
      output << table << "." << name;
      return output.str();
    }
  }
};

class CountColumn {
 public:
  // Used internally for meta-programming.
  using ColumnType = int;

  static bool ParseFromString(const std::string& value, int* output) {
    std::istringstream stream(value);
    if ((stream >> *output) && stream.eof()) {
      return true;
    } else {
      std::cerr << "failed to parse \"" << value << "\"" << std::endl;
      return false;
    }
  }

 protected:
  static std::string WithTable(const std::string& table, const std::string& name) {
    std::ostringstream output;
    if (table.empty()) {
      output << "COUNT(" << name << ")";
    } else {
      output << "COUNT(" << table << "." << name << ")";
    }
    return output.str();
  }
};

}  // namespace const_query

#endif  // COLUMN_BASE_H_
