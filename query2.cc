#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include <list>

// template <class Base, int Count, class ... PrevTypes>
// struct RepeatTimes {
//   using Type = typename RepeatTimes<Base, Count-1, PrevTypes..., Base>::Type;
// };
//
// template <class Base, class ... PrevTypes>
// struct RepeatTimes<Base, 0, PrevTypes...> {
//   using Type = std::tuple<PrevTypes...>;
// };
//
template<class Type>
struct Column {
  using ValueType = Type;
  virtual const std::string& ColumnName() const = 0;
  virtual bool ParseFromString(const std::string& value, ValueType* output) const = 0;
  virtual ~Column() = default;
};
//
// template <class ... Types>
// struct ColumnsFromValueTypes {
//   using Type = std::tuple<std::unique_ptr<Column<typename Types::ValueType>>...>;
// };
//
// template <class Tuple, int Offset>
// struct CollectColumns {
//   template <class NewTuple>
//   static void ForOffset(NewTuple* tuple) {
//     using RealType = typename std::tuple_element<Offset-1, Tuple>::type;
//     std::get<Offset-1>(*tuple) = std::unique_ptr<RealType>(new RealType);
//     CollectColumns<Tuple, Offset-1>::ForOffset(tuple);
//   }
// };
//
// template <class Tuple>
// struct CollectColumns<Tuple, 0> {
//   template <class NewTuple>
//   static void ForOffset(NewTuple* tuple) {}
// };

template <class Type>
class SimpleColumn : public Column<Type> {
 public:
  const std::string& ColumnName() const override {
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

 protected:
  SimpleColumn(std::string name, std::string type)
      : name_(std::move(name)),
        type_(std::move(type)) {}

  virtual ~SimpleColumn() = default;

 private:
  SimpleColumn(const SimpleColumn&) = default;
  SimpleColumn(SimpleColumn&&) = default;

  const std::string name_;
  const std::string type_;
};

template<class ColumnEnum, ColumnEnum ... Names>
class Query {
 public:
  using TableType = ColumnEnum;

  constexpr Query() = default;

  template<ColumnEnum Name>
  constexpr Query<ColumnEnum, Names..., Name> Get() const {
    return Query<ColumnEnum, Names..., Name>();
  }

  constexpr bool operator == (const Query& other) const {
    return true;
  }

  template<class ColumnEnum2, ColumnEnum2 ... Names2>
  constexpr bool operator == (const Query<ColumnEnum2, Names2...>& other) const {
    return false;
  }

  template<class ColumnEnum2, ColumnEnum2 ... Names2>
  constexpr bool operator != (const Query<ColumnEnum2, Names2...>& other) const {
    return !(*this == other);
  }
};

// template <class Query, class Join, class Tuple>
// struct ExtendParts {};
//
// template <class Query, class Join, class ... Types>
// struct ExtendParts<Query, Join, std::tuple<Types...>> {
//   using TYPE = std::tuple<Query, Join, Types...>;
// };
//
// template <class Enum, Enum>
// class Joiner {};
//
// template <int Table, class EnumL, EnumL, class EnumR, EnumR>
// class Join {};
//
// template <int Table, class EnumL, EnumL L, class EnumR, EnumR R>
// constexpr Join<Table, EnumL, L, EnumR, R> AutoJoin(Joiner<EnumL, L> l, Joiner<EnumR, R> r) {
//   return Join<Table, EnumL, L, EnumR, R>();
// }
//
// template <class ... Types>
// struct Select  {
//   Select() = delete;
//   ~Select() = delete;
// };
//
// template <>
// struct Select<> {
//   using Parts = std::tuple<>;
//
//   constexpr Parts GetParts() const {
//     return Parts();
//   }
// };
//
// template <class Query>
// struct Select<Query> {
//   using Parts = std::tuple<Query>;
//
//   constexpr Parts GetParts() const {
//     return Parts();
//   }
// };

// template <class SchemaL, SchemaL ... Ls,
//           int TableQ, class SchemaQ, SchemaQ Q,
//           class SchemaR, SchemaR R, SchemaR ... Rs,
//           class ... Types>
// struct Select<Query<SchemaL, Ls...>,
//               Join<TableQ, SchemaQ, Q, SchemaR, R>,
//               Query<SchemaR, Rs...>,
//               Types...> {
//  private:
//   using QueryL = Query<SchemaL, Ls...>;
//   using JoinQR = Join<TableQ, SchemaQ, Q, SchemaR, R>;
//   using QueryR = Query<SchemaR, Rs...>;
//   using Next = Select<QueryR, Types...>;
//
//  public:
//   using Parts = typename ExtendParts<QueryL, JoinQR, typename Next::Parts>::TYPE;
//
//   constexpr Parts GetParts() const {
//     return Parts();
//   }
// };
//
// template <class ... Types>
// constexpr Select<Types...> AutoSelect(Types... types) {
//   return Select<Types...>();
// }

template<class ColumnEnum, ColumnEnum Name>
class ColumnType {};

template <class ColumnEnum>
class Table {};

class MySchema {
 public:
  enum class ColumnName {
    KEY,
    NAME,
    TIME,
  };

