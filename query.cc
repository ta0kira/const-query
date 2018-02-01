#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

// Result Parsing

template <class QueryColumns, class ResultsTuple, int Offset>
struct DeserializeResult {
  static bool DeserializeAt(const QueryColumns& converters,
                            const std::vector<std::string>& values,
                            ResultsTuple* results) {
    assert(Offset <= values.size());
    assert(Offset > 0);
    const auto& converter = std::get<Offset-1>(converters);
    assert(converter);
    if (!converter->ParseFromString(values[Offset-1], &std::get<Offset-1>(*results))) {
      return false;
    } else {
      return DeserializeResult<QueryColumns, ResultsTuple, Offset-1>::DeserializeAt(converters, values, results);
    }
  }
};

template <class QueryColumns, class ResultsTuple>
struct DeserializeResult<QueryColumns, ResultsTuple, 0> {
  static bool DeserializeAt(const QueryColumns& converters,
                            const std::vector<std::string>& values,
                            ResultsTuple* results) {
    return true;
  }
};

// Column Names

template <class QueryColumns, class ColumnNames, int Offset>
struct TransposeColumns {
  static void TransposeAt(const QueryColumns& columns, ColumnNames* column_names) {
    std::get<Offset-1>(*column_names) = std::get<Offset-1>(columns)->ColumnName();
    TransposeColumns<QueryColumns, ColumnNames, Offset-1>::TransposeAt(columns, column_names);
  }
};

template <class QueryColumns, class ColumnNames>
struct TransposeColumns<QueryColumns, ColumnNames, 0> {
  static void TransposeAt(const QueryColumns& columns, ColumnNames* column_names) {}
};

// Homogeneous Tuple

template <class Base, int Count, class ... PrevTypes>
struct RepeatTimes {
  using Type = typename RepeatTimes<Base, Count-1, PrevTypes..., Base>::Type;
};

template <class Base, class ... PrevTypes>
struct RepeatTimes<Base, 0, PrevTypes...> {
  using Type = std::tuple<PrevTypes...>;
};

// Column Representation

template<class Type>
struct Column {
  virtual const std::string& ColumnName() const = 0;
  virtual bool ParseFromString(const std::string& value, Type* output) const = 0;
  virtual ~Column() = default;
};

// Query Representation

template <class ... Types>
class Query {
 private:
  using QueryColumns = std::tuple<std::unique_ptr<Column<Types>>...>;

 public:
  using ResultsTuple = std::tuple<Types...>;
  static constexpr int kResultSize = std::tuple_size<ResultsTuple>::value;
  using ColumnNames = typename RepeatTimes<std::string, kResultSize>::Type;

  ColumnNames GetColumns() const {
    return column_names_;
  }

  std::unique_ptr<ResultsTuple>
  ConvertRow(const std::vector<std::string>& values) const {
    auto results = std::unique_ptr<ResultsTuple>(new ResultsTuple);
    if (ConvertRow(values, results.get())) {
      return results;
    } else {
      return nullptr;
    }
  }

  bool ConvertRow(const std::vector<std::string>& values,
                  ResultsTuple* results) const {
    if (values.size() != kResultSize) {
      std::cout << "expected " << kResultSize << " elements but got "
                << values.size() << std::endl;
      return false;
    };
    return DeserializeResult<QueryColumns, ResultsTuple, kResultSize>::DeserializeAt(columns_, values, results);
  }

 protected:
  Query(std::unique_ptr<Column<Types>>... columns)
      : columns_(std::make_tuple(std::move(columns)...)),
        column_names_(CreateColumns(columns_)) {}

  ~Query() = default;

 private:
  static ColumnNames CreateColumns(const QueryColumns& columns) {
    ColumnNames column_names;
    TransposeColumns<QueryColumns, ColumnNames, kResultSize>::TransposeAt(columns, &column_names);
    return column_names;
  }

  Query(const Query&) = delete;
  Query(Query&&) = delete;
  Query& operator =(const Query&) = delete;
  Query& operator =(Query&&) = delete;

  const QueryColumns columns_;
  const ColumnNames column_names_;
};

// Specific Schema Support

struct MySchema {
 private:
  template <class Type>
  class MySchemaColumn;

  template <class Type>
  class SkippedColumn;

 public:
  static const MySchemaColumn<int>&         KEY;
  static const MySchemaColumn<std::string>& NAME;
  static const MySchemaColumn<double>&      TIME;
  static const MySchemaColumn<int>&         SKIP_KEY;
  static const MySchemaColumn<std::string>& SKIP_NAME;
  static const MySchemaColumn<double>&      SKIP_TIME;

