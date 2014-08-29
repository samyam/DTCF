#include "cc.h"
using namespace DLTC;
void ccd_t2() {

#if CCD
  Barrier();

  enum TensorName {
    R_VVOO,
    F_OO, F_VV,
    T_VVOO,
    V_OOOO, V_VVOO, V_OOVV, V_OVOV, V_VVVV,
    I_OO, I_VV, I_OOOO, I_OVOV
  };

  numTensors = 13;
  numMuls = 9;
  numSums = 5;
  T.resize(numTensors);
  m.resize(numMuls);
  s.resize(numSums);

  nget.resize(numTensors, 0);
  ngaget.resize(numTensors, 0);
  tget.resize(numTensors, 0.0);

  Atget.resize(numMuls, 0.0);
  Btget.resize(numMuls, 0.0);


  T[F_VV] = Tensor2(PAR, PAR, F_VV);
  T[I_OO] = Tensor2(HOL, HOL, I_OO);
  T[T_VVOO] = Tensor4(PAR, PAR, HOL, HOL, T_VVOO);
  T[I_OVOV] = Tensor4(HOL, PAR, HOL, PAR, I_OVOV);
  T[V_OOOO] = Tensor4(HOL, HOL, HOL, HOL, V_OOOO);
  T[V_VVVV] = Tensor4(PAR, PAR, PAR, PAR, V_VVVV);
  T[V_OVOV] = Tensor4(HOL, PAR, HOL, PAR, V_OVOV);
  T[F_OO] = Tensor2(HOL, HOL, F_OO);
  T[V_OOVV] = Tensor4(HOL, HOL, PAR, PAR, V_OOVV);
  T[V_VVOO] = Tensor4(PAR, PAR, HOL, HOL, V_VVOO);
  T[I_VV] = Tensor2(PAR, PAR, I_VV);
  T[R_VVOO] = Tensor4(PAR, PAR, HOL, HOL, R_VVOO);
  T[I_OOOO] = Tensor4(HOL, HOL, HOL, HOL, I_OOOO);

  setIndexName4(T[R_VVOO], p3, p4, h1, h2);
  setIndexName4(T[V_VVOO], p3, p4, h1, h2);
  s[0] = Summation(T[R_VVOO], T[V_VVOO], 1.0);
  setIndexName2(T[I_OO], h5, h1);
  setIndexName2(T[F_OO], h5, h1);
  s[1] = Summation(T[I_OO], T[F_OO], 1.0);
  setIndexName4(T[T_VVOO], p6, p7, h1, h8);
  setIndexName4(T[V_OOVV], h5, h8, p6, p7);
  setIndexName2(T[I_OO], h5, h1);
  m[0] = Multiplication(T[I_OO], T[T_VVOO], T[V_OOVV], 1.0);
  setIndexName4(T[T_VVOO], p3, p4, h1, h5);
  setIndexName2(T[I_OO], h5, h2);
  setIndexName4(T[R_VVOO], p3, p4, h1, h2);
  m[1] = Multiplication(T[R_VVOO], T[T_VVOO], T[I_OO], 1.0);
  setIndexName2(T[I_VV], p3, p5);
  setIndexName2(T[F_VV], p3, p5);
  s[2] = Summation(T[I_VV], T[F_VV], 1.0);
  setIndexName4(T[T_VVOO], p3, p6, h7, h8);
  setIndexName4(T[V_OOVV], h7, h8, p5, p6);
  setIndexName2(T[I_VV], p3, p5);
  m[2] = Multiplication(T[I_VV], T[T_VVOO], T[V_OOVV], 1.0);
  setIndexName4(T[T_VVOO], p3, p5, h1, h2);
  setIndexName2(T[I_VV], p4, p5);
  setIndexName4(T[R_VVOO], p3, p4, h1, h2);
  m[3] = Multiplication(T[R_VVOO], T[T_VVOO], T[I_VV], 1.0);
  setIndexName4(T[I_OOOO], h7, h9, h1, h2);
  setIndexName4(T[V_OOOO], h7, h9, h1, h2);
  s[3] = Summation(T[I_OOOO], T[V_OOOO], 1.0);
  setIndexName4(T[T_VVOO], p5, p6, h1, h2);
  setIndexName4(T[V_OOVV], h7, h9, p5, p6);
  setIndexName4(T[I_OOOO], h7, h9, h1, h2);
  m[4] = Multiplication(T[I_OOOO], T[T_VVOO], T[V_OOVV], 1.0);
  setIndexName4(T[T_VVOO], p3, p4, h7, h9);
  setIndexName4(T[I_OOOO], h7, h9, h1, h2);
  setIndexName4(T[R_VVOO], p3, p4, h1, h2);
  m[5] = Multiplication(T[R_VVOO], T[T_VVOO], T[I_OOOO], 1.0);
  setIndexName4(T[I_OVOV], h6, p3, h1, p5);
  setIndexName4(T[V_OVOV], h6, p3, h1, p5);
  s[4] = Summation(T[I_OVOV], T[V_OVOV], 1.0);
  setIndexName4(T[T_VVOO], p3, p7, h1, h8);
  setIndexName4(T[V_OOVV], h6, h8, p5, p7);
  setIndexName4(T[I_OVOV], h6, p3, h1, p5);
  m[6] = Multiplication(T[I_OVOV], T[T_VVOO], T[V_OOVV], 1.0);
  setIndexName4(T[T_VVOO], p3, p5, h1, h6);
  setIndexName4(T[I_OVOV], h6, p4, h2, p5);
  setIndexName4(T[R_VVOO], p3, p4, h1, h2);
  m[7] = Multiplication(T[R_VVOO], T[T_VVOO], T[I_OVOV], 1.0);
  setIndexName4(T[T_VVOO], p5, p6, h1, h2);
  setIndexName4(T[V_VVVV], p3, p4, p5, p6);
  setIndexName4(T[R_VVOO], p3, p4, h1, h2);
  m[8] = Multiplication(T[R_VVOO], T[T_VVOO], T[V_VVVV], 1.0);

  Barrier();
  if (!me) { cout << "layer 1\n"; cout.flush(); }
  TaskPool tp1 = gtc_create(AUTO_BODY_SIZE, NUM_TASKS, NULL, MPI_COMM_WORLD);
  exec(SUM,0,tp1); // partition expressions into tasks
  exec(SUM,1,tp1);
  exec(MUL,0,tp1);
  exec(SUM,2,tp1);
  exec(MUL,2,tp1);
  exec(SUM,3,tp1);
  exec(MUL,4,tp1);
  exec(SUM,4,tp1);
  exec(MUL,6,tp1);
  exec(MUL,8,tp1);
#if TIMER
  MyTimer::start(TOT);
#endif
  gtc_process(tp1);
#if TIMER
  MyTimer::stop(TOT);
#endif

  Barrier();
  if (!me) { cout << "layer 2\n"; cout.flush(); }
  TaskPool tp2 = gtc_create(AUTO_BODY_SIZE, NUM_TASKS, NULL, MPI_COMM_WORLD);
  exec(MUL,1,tp2);
  exec(MUL,3,tp2);
  exec(MUL,5,tp2);
  exec(MUL,7,tp2);
#if TIMER
  MyTimer::start(TOT);
#endif
  gtc_process(tp2);
#if TIMER
  MyTimer::stop(TOT);
#endif

  Barrier();

#endif
}

