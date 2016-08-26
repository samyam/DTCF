#include "loops.h"
namespace DLTC
{
bool
IndexLoops::hasNext(vector<int>& idvm) {
  vector<int> vec;
  if (next(vec)) {
    for (int i = 0; i < ids_.size(); ++i) {
      idvm[ids_[i]] = vec[i];
    }
    return true;
  }
  return false;
}

bool
IndexLoops::next(vector<int>& vec) {
  int n = curr_.size();
  if (curr_[0] >= hi_[0]) return false; // end of iteration
  vec = curr_; // output current
  ++curr_[n - 1]; // plus 1
  for (int i = n - 1; i > 0; --i) { // compute carry
    if (curr_[i] >= hi_[i]) {
      ++curr_[i - 1]; // carry
      for (int j = n - 1; j >= i; --j) { // reset lower
        curr_[j] = lo_[j];
      }
    }
  }
  return true;
}

IndexLoops::IndexLoops(const vector<IndexName>& ids, const vector<int>& lo, const vector<int>& hi) {
  assert(lo.size() > 0 && lo.size() == hi.size() && ids.size() == lo.size());
  ids_ = ids;
  lo_ = lo;
  hi_ = hi;
  curr_ = lo; // reset
}
}
