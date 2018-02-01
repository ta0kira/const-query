#ifndef COLLECTION_H_
#define COLLECTION_H_

#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "query.h"
#include "schema.h"

// Converting Result Data

template<class Columns, int Offset>
struct ConvertAll {
  template<class Tuple>
  static bool At(const std::vector<std::string>& values, Tuple* output) {
    if (values.size() < Offset) {
      std::cerr << "not enough elements" << std::endl;
      return false;
    }
    if (!std::tuple_element<Offset-1, Columns>::type::ParseFromString(values[Offset-1], &std::get<Offset-1>(*output))) {
      return false;
    } else {
      return ConvertAll<Columns, Offset-1>::At(values, output);
    }
  }
};

template<class Columns>
struct ConvertAll<Columns, 0> {
  template<class Tuple>
  static bool At(const std::vector<std::string>& values, Tuple* output) {
    return true;
  }
};

// Collecting Table Names from Queries

template <int Count, class Tuple>
struct CollectTableNames {};

template <int Count, class ColumnEnum, ColumnEnum ... Names, class ... Queries>
struct CollectTableNames<Count, std::tuple<Query<ColumnEnum, Names...>, Queries...>> {
  static void To(std::vector<std::string>* output) {
    output->push_back(TableAlias<ColumnEnum, Count>::Define());
    CollectTableNames<Count+1, std::tuple<Queries...>>::To(output);
  }
};

template <int Count>
struct CollectTableNames<Count, std::tuple<>> {
  static void To(std::vector<std::string>* output) {}
};

// Collecting Aliased Columns from Queries

template <class ColumnEnum, ColumnEnum ... Names>
struct TransposeColumns {};

template <class ColumnEnum, ColumnEnum Name, ColumnEnum ... Names>
struct TransposeColumns<ColumnEnum, Name, Names...> {
  static void To(const std::string& table_name, std::vector<std::string>* output) {
    output->push_back(Column<ColumnEnum, Name>::ColumnName(table_name));
    TransposeColumns<ColumnEnum, Names...>::To(table_name, output);
  }
};

template <class ColumnEnum>
struct TransposeColumns<ColumnEnum> {
  static void To(const std::string& table_name, std::vector<std::string>* output) {}
};

template <int Count, class Tuple>
struct CollectTableColumns {};

template <int Count, class ColumnEnum, ColumnEnum ... Names, class ... Queries>
struct CollectTableColumns<Count, std::tuple<Query<ColumnEnum, Names...>, Queries...>> {
  static void To(std::vector<std::string>* output) {
    TransposeColumns<ColumnEnum, Names...>::To(TableAlias<ColumnEnum, Count>::Get(), output);
    CollectTableColumns<Count+1, std::tuple<Queries...>>::To(output);
  }
};

template <int Count>
struct CollectTableColumns<Count, std::tuple<>> {
  static void To(std::vector<std::string>* output) {}
};

// Collect Join Information

template <class Tuple>
struct CollectJoins {};

template <class Join, class ... Joins>
struct CollectJoins<std::tuple<Join, Joins...>> {
  static void To(std::vector<std::string>* output) {
    output->push_back(Join::ToString());
    CollectJoins<std::tuple<Joins...>>::To(output);
  }
};

template <>
struct CollectJoins<std::tuple<>> {
  static void To(std::vector<std::string>* output) {}
};

#endif  // COLLECTION_H_