  static constexpr Query<ColumnName> EmptyQuery() {
    return Query<ColumnName>();
  }

//   template<ColumnName Name>
//   static constexpr Joiner<ColumnName, Name> JoinOn() {
//     return Joiner<ColumnName, Name>();
//   }

//   template<ColumnName ... Names>
//   static typename ColumnsFromValueTypes<ColumnType<ColumnName, Names>...>::Type
//   CreateColumns(Query<ColumnName, Names...> query) {
//     using Columns = typename ColumnsFromValueTypes<ColumnType<ColumnName, Names>...>::Type;
//     Columns columns;
//     CollectColumns<std::tuple<ColumnType<ColumnName, Names>...>,
//                     std::tuple_size<Columns>::value>::ForOffset(&columns);
//     return columns;
//   }
};

template <>
class Table<MySchema::ColumnName> {
 public:
  static const char TABLE_NAME[];
};
const char Table<MySchema::ColumnName>::TABLE_NAME[] = "MySchema";

template<>
class ColumnType<MySchema::ColumnName, MySchema::ColumnName::KEY> : public SimpleColumn<int> {
 public:
  ColumnType()
      : SimpleColumn<int>("Key", "int") {}
//   template <class Tuple, int Offset> friend class CollectColumns;
};

template<>
class ColumnType<MySchema::ColumnName, MySchema::ColumnName::NAME> : public SimpleColumn<std::string> {
 public:
  ColumnType()
      : SimpleColumn<std::string>("Name", "std::string") {}
//   template <class Tuple, int Offset> friend class CollectColumns;
};

template<>
class ColumnType<MySchema::ColumnName, MySchema::ColumnName::TIME> : public SimpleColumn<double> {
 public:
  ColumnType()
      : SimpleColumn<double>("Time", "double") {}
//   template <class Tuple, int Offset> friend class CollectColumns;
};

template<bool P>
struct TF {};

template<>
struct TF<false> {
  static const char VALUE[];
};
const char TF<false>::VALUE[] = "FALSE";

template<>
struct TF<true> {
  static const char VALUE[];
};
const char TF<true>::VALUE[] = "TRUE";

std::string GetTableName(int counter) {
  std::ostringstream output;
  output << "t" << counter;
  return output.str();
}

// template <class ColumnEnum, ColumnEnum ... Columns>
// struct AppendColumns {};
//
// template <class ColumnEnum, ColumnEnum Column, ColumnEnum ... Columns>
// struct AppendColumns<ColumnEnum, Column, Columns...> {
//   static void To(const std::string& table_name, std::ostringstream* output) {
//     const auto column = std::unique_ptr<ColumnType<ColumnEnum, Column>>(new ColumnType<ColumnEnum, Column>);
//     *output << table_name << "." << column->ColumnName() << ", ";
//     AppendColumns<ColumnEnum, Columns...>::To(table_name, output);
//   }
// };
//
// template <class ColumnEnum>
// struct AppendColumns<ColumnEnum> {
//   static void To(const std::string& table_name, std::ostringstream* output) {}
// };
//
// template <class Selector>
// struct BuildQuery {};
//
// template <class ColumnEnum, ColumnEnum ... Columns, class ... Types>
// struct BuildQuery<Select<Query<ColumnEnum, Columns...>, Types...>> {
//   static void SelectTo(int* table_counter, std::ostringstream* output) {
//     const std::string table_name = GetTableName((*table_counter)++);
//     AppendColumns<ColumnEnum, Columns...>::To(table_name, output);
//     BuildQuery<Select<Types...>>::SelectTo(table_counter, output);
//   }
//
//   static void JoinTo(int* table_counter, std::ostringstream* output) {
//     const std::string table_name = GetTableName((*table_counter)++);
//     *output << " FROM " << Table<ColumnEnum>::TABLE_NAME
//             << " AS " << table_name;
//     BuildQuery<Select<Types...>>::JoinTo(table_counter, output);
//   }
// };
//
// template <class ColumnEnumL, ColumnEnumL L, int TableL,
//           class ColumnEnumR, ColumnEnumR R, ColumnEnumR ... Rs,
//           class ... Types>
// struct BuildQuery<Select<Join<TableL, ColumnEnumL, L, ColumnEnumR, R>, Query<ColumnEnumR, Rs...>, Types...>> {
//   static void SelectTo(int* table_counter, std::ostringstream* output) {
//     const std::string table_name = GetTableName((*table_counter)++);
//     AppendColumns<ColumnEnumR, Rs...>::To(table_name, output);
//     BuildQuery<Select<Types...>>::SelectTo(table_counter, output);
//   }
//
//   static void JoinTo(int* table_counter, std::ostringstream* output) {
//     const auto column_l = std::unique_ptr<ColumnType<ColumnEnumL, L>>(new ColumnType<ColumnEnumL, L>);
//     const auto column_r = std::unique_ptr<ColumnType<ColumnEnumR, R>>(new ColumnType<ColumnEnumR, R>);
//     const std::string table_name_r = GetTableName((*table_counter)++);
//     const std::string table_name_l = GetTableName(TableL);
//     *output << " JOIN " << Table<ColumnEnumR>::TABLE_NAME
//             << " AS " << table_name_r << " ON ("
//             << table_name_l << "." << column_l->ColumnName() << " == "
//             << table_name_r << "." << column_r->ColumnName() << ")";
//     BuildQuery<Select<Types...>>::JoinTo(table_counter, output);
//   }
// };
//
// template <>
// struct BuildQuery<Select<>> {
//   static void SelectTo(int* table_counter, std::ostringstream* output) {}
//   static void JoinTo(int* table_counter, std::ostringstream* output) {}
// };
//
// template <class Selector>
// void AutoSelectTo(Selector selector, std::ostringstream* output) {
//   int table_counter = 0;
//   BuildQuery<Selector>::SelectTo(&table_counter, output);
//   table_counter = 0;
//   BuildQuery<Selector>::JoinTo(&table_counter, output);
// }

template <class Query, class Tuple>
struct ExtendParts {};

template <class Query, class ... Types>
struct ExtendParts<Query, std::tuple<Types...>> {
  using Parts = std::tuple<Query, Types...>;
};

template <class ColumnEnum, ColumnEnum Column>
struct Joiner {};

struct JoinValue {
  const int table_left;
  const std::string column_left;
  const int table_right;
  const std::string column_right;
};

template<class ... Queries>
struct Selector {
  Selector() = delete;
  ~Selector() = delete;
};

template<>
struct Selector<> {
 private:
  using Parts = std::tuple<>;
  template <class ... Queries2> friend class Selector;
};

template<class ColumnEnum, ColumnEnum ... Columns, class ... Queries>
struct Selector<Query<ColumnEnum, Columns...>, Queries...> {
 private:
  using Parts = typename ExtendParts<Query<ColumnEnum, Columns...>,
                                     typename Selector<Queries...>::Parts>::Parts;
  template <class ... Queries2> friend class Selector;

