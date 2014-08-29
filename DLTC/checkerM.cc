#include "checker.h"
namespace DLTC{
extern int me, nprocs;
extern vector<Multiplication> m;

void checkM(int id) {

  Tensor tA = m[id].tA();
  Tensor tB = m[id].tB();
  Tensor tC = m[id].tC();
  uint64_t A_size = tA.tensorSize();
  uint64_t B_size = tB.tensorSize();
  uint64_t C_size = tC.tensorSize();
  uint64_t a_size = tA.tileSize();
  uint64_t b_size = tB.tileSize();
  uint64_t c_size = tC.tileSize();

#if VERIFY
  double *A, *B, *sC, *pC;
  if (!me) {
    A  = new double[A_size]();
    B  = new double[B_size]();
    pC = new double[C_size]();
    sC = new double[C_size]();
    for (int i=0; i<A_size; i++) A[i]=2*drand48()-1;
    for (int i=0; i<B_size; i++) B[i]=2*drand48()-1;
    tA.putData(A,A_size);
    tB.putData(B,B_size);

    int lo[] = {0, 0, 0, 0, 0, 0};
    int hi[] = {V, V, O, O, O, O};
    IndexName name[] = {P1, P2, H1, H2, H3, H4};
    vector<int> lb(lo, lo + sizeof(lo) / sizeof(lo[0]));
    vector<int> ub(hi, hi + sizeof(hi) / sizeof(hi[0]));
    vector<IndexName> ids(name, name + sizeof(name) / sizeof(name[0]));
    IndexLoops lps = IndexLoops(ids, lb, ub);

    vector<int> idvmx(NIDX, 0);
    while (lps.hasNext(idvmx)) {
      int a = idvmx[P1];
      int b = idvmx[P2];
      int i = idvmx[H1];
      int j = idvmx[H2];
      int k = idvmx[H3];
      int l = idvmx[H4];
      sC[a*V*O*O+b*O*O+i*O+j] += A[a*O*O*O+k*O*O+i*O+l]*B[b*O*O*O+l*O*O+j*O+k]; // abij += akil * bljk
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
  props.functions(tf, &frt).taskSize(sizeof(TaskDesc))
    .maxTasks(100000).localData(&tl, sizeof(TaskLocal));
  UniformTaskCollectionSplit* tp = new UniformTaskCollectionSplit(props, 0);
#else
  TaskPool tp = gtc_create(AUTO_BODY_SIZE, NUM_TASKS, NULL, MPI_COMM_WORLD);
#endif
  exec(MUL, 0, tp);

#if CTX
  if (!me) cout << "run ctx for checking multiplication!\n";

#else
  if (!me) cout << "run scioto for checking multiplication!\n";

  Barrier();

#if TASCEL
  tp->process();
#else
  gtc_process(tp);
  gtc_print_stats(tp);
#endif

  Barrier();
#endif

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

#if VERIFY
  if (!me) {
    print(pC,"pC = \n", C_size, 30);
    tC.getData(pC,C_size);
    print(pC,"pC = \n", C_size, 30);
    if (check_diff(C_size, sC, pC)) printf("correct!\n");
    else printf("wrong!\n");
    delete A; delete B; delete sC; delete pC;
  }
#endif
}

}
