#include "define.h"
#include "nxtask.h"
#include "dltc_tensor.h"
#include "loops.h"
#include "expr.h"
#include "transp.h"
#include "compute.h"
#include "task.h"
#include "profiler.h"
#include "cache.h"
#define TIMER 1


#if CHECK
#include "checker.h"
#endif

#include "exec.h"
#include "command-line-parser.h"
#include "cc.h"

using namespace DLTC;

#if TIMER
vector<double> MyTimer::name(nTimers, 0.0);
vector<TimeStat> tstat(NMULT);
int tasknum = 0;
#endif

vector<Multiplication> m;
vector<Summation> s;
vector<Tensor> T;
int numTensors;
int numMuls;
int numSums;

int nget2, nhit2;
int nget4, nhit4;
vector<int> nget;
vector<int> ngaget;
vector<double> tget;
double traffic;

vector<double> Atget;
vector<double> Btget;


//int O, V, spO, spV, wbO, wbV;
int me, nprocs, nthrds;
LRUCache Cache;

#if VERBOSE
ofstream ofs, ofm;
#endif

#if TASCEL
#else
task_class_t TaskClass;
#endif

#if MKL
#include "mkl.h"
#endif

#if OMP
#include "omp.h"
#endif

void printSetup() {
  if (!me) {
    printf("O = %d  V = %d  wbO = %d  wbV = %d\n", O, V, wbO, wbV);
    printf("nprocs = %d  nthrds = %d\n", nprocs, nthrds);
  }
}