 public:
  template<int L, typename std::tuple_element<L, Parts>::type::TableType ColumnL,
           int R, typename std::tuple_element<R, Parts>::type::TableType ColumnR>
  Selector& JoinOn() {
    joins.push_back(JoinValue{L, "", R, ""});
    return *this;
  }

 private:
  std::list<JoinValue> joins;
};

template<class ... Queries>
Selector<Queries...> Select(Queries ... queries) {
  return Selector<Queries...>();
}

int main() {
  constexpr auto query = MySchema::EmptyQuery()
      .Get<MySchema::ColumnName::KEY>()
      .Get<MySchema::ColumnName::NAME>()
      .Get<MySchema::ColumnName::TIME>();

  constexpr auto query2 = Query<MySchema::ColumnName,
                                MySchema::ColumnName::KEY,
                                MySchema::ColumnName::NAME,
                                MySchema::ColumnName::TIME>();

  std::cout << TF<query == query2>::VALUE << std::endl;

//   const auto columns = MySchema::CreateColumns(query2);
//
//   for (const auto raw_result :
//         {
//           std::vector<std::string>{ "1",   "One",    "0.1" },
//           std::vector<std::string>{ "2",   "Two",    "0.2" },
//           std::vector<std::string>{ "Bad", "Three",  "0.3" },
//         }) {
//     int x;
//     std::get<0>(columns)->ParseFromString(raw_result[0], &x);
//     std::cout << std::get<0>(columns)->ColumnName() << ": " << x << std::endl;
//
//     std::string y;
//     std::get<1>(columns)->ParseFromString(raw_result[1], &y);
//     std::cout << std::get<1>(columns)->ColumnName() << ": " << y << std::endl;
//
//     double z;
//     std::get<2>(columns)->ParseFromString(raw_result[2], &z);
//     std::cout << std::get<2>(columns)->ColumnName() << ": " << z << std::endl;
//   }



  auto selector = Select(
    MySchema::EmptyQuery()
      .Get<MySchema::ColumnName::KEY>()
      .Get<MySchema::ColumnName::TIME>(),
    MySchema::EmptyQuery()
      .Get<MySchema::ColumnName::KEY>()
      .Get<MySchema::ColumnName::NAME>())
    .JoinOn<0, MySchema::ColumnName::KEY,
            1, MySchema::ColumnName::NAME>();
//
//   std::ostringstream output;
//   AutoSelectTo(selector, &output);
//   std::cout << output.str() << std::endl;
}
