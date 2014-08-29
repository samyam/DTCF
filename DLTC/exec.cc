#include "exec.h"

#define EXPLEV 0
#define EXTLEV 1
#define SUMLEV 2


extern int tasknum;
extern ofstream ofs;
extern ofstream ofm;
extern vector<DLTC::Multiplication> m;
extern vector<DLTC::Summation> s;
namespace DLTC{
#if TASCEL
void execS(int id, TaskPool* tp) {
#else
void execS(int id, TaskPool tp) {
#endif
  if (id % nprocs != me) return;

  Tensor tC = s[id].tC();
  Tensor tA = s[id].tA();
  uint64_t C_size = tC.tensorSize();
  uint64_t A_size = tA.tensorSize();
  uint64_t c_size = tC.tileSize();
  uint64_t a_size = tA.tileSize();

#if VERBOSE
  if (!me) {
    ofs << id << " " << C_size << " " << A_size << " " << c_size << " " << a_size << "\n";
  }
#endif

  vector<int> itlb(NIDX, 0);
  vector<int> itub(NIDX, 0);
  getITUB(tC.ids(), itub);

  createTask(tp, SUM, id, itlb, itub);
}

#if TASCEL
void execM(int id, TaskPool* tp) {
#else
void execM(int id, TaskPool tp) {
#endif

  Tensor tA = m[id].tA();
  Tensor tB = m[id].tB();
  Tensor tC = m[id].tC();
  uint64_t A_size = tA.tensorSize();
  uint64_t B_size = tB.tensorSize();
  uint64_t C_size = tC.tensorSize();
  uint64_t a_size = tA.tileSize();
  uint64_t b_size = tB.tileSize();
  uint64_t c_size = tC.tileSize();

  int count = 0;
#if 1
  int PARLEV = EXPLEV;
  if (m[id].order()==6) PARLEV=SUMLEV;
  if (m[id].order()==5) PARLEV=EXTLEV;
  if (m[id].order()==4) PARLEV=EXPLEV;
#endif
//  int PARLEV = EXTLEV;
  if (PARLEV == EXPLEV) { // exp level parallelism
    if (id % nprocs != me) return;
    vector<int> itlb(NIDX, 0);
    vector<int> itub(NIDX, 0);
    getITUB(m[id].all_ids(), itub);
    createTask(tp, MUL, id, itlb, itub);
    tasknum++;
    count++;
  }
  else if (PARLEV == EXTLEV) { // ext inds level parallelism
    IndexLoops extlps = m[id].createExtlps();
    vector<IndexName> ext_ids = m[id].ext_ids();
    vector<IndexName> sum_ids = m[id].k_ids();
    vector<int> itlb(NIDX, 0);
    while(extlps.hasNext(itlb)) {
      if (count % nprocs == me) {
        vector<int> itub(NIDX, 0);
        for (int i = 0; i < ext_ids.size(); i++) {
          itub[ext_ids[i]] = itlb[ext_ids[i]] + 1;
        }
        getITUB(sum_ids, itub); // get upperbound for sum_ids
        createTask(tp, MUL, id, itlb, itub);
        tasknum++;
      }
      count++;
    } // extlps
  }
  else if (PARLEV == SUMLEV) { // sum inds level parallelism
    IndexLoops extlps = m[id].createExtlps();
    vector<IndexName> ext_ids = m[id].ext_ids();
    vector<IndexName> sum_ids = m[id].k_ids();
    vector<int> itlb(NIDX, 0);
    while(extlps.hasNext(itlb)) {
      IndexLoops sumlps = m[id].createSumlps();
      while(sumlps.hasNext(itlb)) {
        if (count % nprocs == me) {
          vector<int> itub(NIDX, 0);
          for (int i = 0; i < ext_ids.size(); i++) itub[ext_ids[i]] = itlb[ext_ids[i]] + 1;
          for (int i = 0; i < sum_ids.size(); i++) itub[sum_ids[i]] = itlb[sum_ids[i]] + 1;
          createTask(tp, MUL, id, itlb, itub);
          tasknum++;
        }
        count++;
      } // while sumlps
    } // while extlps
  }
}

#if TASCEL
void exec(ExprType et, int id, TaskPool* tp) {
#else
void exec(ExprType et, int id, TaskPool tp) {
#endif
  if (et == MUL) execM(id, tp);
  else if (et == SUM) execS(id, tp);
}
}

