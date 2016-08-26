#include "checker.h"

namespace DLTC{
extern int me, nprocs;
extern vector<Summation> s;

void checkS(int id) {

  Tensor tC = s[id].tC();
  Tensor tA = s[id].tA();
  uint64_t C_size = tC.tensorSize();
  uint64_t A_size = tA.tensorSize();
  uint64_t c_size = tC.tileSize();
  uint64_t a_size = tA.tileSize();

#if CHECK
  double *A, *sC, *pC;
  if (!me) {
    A  = new double[A_size]();
    pC = new double[C_size](); //zeros(pC, double, C_size);
    sC = new double[C_size](); //zeros(sC, double, C_size);
    for (int i = 0; i < A_size; i++) A[i] = 2 * drand48() - 1;
    tA.putData(A, A_size);

    int d = 4;
    int lo[] = {0, 0, 0, 0};
    int hi[] = {V, V, O, O};
    IndexName name[] = {P1, P2, H1, H2};
    vector<int> lb(lo, lo + sizeof(lo) / sizeof(lo[0]));
    vector<int> ub(hi, hi + sizeof(hi) / sizeof(hi[0]));
    vector<IndexName> ids(name, name + sizeof(name) / sizeof(name[0]));
    IndexLoops lps = IndexLoops(ids, lb, ub);

    vector<int> idvmx(NIDX, 0);
    while(lps.hasNext(idvmx)) {
      int a = idvmx[P1];
      int b = idvmx[P2];
      int i = idvmx[H1];
      int j = idvmx[H2];
      sC[a*V*O*O+b*O*O+i*O+j] += A[b*V*O*O+a*O*O+j*O+i]; // abij += 0.5*(abij + baji)
    }
    print(sC,"sC = \n", C_size, 30);
  }
#endif

  Barrier();

#if TIMER
  MyTimer::start(TOT);
#endif

#if TASCEL
  TslFuncRegTbl frt;
  TslFunc tf = frt.add(TaskFunc);
  TaskLocal tl;
  TaskCollProps props;
  props.functions(tf, &frt).taskSize(sizeof(TaskDesc)).maxTasks(100000).localData(&tl, sizeof(TaskLocal));
  UniformTaskCollectionSplit *tp = new UniformTaskCollectionSplit(props, 0);
#else
  TaskPool tp = gtc_create(AUTO_BODY_SIZE, NUM_TASKS, NULL, MPI_COMM_WORLD);
#endif
  exec(SUM, 0, tp);

  Barrier();

#if TASCEL
  tp->process();
#else
  gtc_process(tp);
#endif

  Barrier();

#if TIMER
  MyTimer::stop(TOT);
  double t1, t2, t3, t4, t5;
  t1 = MyTimer::show(TOT);
  t2 = MyTimer::show(GET);
  t3 = MyTimer::show(TSP);
  t4 = MyTimer::show(DGM);
  t5 = MyTimer::show(ACC);
  if (!me) {
    cout << " tot time = " << t1 << endl << flush;
    cout << " get time = " << t2 << endl << flush;
    cout << " tsp time = " << t3 << endl << flush;
    cout << " dgm time = " << t4 << endl << flush;
    cout << " acc time = " << t5 << endl << flush;
  }
#endif

#if CHECK
  if (!me) {
    tC.getData(pC, C_size);
    print(pC,"pC = \n", C_size, 30);
    if (check_diff(C_size, sC, pC)) printf("correct!\n");
    else printf("wrong!\n");
    delete A;
    delete sC;
    delete pC;
  }
#endif
}

}
