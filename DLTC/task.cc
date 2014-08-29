#include "task.h"


extern vector<DLTC::Multiplication> m;
extern vector<DLTC::Summation> s;

extern vector<double> Atget;
extern vector<double> Btget;
namespace DLTC{
#if TASCEL
void createTask(TaskPool* tp,
    ExprType et, int id,
    const vector<int>& itlb, const vector<int>& itub) {
  TaskDesc t;
  t.et = et;
  t.id = id;
  for (int i=0; i<NIDX; i++) {
    t.itlb[i] = itlb[i];
    t.itub[i] = itub[i];
  }
  tp->addTask(&t, sizeof(t));
}
#else
void createTask(TaskPool tp,
    ExprType et, int id,
    const vector<int>& itlb, const vector<int>& itub) {
  task_t *task = gtc_task_create(TaskClass);
  TaskDesc *t = (TaskDesc*)gtc_task_body(task);
  t->et = et;
  t->id = id;
  for (int i=0; i<NIDX; i++) {
    t->itlb[i] = itlb[i];
    t->itub[i] = itub[i];
  }
  gtc_add(tp, task, 0);
  gtc_task_destroy(task);
}
#endif

IndexLoops createIndexLoops(const vector<IndexName>& ids, int lmap[], int hmap[]) {
  int n = ids.size();
  vector<int> lo(n, 0);
  vector<int> hi(n, 0);
  for (int i = 0; i < n; i++) {
    lo[i] = lmap[ids[i]];
    hi[i] = hmap[ids[i]];
  }
  return IndexLoops(ids, lo, hi);
}

void task_sum(int id, int lmap[], int hmap[]) {

  tasknum += 1;

  Tensor tC = s[id].tC();
  Tensor tA = s[id].tA();
  uint64_t c_size = tC.tileSize();
  uint64_t a_size = tA.tileSize();

  IndexLoops extlps = createIndexLoops(tC.ids(), lmap, hmap);
  vector<int> idvm(NIDX, 0);
  while(extlps.hasNext(idvm)) {
#if 0
    auto_ptr<double> buf_a (new double[a_size]);
    auto_ptr<double> buf_at(new double[a_size]);
    tA.getTile2(buf_a.get(), a_size, idvm);
    transpose(buf_a.get(), tA.ids(), buf_at.get(), s[id].ext_ids());
    tC.accTile2(buf_at.get(), a_size, idvm, s[id].coef());
#else
    double *buf_a  = new double[a_size];
    double *buf_at = new double[a_size];
    bool inCache = false;
    tA.getTile2(buf_a, a_size, idvm, inCache);
    transpose(buf_a, tA.ids(), buf_at, s[id].ext_ids());
    tC.accTile2(buf_at, a_size, idvm, s[id].coef());
    if (!inCache) delete buf_a;
    delete buf_at;
#endif
  }

}

void task_mul(int id, int lmap[], int hmap[]) {

  tasknum += 1;

  vector<IndexName> ext_ids = m[id].ext_ids();
  vector<IndexName> sum_ids = m[id].k_ids();

  Tensor tA = m[id].tA();
  Tensor tB = m[id].tB();
  Tensor tC = m[id].tC();
  uint64_t a_size = tA.tileSize();
  uint64_t b_size = tB.tileSize();
  uint64_t c_size = tC.tileSize();

  IndexLoops extlps = createIndexLoops(tC.ids(), lmap, hmap);
  vector<int> idvm(NIDX, 0);
  while(extlps.hasNext(idvm)) {
    IndexLoops sumlps = createIndexLoops(m[id].k_ids(), lmap, hmap);
    double *buf_c  = new double[c_size];
    double *buf_ct = new double[c_size]();

    while(sumlps.hasNext(idvm)) {
      double *buf_a  = new double[a_size];
      double *buf_at = new double[a_size];
      double *buf_b  = new double[b_size];
      double *buf_bt = new double[b_size];
      bool tAinCache = false, tBinCache = false;

      Atget[id] -= MPI_Wtime();
      tA.getTile2(buf_a, a_size, idvm, tAinCache);
      Atget[id] += MPI_Wtime();

      transpose(buf_a, tA.ids(), buf_at, m[id].at_ids());

      Btget[id] -= MPI_Wtime();
      tB.getTile2(buf_b, b_size, idvm, tBinCache);
      Btget[id] += MPI_Wtime();

      transpose(buf_b, tB.ids(), buf_bt, m[id].bt_ids());

      compute(buf_ct, buf_at, buf_bt, m[id]);

      if (!tAinCache) delete buf_a;
      delete buf_at;
      if (!tBinCache) delete buf_b;
      delete buf_bt;
    } // sumlps

    transpose(buf_ct, m[id].ct_ids(), buf_c, tC.ids());
    tC.accTile2(buf_c, c_size, idvm, m[id].coef());
    delete buf_c;
    delete buf_ct;
  }

}

#if TASCEL
void TaskFunc(TaskPool *tp,
    void *_bigd, int bigd_len,
    void *pldata, int pldata_len, vector<void *> data_bufs,
    int thd) {

  TaskDesc *t = (TaskDesc*)_bigd;
  ExprType et = t->et;

  if (et == SUM) {
    task_sum(t->id, t->itlb, t->itub);
  }
  else if (et == MUL) {
    task_mul(t->id, t->itlb, t->itub);
  }
}
#else
void TaskFunc(TaskPool tp, task_t *task) {

  TaskDesc *t = (TaskDesc*)gtc_task_body(task);
  ExprType et = t->et;

  if (et == SUM) {
    task_sum(t->id, t->itlb, t->itub);
  }
  else if (et == MUL) {
    task_mul(t->id, t->itlb, t->itub);
  }
}
#endif

}
