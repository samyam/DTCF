#ifndef _TRANSP_H_
#define _TRANSP_H_

#include "timer.h"
#include "define.h"
#include "transp-kernel.h"
namespace DLTC{
inline vector<int>
position(const vector<IndexName>& x, const vector<IndexName>& y) {
  assert(x.size() == y.size());
  int dim = x.size();
  vector<int> pos(dim, 0);
  vector<int> tab(NIDX, 0);
  for (int i = 0; i < dim; i++) tab[y[i]] = i;
  for (int i = 0; i < dim; i++) pos[i] = tab[x[i]];
  return pos;
}

void
transpose(double* buf_x, const vector<IndexName>& x,
    double* buf_y, const vector<IndexName>& y);
}
#endif
