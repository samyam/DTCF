#ifndef _TIMER_H_
#define _TIMER_H_

#include <vector>
#include "mpi.h"

using namespace std;

const int nTimers = 5;
enum TimerType { TOT, GET, TSP, DGM, ACC };

class MyTimer {
private:
  static vector<double> name;
public:
  static void start(TimerType x) {
    name[x] -= MPI_Wtime();
  }
  static void stop(TimerType x) {
    name[x] += MPI_Wtime();
  }
  static double show(TimerType x) {
    return name[x];
  }
};

class TimeStat {
  public:
    int id;
    double tot;
    double com;
    double dgm;
    double tsp;
    TimeStat() { id = -1; tot = 0; com = 0; dgm = 0; tsp = 0; };
    ~TimeStat() {};
};
inline bool compStat(TimeStat x, TimeStat y) { return x.com < y.com; }

extern vector<TimeStat> tstat;

#endif
