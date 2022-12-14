#ifndef QUERY_H_
#define QUERY_H_

namespace const_query {

// Generic Query

template<class ColumnEnum, ColumnEnum ... Names>
class Query {
 public:
  // Used internally for meta-programming.
  using TableType = ColumnEnum;

  constexpr Query() = default;
  constexpr Query(const Query&) = default;
  constexpr Query(Query&&) = default;

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

}  // namespace const_query

#endif  // QUERY_H_
