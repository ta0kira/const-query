#ifndef RECORD_H_
#define RECORD_H_

#include "selector.h"

namespace const_query {

template<class T, class Select>
class RecordBuilder {
 public:
  using ColumnTypes = typename Select::ColumnTypes;
  virtual T CreateFrom(const ColumnTypes& source) const = 0;

 protected:
  RecordBuilder() = default;
  virtual ~RecordBuilder() = default;
};

}  // namespace const_query

#endif  // RECORD_H_
