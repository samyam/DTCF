#ifndef _PROFILER_H_
#define _PROFILER_H_

#include <map>
#include <vector>
using namespace std;

class Profiler {
private:
  typedef map<pair<int, int>, pair<int, int> > Records;
  static Records* recs;
  // < <tensorID, tileIndex>, <freq, dim> >

  static map<int, int>* reuse;
public:
  static inline void init() {
    recs = new map<pair<int, int>, pair<int, int> >();
    reuse = new map<int, int>();
  }
  static inline void close() {
    delete recs;
    delete reuse;
    recs = 0;
    reuse = 0;
  }
  static inline void inc(int tid, int tile, int dim) {
    pair<int, int> key = make_pair(tid, tile);
    if (recs->find(key) == recs->end()) {
      pair<int, int> val = make_pair(1, dim);
      (*recs)[key] = val;
    }
    else {
      (*recs)[key].first++;
    }
  }
  static inline int getFreq(int tid, int tile) {
    pair<int, int> key = make_pair(tid, tile);
    if (recs->find(key) == recs->end()) return 0;
    else return (*recs)[key].first;
  }
  static inline int getDim(int tid, int tile) {
    pair<int, int> key = make_pair(tid, tile);
    if (recs->find(key) == recs->end()) return 0;
    else return (*recs)[key].second;
  }
  static inline int nKeys() {
    int num = 0;
    for (Records::iterator it = recs->begin(); it != recs->end(); ++it) num++;
    return num;
  }
  static inline int nKeys(int dim) {
    int num = 0;
    for (Records::iterator it = recs->begin(); it != recs->end(); ++it) {
      if (it->second.second == dim) num++;
    }
    return num;
  }
  static inline map<int, int> getRawReuseInfo(int dim) {
    map<int, int> res;
    for (Records::iterator it = recs->begin(); it != recs->end(); ++it) {
      pair<int, int>& val = it->second;
      if (val.second == dim) {
        if (res.find(val.first) == res.end()) res[val.first] = 1;
        else res[val.first]++;
      }
    }
    return res;
  }
  static inline vector<double> getReuseInfo(int dim, int fmax) {
    vector<double> res(fmax + 1, 0.0);
    for (Records::iterator it = recs->begin(); it != recs->end(); ++it) {
      pair<int, int>& val = it->second;
      if (val.second == dim) {
        if (val.first >= fmax) res[fmax]++;
        else res[val.first]++;
      }
    }
    return res;
  }
};

#endif
