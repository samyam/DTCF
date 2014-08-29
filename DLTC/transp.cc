#include "transp.h"
namespace DLTC{
void
transpose(double* buf_x, const vector<IndexName>& x,
    double* buf_y, const vector<IndexName>& y) { // x->y

#if TIMER
  MyTimer::start(TSP);
#endif

  assert(x.size() == y.size());
  int dim = x.size();
  vector<int> s = position(y,x);
  vector<int> d = getWB(x); // block width of x
  if (dim == 2) {
    if (s[0] == 0) buf_y = buf_x;
    else transpose_ji(buf_x, buf_y, d[0], d[1]);
  }
  else if (dim == 4) {
    int di = d[0];
    int dj = d[1];
    int dk = d[2];
    int dl = d[3];
    if      (s[0] == 0 && s[1] == 1 && s[2] == 2 && s[3] == 3) buf_y = buf_x; //transpose_ijkl(buf_x,buf_y,di,dj,dk,dl);
    else if (s[0] == 0 && s[1] == 1 && s[2] == 3 && s[3] == 2) transpose_ijlk(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 0 && s[1] == 2 && s[2] == 1 && s[3] == 3) transpose_ikjl(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 0 && s[1] == 2 && s[2] == 3 && s[3] == 1) transpose_iklj(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 0 && s[1] == 3 && s[2] == 1 && s[3] == 2) transpose_iljk(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 0 && s[1] == 3 && s[2] == 2 && s[3] == 1) transpose_ilkj(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 1 && s[1] == 0 && s[2] == 2 && s[3] == 3) transpose_jikl(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 1 && s[1] == 0 && s[2] == 3 && s[3] == 2) transpose_jilk(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 1 && s[1] == 2 && s[2] == 0 && s[3] == 3) transpose_jkil(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 1 && s[1] == 2 && s[2] == 3 && s[3] == 0) transpose_jkli(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 1 && s[1] == 3 && s[2] == 0 && s[3] == 2) transpose_jlik(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 1 && s[1] == 3 && s[2] == 2 && s[3] == 0) transpose_jlki(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 2 && s[1] == 0 && s[2] == 1 && s[3] == 3) transpose_kijl(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 2 && s[1] == 0 && s[2] == 3 && s[3] == 1) transpose_kilj(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 2 && s[1] == 1 && s[2] == 0 && s[3] == 3) transpose_kjil(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 2 && s[1] == 1 && s[2] == 3 && s[3] == 0) transpose_kjli(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 2 && s[1] == 3 && s[2] == 0 && s[3] == 1) transpose_klij(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 2 && s[1] == 3 && s[2] == 1 && s[3] == 0) transpose_klji(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 3 && s[1] == 0 && s[2] == 1 && s[3] == 2) transpose_lijk(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 3 && s[1] == 0 && s[2] == 2 && s[3] == 1) transpose_likj(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 3 && s[1] == 1 && s[2] == 0 && s[3] == 2) transpose_ljik(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 3 && s[1] == 1 && s[2] == 2 && s[3] == 0) transpose_ljki(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 3 && s[1] == 2 && s[2] == 0 && s[3] == 1) transpose_lkij(buf_x, buf_y, di, dj, dk, dl);
    else if (s[0] == 3 && s[1] == 2 && s[2] == 1 && s[3] == 0) transpose_lkji(buf_x, buf_y, di, dj, dk, dl);
    else {
      cout << "transpose position error\n";
      exit(0);
    }
  }

//  delete buf_x;

#if TIMER
  MyTimer::stop(TSP);
#endif
}

}
