#ifndef FILTER_H_
#define FILTER_H_

#include <memory>
#include <sstream>
#include <tuple>

#include "query.h"
#include "selector.h"

namespace const_query {

template<class Select>
class Filter {
 public:
  virtual std::string GetQuery() const = 0;

 private:
  Filter() = default;
  virtual ~Filter() = default;

  template<class Select2> friend class EmptyFilter;
  template<class Select2> friend class NonEmptyFilter;
};

template<class Select>
class EmptyFilter;

template<class Select>
class NonEmptyFilter;

template<class Select>
class FilterBuilder {
  FilterBuilder() = delete;
  ~FilterBuilder() = delete;
};

template<class Queries, class Joins>
class FilterBuilder<Selector<Queries, Joins>> {
 public:
  constexpr FilterBuilder() = default;

  using FilterType = NonEmptyFilter<Selector<Queries, Joins>>;

  FilterType And(FilterType l, FilterType r) const;

  FilterType Or(FilterType l, FilterType r) const;

  template<int L, typename std::tuple_element<L, Queries>::type::TableType ColumnL,
           int R, typename std::tuple_element<R, Queries>::type::TableType ColumnR>
  FilterType Equals() const;
};

template<class Select>
class EmptyFilter : public Filter<Select> {
 public:
  constexpr EmptyFilter() = default;

  virtual std::string GetQuery() const final {
    return Select().GetQuery();
  }
};

namespace internal {

template<class Select>
class Predicate;

} //  namespace internal

template<class Select>
class NonEmptyFilter : public Filter<Select> {
 public:
  std::string GetQuery() const final {
    std::ostringstream output;
    output << Select().GetQuery() << " WHERE " << *p_;
    return output.str();
  }

 private:
  NonEmptyFilter(std::unique_ptr<const internal::Predicate<Select>> p)
      : p_(std::move(p)) {}

  std::unique_ptr<const internal::Predicate<Select>> p_;

  template<class Select2> friend class FilterBuilder;
};

namespace internal {

template<class Select>
class Predicate {
 public:
  virtual std::ostream& SerializeTo(std::ostream& out) const = 0;
  virtual ~Predicate() = default;
};

template<class Queries, class Joins>
std::ostream& operator <<(std::ostream& out, const Predicate<Selector<Queries, Joins>>& predicate) {
  return predicate.SerializeTo(out);
}

template<class Queries, class Joins>
class PredicateAnd
    : public Predicate<Selector<Queries, Joins>> {
 public:
  std::ostream& SerializeTo(std::ostream& out) const final {
    return out << "(" << *l_ << " AND " << *r_ << ")";
  }

 private:
  PredicateAnd(std::unique_ptr<const Predicate<Selector<Queries, Joins>>> l,
               std::unique_ptr<const Predicate<Selector<Queries, Joins>>> r)
      : l_(std::move(l)), r_(std::move(r)) {}

  const std::unique_ptr<const Predicate<Selector<Queries, Joins>>> l_, r_;

  template <class Select> friend class FilterBuilder;
};

template<class Queries, class Joins>
class PredicateOr
    : public Predicate<Selector<Queries, Joins>> {
 public:
  std::ostream& SerializeTo(std::ostream& out) const final {
    return out << "(" << *l_ << " OR " << *r_ << ")";
  }

 private:
  PredicateOr(std::unique_ptr<const Predicate<Selector<Queries, Joins>>> l,
              std::unique_ptr<const Predicate<Selector<Queries, Joins>>> r)
      : l_(std::move(l)), r_(std::move(r)) {}

  const std::unique_ptr<const Predicate<Selector<Queries, Joins>>> l_, r_;

  template <class Select> friend class FilterBuilder;
};

template<class Queries, class Joins,
         int L, typename std::tuple_element<L, Queries>::type::TableType ColumnL,
         int R, typename std::tuple_element<R, Queries>::type::TableType ColumnR>
class PredicateEquals
    : public Predicate<Selector<Queries, Joins>> {
 private:
  using ColumnEnumL = typename std::tuple_element<L, Queries>::type::TableType;
  using ColumnEnumR = typename std::tuple_element<R, Queries>::type::TableType;

 public:
  std::ostream& SerializeTo(std::ostream& out) const final {
    return out << "("
                << Column<ColumnEnumL, ColumnL>::ColumnName(TableAlias<ColumnEnumL, L>::Get()) << " = "
                << Column<ColumnEnumR, ColumnR>::ColumnName(TableAlias<ColumnEnumR, R>::Get()) << ")";
  }

 private:
  PredicateEquals() = default;

  template <class Select> friend class FilterBuilder;
};

} //  namespace internal

template<class Queries, class Joins>
typename FilterBuilder<Selector<Queries, Joins>>::FilterType
FilterBuilder<Selector<Queries, Joins>>::And(FilterType l, FilterType r) const {
  return FilterType(
      std::unique_ptr<const internal::Predicate<Selector<Queries, Joins>>>(
          new internal::PredicateAnd<Queries, Joins>(std::move(l.p_), std::move(r.p_))));
}

template<class Queries, class Joins>
typename FilterBuilder<Selector<Queries, Joins>>::FilterType
FilterBuilder<Selector<Queries, Joins>>::Or(FilterType l, FilterType r) const {
  return FilterType(
      std::unique_ptr<const internal::Predicate<Selector<Queries, Joins>>>(
          new internal::PredicateOr<Queries, Joins>(std::move(l.p_), std::move(r.p_))));
}

template<class Queries, class Joins>
template<int L, typename std::tuple_element<L, Queries>::type::TableType ColumnL,
         int R, typename std::tuple_element<R, Queries>::type::TableType ColumnR>
typename FilterBuilder<Selector<Queries, Joins>>::FilterType
FilterBuilder<Selector<Queries, Joins>>::Equals() const {
  return FilterType(
      std::unique_ptr<const internal::Predicate<Selector<Queries, Joins>>>(
          new internal::PredicateEquals<Queries, Joins, L, ColumnL, R, ColumnR>()));
}

}  // namespace const_query

#endif  // FILTER_H_
