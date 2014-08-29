#ifndef _DEFINE_H_
#define _DEFINE_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cmath>
#include <stdint.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <iomanip>

#include "mpi.h"

#if TASCEL
#include "tascel-gapi.h"
#include "UniformTaskCollectionSplit.h"
typedef UniformTaskCollection TaskPool;
#else
#include "ga.h"
#include "macdecls.h"
#include "tc.h"
#define NUM_TASKS 1000000
typedef gtc_t TaskPool;
extern task_class_t TaskClass;
#endif
#include "param.h"
namespace DLTC{
using namespace std;




inline string int2str(int i) {
  string s;
  stringstream ss(s);
  ss << i;
  return ss.str();
}

inline IndexType typeOf(const IndexName& x) {
  if (0 <= x && x < NOCC) return HOL;
  else return PAR;
}

inline vector<int> getIdv(const vector<IndexName>& ids, const vector<int>& idvm) {
  int n = ids.size();
  vector<int> res(n, -1);
  for (int i = 0; i < n; i++) {
    res[i] = idvm[ids[i]];
  }
  return res;
}

inline vector<int> getWB(const vector<IndexName>& ids) {
  int n = ids.size();
  vector<int> res(n, 0);
  for (int i = 0; i < n; i++) {
    res[i] = typeOf(ids[i]) == HOL ? wbO : wbV;
  }
  return res;
}

inline vector<int> getSP(const vector<IndexName>& ids) {
  int n = ids.size();
  vector<int> res(n, 0);
  for (int i = 0; i < n; i++) {
    res[i] = typeOf(ids[i]) == HOL ? spO : spV;
  }
  return res;
}

// return number of blocks for each dimension
inline void getITUB(const vector<IndexName>& ids, vector<int>& itub) {
  int n = ids.size();
  for (int i = 0; i < n; i++) {
    itub[ids[i]] = typeOf(ids[i]) == HOL ? spO : spV;
  }
}

inline uint64_t getSize(const vector<IndexName>& ids) {
  //  vector<int> idv = getIdv(ids, idvm);
  uint64_t size = 1;
  for (int i = 0; i < ids.size(); i++) {
    size *= typeOf(ids[i]) == HOL ? wbO : wbV;
  }
  return size;
}
}
#endif
