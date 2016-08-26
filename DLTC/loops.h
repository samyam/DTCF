#ifndef _LOOPS_H_
#define _LOOPS_H_

#include <cassert>
#include <vector>

#include "param.h"
namespace DLTC{
using namespace std;

class IndexLoops {
  private:
    vector<IndexName> ids_; /* index names */
    vector<int> lo_;        /* lower bound of ids */
    vector<int> hi_;        /* upper bound of ids */
    vector<int> curr_;      /* current value of ids */
  public:
    IndexLoops(const vector<IndexName>& ids, const vector<int>& lo, const vector<int>& hi);
    ~IndexLoops() {};
    bool next(vector<int>& vec);
    bool hasNext(vector<int>& idvm); /* all indices value mapping */
};
}
#endif
