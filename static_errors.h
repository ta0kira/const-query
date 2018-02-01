#ifndef STATIC_ERRORS_H_
#define STATIC_ERRORS_H_

namespace const_query {

template<class ColumnEnum> class Table;
template<class ColumnEnum, ColumnEnum Name> class Column;

namespace internal {
template<class Queries, class Joins, bool Complete> class JoinedSelector;
}  // namespace internal

namespace errors {

template<class Base>
struct SpecializeWithTypedef {
  SpecializeWithTypedef() = delete;
  ~SpecializeWithTypedef() = delete;
};

template<class Base, class Type>
class SpecializeWithStaticFunction {
 private:
  SpecializeWithStaticFunction() = default;
  ~SpecializeWithStaticFunction() = default;
  template<class ColumnEnum> friend class Table;
  template<class ColumnEnum, ColumnEnum Name> friend class Column;
};

template <int Count>
class YouNeedThisManyMoreJoins {
 private:
  YouNeedThisManyMoreJoins() = default;
  ~YouNeedThisManyMoreJoins() = default;
  template<class Queries, class Joins, bool Complete> friend class ::const_query::internal::JoinedSelector;
};

class YouAlreadyHaveEnoughJoins {
 private:
  YouAlreadyHaveEnoughJoins() = default;
  ~YouAlreadyHaveEnoughJoins() = default;
  template<class Queries, class Joins, bool Complete> friend class ::const_query::internal::JoinedSelector;
};

template <int Index>
struct YouMustJoinWithAnIndexLessThan {
  YouMustJoinWithAnIndexLessThan() = delete;
  ~YouMustJoinWithAnIndexLessThan() = delete;
};

struct YouMayNotHaveAnEmptySelector {
  YouMayNotHaveAnEmptySelector() = delete;
  ~YouMayNotHaveAnEmptySelector() = delete;
};

template<class Table1, class Database1, class Table2, class Database2>
struct TheseTablesAreNotInTheSameDatabase {
  TheseTablesAreNotInTheSameDatabase() = delete;
  ~TheseTablesAreNotInTheSameDatabase() = delete;
};

}  // namespace errors
}  // namespace const_query

#endif  // STATIC_ERRORS_H_