void printTimer() {
  //  printf("T: rank:%d tot:%.2f com:%.2f tsp:%.2f dgm:%.2f\n", me, t1, t2 + t5, t3, t4);
  double t1, t2, t3, t4, t5;
  t1 = MyTimer::show(TOT);
  t2 = MyTimer::show(GET);
  t3 = MyTimer::show(TSP);
  t4 = MyTimer::show(DGM);
  t5 = MyTimer::show(ACC);
//  printf("TIME %d  TOT:%.2f COMM:%.2f TRAN:%.2f DGEM:%.2f\n", me, t1, t2 + t5, t3, t4);

  double g1, g2, g3, g4, g5;

  MPI_Reduce(&t1, &g1, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&t2, &g2, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&t3, &g3, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&t4, &g4, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&t5, &g5, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  printSetup();
  if (!me) {
    printf("TIMEAVG TOT:%.2f COMM:%.2f TRAN:%.2f DGEM:%.2f\n", g1/nprocs, (g2 + g5)/nprocs, g3/nprocs, g4/nprocs);
  }

#if 0
  double n4time = 0;
  double n5time = 0;
  double n6time = 0;
  double nxtime = 0;
  //sort(tstat.begin(), tstat.end(), compStat);
  for (int i = 0; i < NMULT; i++) {
    if (m[i].order() == 6) n6time += tstat[i].tot;
    else if (m[i].order() == 5) n5time += tstat[i].tot;
    else if (m[i].order() == 4) n4time += tstat[i].tot;
    else nxtime += tstat[i].tot;
  }
  double tottime = n4time + n5time + n6time + nxtime;
//  printf("R1: rank:%d total:%.2f n6:%.2f n5:%.2f n4:%.2f other:%.2f\n", me, tottime, n6time, n5time, n4time, nxtime);
//  printf("R2: rank:%d total:%.2f n6:%.2f n5:%.2f n4:%.2f other:%.2f\n", me, tottime/tottime, n6time/tottime, n5time/tottime, n4time/tottime, nxtime/tottime);
#endif

  vector<int> gnget(numTensors);
  vector<int> gngaget(numTensors);
  vector<double> gtget(numTensors);
  double gtraffic;
  MPI_Reduce(&traffic, &gtraffic, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  MPI_Reduce(&nget.front(), &gnget.front(), numTensors, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&ngaget.front(), &gngaget.front(), numTensors, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&tget.front(), &gtget.front(), numTensors, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  int fmax = 10; // record 0X to 20X
  int nKeys2D = Profiler::nKeys(2);
  int nKeys4D = Profiler::nKeys(4);
  map<int, int> map2D = Profiler::getRawReuseInfo(2);
  map<int, int> map4D = Profiler::getRawReuseInfo(2);
  vector<double> tab2D = Profiler::getReuseInfo(2, fmax);
  vector<double> tab4D = Profiler::getReuseInfo(4, fmax);

  MPI_Barrier(MPI_COMM_WORLD);

  int gKeys2D, gKeys4D;
  MPI_Reduce(&nKeys2D, &gKeys2D, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&nKeys4D, &gKeys4D, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  int vsize = tab2D.size();
  vector<double> gtab2D(vsize, 0);
  vector<double> gtab4D(vsize, 0);

  MPI_Reduce(&tab2D.front(), &gtab2D.front(), vsize, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&tab4D.front(), &gtab4D.front(), vsize, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  int gnget2, gnhit2;
  int gnget4, gnhit4;

  MPI_Reduce(&nget2, &gnget2, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&nget4, &gnget4, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&nhit2, &gnhit2, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&nhit4, &gnhit4, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

//  cout << "RANK " << me << "nget2:" << nget2 << " " << "nhit2:" << nhit2 << " " << nhit2/double(nget2) * 100 << "%" << endl;
//  cout << "RANK " << me << "nget4:" << nget4 << " " << "nhit4:" << nhit4 << " " << nhit4/double(nget4) * 100 << "%" << endl;

  vector<double> gAtget(numMuls);
  vector<double> gBtget(numMuls);
  MPI_Reduce(&Atget.front(), &gAtget.front(), numMuls, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&Btget.front(), &gBtget.front(), numMuls, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  double cuse = Cache.usedInGB();
  double guse = 0;
  MPI_Reduce(&cuse, &guse, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  MPI_Barrier(MPI_COMM_WORLD);
  //printf("nKeys of rank %d: 2D = %d, 4D = %d \n", me, nKeys2D, nKeys4D);
  if (!me) {
//#if CACHING
    printf("Cache size = %.2f GB (%ld bytes), used = %.2f GB (%ld bytes), average used = %.2f GB\n",
      Cache.sizeInGB(), Cache.size(), Cache.usedInGB(), Cache.used(), guse);
//#endif
    cout << "AVG nget2:" << gnget2 << " " << "nhit2:" << gnhit2 << " " << gnhit2/double(gnget2) * 100 << "%" << endl;
    cout << "AVG nget4:" << gnget4 << " " << "nhit4:" << gnhit4 << " " << gnhit4/double(gnget4) * 100 << "%" << endl;
    cout << "AVG traffic:" << gtraffic/1024/1024/1024 << "GB " << gtraffic/1024/1024/1024/nprocs << endl;

    for (int i = 0; i < numTensors; ++i) {
      cout << "Tensor" << i << ":\n";
//      printf("  #    get: %d\n", gnget[i]);
      printf("  # ga get: %d\n", gngaget[i]);
      printf("   traffic: %.2f  %.2f  %.2f\n", gtget[i], gtget[i] / nprocs, gtget[i]/(g2 + g5)*100);
    }

    for (int i = 0; i < numMuls; ++i) {
      cout << "Multiplication" << i << ":\n";
//      printf("  #    get: %d\n", gnget[i]);
      printf(" Tensor %d: %.2f\n", m[i].tA().id(), gAtget[i]);
      printf(" Tensor %d: %.2f\n", m[i].tB().id(), gBtget[i]);
    }

#if 0
    printf("AVG miss 2D-tile = %.2f (#%.2f / %d nprocs)\n", double(gKeys2D) / nprocs, double(gKeys2D), nprocs);
    printf("AVG miss 4D-tile = %.2f (#%.2f / %d nprocs)\n", double(gKeys4D) / nprocs, double(gKeys4D), nprocs);
    for (int i = 1; i < gtab2D.size(); i++) {
      if (i != gtab2D.size() - 1)
        printf("avg # of 2D-tile with %dX reuse = %.2f (%.2f%)\n", i, gtab2D[i] / nprocs, gtab2D[i] / gKeys2D * 100);
      else
        printf("avg # of 2D-tile with %dX+ reuse = %.2f (%.2f%)\n", i, gtab2D[i] / nprocs, gtab2D[i] / gKeys2D * 100);
    }
    for (int i = 1; i < gtab4D.size(); i++) {
      if (i != gtab4D.size() - 1)
        printf("avg # of 4D-tile with %dX reuse = %.2f (%.2f%)\n", i, gtab4D[i] / nprocs, gtab4D[i] / gKeys4D * 100);
      else
        printf("avg # of 4D-tile with %dX+ reuse = %.2f (%.2f%)\n", i, gtab4D[i] / nprocs, gtab4D[i] / gKeys4D * 100);
    }
#endif

  }
}
/*
int main(int argc, char **argv) {

  nget2 = 0;
  nhit2 = 0;
  nget4 = 0;
  nhit4 = 0;
  traffic = 0;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &me);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

#if TASCEL
  if (!me) cout << "TASCEL" << endl;
  Tascel_Init();
#else // SCIOTO
  if (!me) cout << "SCIOTO" << endl;
  GA_Initialize();
  TaskClass = gtc_task_class_register(sizeof(TaskDesc), TaskFunc); // register function
#endif
  Profiler::init();
//#if CACHING
  int cacheSize = read_int(argc, argv, "-cs", 2);
  Cache.init(cacheSize); // 2 GB
  if (!me) {
    printf("Cache init = %.2f GB (%ld bytes), used = %.2f GB (%ld bytes)\n",
        Cache.sizeInGB(), Cache.size(), Cache.usedInGB(), Cache.used());
  }
//#endif

  O   = read_int(argc, argv, "-O", 10);
  V   = read_int(argc, argv, "-V", 100);
  wbO = read_int(argc, argv, "-wbO", 5);
  wbV = read_int(argc, argv, "-wbV", 25);
  spO = O / wbO;
  spV = V / wbV;
  assert(O == spO * wbO && O > 0 && wbO > 0);
  assert(V == spV * wbV && V > 0 && wbV > 0);
  nthrds = read_int(argc, argv, "-t", 1);
  assert(nthrds > 0);

#if MKL
  mkl_set_num_threads(nthrds);
#endif
#if OMP
  omp_set_num_threads(nthrds);
#endif
//  printSetup();

#if VERBOSE
  if (!me) {
    ofs.open("ofs.txt");
    ofm.open("ofm.txt");
  }
  MPI_Barrier(MPI_COMM_WORLD);
#endif

#if CHECK

  O   = 4;
  V   = 4;
  wbO = 1;
  wbV = 1;
  spO = 4;
  spV = 4;

#if 1
  m.resize(1);
//  srand48(time(NULL));
  Tensor tC = Tensor4(PAR, PAR, HOL, HOL, 0); setIndexName4(tC, P1, P2, H1, H2);
  Tensor tA = Tensor4(PAR, HOL, HOL, HOL, 1); setIndexName4(tA, P1, H4, H1, H3);
  Tensor tB = Tensor4(PAR, HOL, HOL, HOL, 2); setIndexName4(tB, P2, H3, H2, H4);
  m[0] = Multiplication(tC, tA, tB, 1.0); // abij += akil * bljk -> iajb += iakl * kljb
  checkM(0);
#else
  s.resize(1);
  Tensor tX = Tensor4(PAR, PAR, HOL, HOL, 0); setIndexName4(tX, P1, P2, H1, H2);
  Tensor tY = Tensor4(PAR, PAR, HOL, HOL, 1); setIndexName4(tY, P2, P1, H2, H1);
  s[0] = Summation(tX, tY, 1.0);
  checkS(0);
#endif

#else

  int method = read_int(argc, argv, "-cc", 1);
  if (method == 1) {
    if (!me) cout << "CCD-T2 start...\n";
    ccd_t2();
    printTimer();
  }
  else if (method==2) {
    if (!me) cout << "CCSD-T1 start...\n";
    ccsd_t1();
    printTimer();
  }
  else if (method==3) {
    if (!me) cout << "CCSD-T2 start...\n";
    ccsd_t2();
    printTimer();
  }

#endif

  MPI_Barrier(MPI_COMM_WORLD);

#if VERBOSE
  if (!me) {
    ofs.close();
    ofm.close();
  }
#endif

  if (!me) cout << "finalize\n";

  Profiler::close();
//#if CACHING
  Cache.close();
//#endif
#if TASCEL
  Tascel_Finalize();
#else
  GA_Terminate();
#endif

  MPI_Finalize();
}*/