  template <class ... Types>
  class MyQuery : public Query<Types...> {
   private:
    MyQuery(std::unique_ptr<Column<Types>>... columns)
        : Query<Types...>(std::move(columns)...) {}
    friend class MySchema;
  };

  template <class ... Types>
  static std::unique_ptr<MyQuery<Types...>> CreateQuery(const MySchemaColumn<Types>&... columns) {
    return std::unique_ptr<MyQuery<Types...>>(new MyQuery<Types...>(TransposeColumn(columns)...));
  }

 private:
  template<class Type>
  static std::unique_ptr<Column<Type>> TransposeColumn(const MySchemaColumn<Type>& column) {
    return column.GetCopy();
  }

  MySchema() = delete;
  MySchema(const MySchema&) = delete;
  MySchema(MySchema&&) = delete;
};

template <class Type>
class MySchema::MySchemaColumn : private Column<Type> {
 private:
  MySchemaColumn(const MySchemaColumn&) = default;
  MySchemaColumn(MySchemaColumn&&) = default;

  const std::string& ColumnName() const final {
    return name_;
  }

  bool ParseFromString(const std::string& value, Type* output) const override {
    std::istringstream stream(value);
    if ((stream >> *output) && stream.eof()) {
      return true;
    } else {
      std::cout << "failed to parse \"" << value << "\" as "
                << name_ << " (" << type_ << ")" << std::endl;
      return false;
    }
  }

  virtual std::unique_ptr<Column<Type>> GetCopy() const {
    return std::unique_ptr<Column<Type>>(new MySchemaColumn<Type>(*this));
  }

  MySchemaColumn(std::string name, std::string type)
      : name_(std::move(name)),
        type_(std::move(type)) {}

  const std::string name_;
  const std::string type_;

  friend class MySchema;
};

template <class Type>
class MySchema::SkippedColumn : public MySchema::MySchemaColumn<Type> {
 private:
  SkippedColumn(const SkippedColumn&) = default;
  SkippedColumn(SkippedColumn&&) = default;

  bool ParseFromString(const std::string& value, Type* output) const final {
    return true;
  }

  std::unique_ptr<Column<Type>> GetCopy() const override {
    return std::unique_ptr<Column<Type>>(new SkippedColumn<Type>(*this));
  }

  SkippedColumn() : MySchemaColumn<Type>("NULL", "") {}

  friend class MySchema;
};

const MySchema::MySchemaColumn<int>& MySchema::KEY =
    *new MySchemaColumn<int>("key", "int");

const MySchema::MySchemaColumn<std::string>& MySchema::NAME =
    *new MySchemaColumn<std::string>("name", "std::string");

const MySchema::MySchemaColumn<double>& MySchema::TIME =
    *new MySchemaColumn<double>("time", "double");

const MySchema::MySchemaColumn<int>& MySchema::SKIP_KEY =
    *new SkippedColumn<int>;

const MySchema::MySchemaColumn<std::string>& MySchema::SKIP_NAME =
    *new SkippedColumn<std::string>;

const MySchema::MySchemaColumn<double>& MySchema::SKIP_TIME =
    *new SkippedColumn<double>;

int main() {
  const auto query = MySchema::CreateQuery(MySchema::KEY,
                                           MySchema::NAME,
                                           MySchema::TIME);
  const auto columns = query->GetColumns();
  for (const auto raw_result :
        {
          std::vector<std::string>{ "1",   "One",    "0.1" },
          std::vector<std::string>{ "2",   "Two",    "0.2" },
          std::vector<std::string>{ "Bad", "Three",  "0.3" },
          std::vector<std::string>{ "4",   "Missing"       },
          std::vector<std::string>{ "5",   "Five",   "0.5", "Extra" },
          std::vector<std::string>{ "6",   "Six",    "0.6" },
        }) {
    const auto result = query->ConvertRow(raw_result);
    if (!result) {
      std::cout << "error parsing results" << std::endl;
    } else {
      std::cout << std::get<0>(columns) << ": " << std::get<0>(*result) << std::endl;
      std::cout << std::get<1>(columns) << ": " << std::get<1>(*result) << std::endl;
      std::cout << std::get<2>(columns) << ": " << std::get<2>(*result) << std::endl;
    }
  }
}
