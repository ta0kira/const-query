#ifndef FILTER_H_
#define FILTER_H_

#include <memory>
#include <sstream>
#include <tuple>

#include "query.h"
#include "selector.h"

namespace const_query {

template<class Select>
class Filter;

template<class Select>
class Predicate {};

template<class Queries, class Joins>
class Predicate<Selector<Queries, Joins>> {
 public:
  virtual std::ostream& SerializeTo(std::ostream& out) const = 0;
  virtual ~Predicate() = default;

  class And;
  class Or;

  template<int L, typename std::tuple_element<L, Queries>::type::TableType ColumnL,
           int R, typename std::tuple_element<R, Queries>::type::TableType ColumnR>
  class Equals;
};

template<class Select>
class Filter {
 public:
  virtual std::string GetQuery() const = 0;

 protected:
  Filter() = default;
  virtual ~Filter() = default;
};

template<class Select>
class EmptyFilter : public Filter<Select> {
 public:
  EmptyFilter() = default;
  virtual ~EmptyFilter() = default;

  virtual std::string GetQuery() const final {
    return Select().GetQuery();
  }
};

template<class Select>
class NonEmptyFilter : public Filter<Select> {
 public:
  std::string GetQuery() const final {
    std::ostringstream output;
    output << Select().GetQuery() << " WHERE " << *p_;
    return output.str();
  }

 private:
  NonEmptyFilter(std::unique_ptr<const Predicate<Select>> p) : p_(std::move(p)) {}

  std::unique_ptr<const Predicate<Select>> p_;
  template<class Select2> friend class Predicate;
};

template<class Queries, class Joins>
class Predicate<Selector<Queries, Joins>>::And : public Predicate<Selector<Queries, Joins>> {
 public:
  using FilterType = NonEmptyFilter<Selector<Queries, Joins>>;

  static FilterType New(FilterType l, FilterType r) {
    return FilterType{ std::unique_ptr<Predicate>(new And(std::move(l), std::move(r))) };
  }

  std::ostream& SerializeTo(std::ostream& out) const final {
    return out << "(" << *l_ << " AND " << *r_ << ")";
  }

 private:
  And(FilterType l, FilterType r) : l_(std::move(l.p_)), r_(std::move(r.p_)) {}

  const std::unique_ptr<const Predicate<Selector<Queries, Joins>>> l_, r_;
};

template<class Queries, class Joins>
class Predicate<Selector<Queries, Joins>>::Or : public Predicate<Selector<Queries, Joins>> {
 public:
  using FilterType = NonEmptyFilter<Selector<Queries, Joins>>;

  static FilterType New(FilterType l, FilterType r) {
    return FilterType(std::unique_ptr<Predicate>(new Or(std::move(l), std::move(r))));
  }

  std::ostream& SerializeTo(std::ostream& out) const final {
    return out << "(" << *l_ << " OR " << *r_ << ")";
  }

 private:
  Or(FilterType l, FilterType r) : l_(std::move(l.p_)), r_(std::move(r.p_)) {}

  const std::unique_ptr<const Predicate<Selector<Queries, Joins>>> l_, r_;
};

template<class Queries, class Joins>
template<int L, typename std::tuple_element<L, Queries>::type::TableType ColumnL,
         int R, typename std::tuple_element<R, Queries>::type::TableType ColumnR>
class Predicate<Selector<Queries, Joins>>::Equals : public Predicate<Selector<Queries, Joins>> {
 private:
  using ColumnEnumL = typename std::tuple_element<L, Queries>::type::TableType;
  using ColumnEnumR = typename std::tuple_element<R, Queries>::type::TableType;

 public:
  using FilterType = NonEmptyFilter<Selector<Queries, Joins>>;

  static FilterType New() {
    return FilterType(std::unique_ptr<Predicate>(new Equals));
  }

  std::ostream& SerializeTo(std::ostream& out) const final {
    return out << "("
                << Column<ColumnEnumL, ColumnL>::ColumnName(TableAlias<ColumnEnumL, L>::Get()) << " = "
                << Column<ColumnEnumR, ColumnR>::ColumnName(TableAlias<ColumnEnumR, R>::Get()) << ")";
  }

 private:
  Equals() {}
};

template<class Queries, class Joins>
std::ostream& operator <<(std::ostream& out, const Predicate<Selector<Queries, Joins>>& predicate) {
  return predicate.SerializeTo(out);
}

}  // namespace const_query

#endif  // FILTER_H_
