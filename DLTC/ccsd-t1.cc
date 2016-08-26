#include "cc.h"
using namespace DLTC;
void ccsd_t1() {

#if CCSD

  enum TensorName{
    F_OO,F_OV,F_VO,F_VV,I_1,I_2,I_3,I_4,I_5,R,T_VO,T_VVOO,V_OOOV,V_OOVV,V_OVOV,V_OVVV};

  numTensors = 16;
  numMuls = 13;
  numSums = 6;

  T.resize(numTensors);
  m.resize(numMuls);
  s.resize(numSums);
  nget.resize(numTensors, 0);
  ngaget.resize(numTensors, 0);
  tget.resize(numTensors, 0.0);

  Atget.resize(numMuls, 0.0);
  Btget.resize(numMuls, 0.0);

  T[F_VV] = Tensor2(PAR, PAR, F_VV);
  T[T_VVOO] = Tensor4(PAR, PAR, HOL, HOL, T_VVOO);
  T[F_OV] = Tensor2(HOL, PAR, F_OV);
  T[V_OVOV] = Tensor4(HOL, PAR, HOL, PAR, V_OVOV);
  T[F_OO] = Tensor2(HOL, HOL, F_OO);
  T[R] = Tensor2(PAR, HOL, R);
  T[V_OVVV] = Tensor4(HOL, PAR, PAR, PAR, V_OVVV);
  T[V_OOOV] = Tensor4(HOL, HOL, HOL, PAR, V_OOOV);
  T[T_VO] = Tensor2(PAR, HOL, T_VO);
  T[F_VO] = Tensor2(PAR, HOL, F_VO);
  T[V_OOVV] = Tensor4(HOL, HOL, PAR, PAR, V_OOVV);
  T[I_5] = Tensor4(HOL, HOL, HOL, PAR, I_5);
  T[I_4] = Tensor2(HOL, PAR, I_4);
  T[I_3] = Tensor2(PAR, PAR, I_3);
  T[I_2] = Tensor2(HOL, PAR, I_2);
  T[I_1] = Tensor2(HOL, HOL, I_1);
  setIndexName2(T[R], p2, h1);
  setIndexName2(T[F_VO], p2, h1);
  s[0] = Summation(T[R], T[F_VO], 1.0);
  setIndexName2(T[T_VO], p3, h4);
  setIndexName4(T[V_OVOV], h4, p2, h1, p3);
  setIndexName2(T[R], p2, h1);
  m[0] = Multiplication(T[R], T[T_VO], T[V_OVOV], -1.0);
  setIndexName4(T[T_VVOO], p3, p4, h1, h5);
  setIndexName4(T[V_OVVV], h5, p2, p3, p4);
  setIndexName2(T[R], p2, h1);
  m[1] = Multiplication(T[R], T[T_VVOO], T[V_OVVV], -0.5);
  setIndexName2(T[I_1], h7, h1);
  setIndexName2(T[F_OO], h7, h1);
  s[1] = Summation(T[I_1], T[F_OO], 1.0);
  setIndexName2(T[T_VO], p4, h5);
  setIndexName4(T[V_OOOV], h5, h7, h1, p4);
  setIndexName2(T[I_1], h7, h1);
  m[2] = Multiplication(T[I_1], T[T_VO], T[V_OOOV], -1.0);
  setIndexName4(T[T_VVOO], p3, p4, h1, h5);
  setIndexName4(T[V_OOVV], h5, h7, p3, p4);
  setIndexName2(T[I_1], h7, h1);
  m[3] = Multiplication(T[I_1], T[T_VVOO], T[V_OOVV], -0.5);
  setIndexName2(T[I_2], h7, p3);
  setIndexName2(T[F_OV], h7, p3);
  s[2] = Summation(T[I_2], T[F_OV], 1.0);
  setIndexName2(T[T_VO], p5, h6);
  setIndexName4(T[V_OOVV], h6, h7, p3, p5);
  setIndexName2(T[I_2], h7, p3);
  m[4] = Multiplication(T[I_2], T[T_VO], T[V_OOVV], -1.0);
  setIndexName2(T[T_VO], p3, h1);
  setIndexName2(T[I_2], h7, p3);
  setIndexName2(T[I_1], h7, h1);
  m[5] = Multiplication(T[I_1], T[T_VO], T[I_2], 1.0);
  setIndexName2(T[T_VO], p2, h7);
  setIndexName2(T[I_1], h7, h1);
  setIndexName2(T[R], p2, h1);
  m[6] = Multiplication(T[R], T[T_VO], T[I_1], -1.0);
  setIndexName2(T[I_3], p2, p3);
  setIndexName2(T[F_VV], p2, p3);
  s[3] = Summation(T[I_3], T[F_VV], 1.0);
  setIndexName2(T[T_VO], p4, h5);
  setIndexName4(T[V_OVVV], h5, p2, p3, p4);
  setIndexName2(T[I_3], p2, p3);
  m[7] = Multiplication(T[I_3], T[T_VO], T[V_OVVV], -1.0);
  setIndexName2(T[T_VO], p3, h1);
  setIndexName2(T[I_3], p2, p3);
  setIndexName2(T[R], p2, h1);
  m[8] = Multiplication(T[R], T[T_VO], T[I_3], 1.0);
  setIndexName2(T[I_4], h8, p7);
  setIndexName2(T[F_OV], h8, p7);
  s[4] = Summation(T[I_4], T[F_OV], 1.0);
  setIndexName2(T[T_VO], p5, h6);
  setIndexName4(T[V_OOVV], h6, h8, p5, p7);
  setIndexName2(T[I_4], h8, p7);
  m[9] = Multiplication(T[I_4], T[T_VO], T[V_OOVV], 1.0);
  setIndexName4(T[T_VVOO], p2, p7, h1, h8);
  setIndexName2(T[I_4], h8, p7);
  setIndexName2(T[R], p2, h1);
  m[10] = Multiplication(T[R], T[T_VVOO], T[I_4], 1.0);
  setIndexName4(T[I_5], h4, h5, h1, p3);
  setIndexName4(T[V_OOOV], h4, h5, h1, p3);
  s[5] = Summation(T[I_5], T[V_OOOV], 1.0);
  setIndexName2(T[T_VO], p6, h1);
  setIndexName4(T[V_OOVV], h4, h5, p3, p6);
  setIndexName4(T[I_5], h4, h5, h1, p3);
  m[11] = Multiplication(T[I_5], T[T_VO], T[V_OOVV], -1.0);
  setIndexName4(T[T_VVOO], p2, p3, h4, h5);
  setIndexName4(T[I_5], h4, h5, h1, p3);
  setIndexName2(T[R], p2, h1);
  m[12] = Multiplication(T[R], T[T_VVOO], T[I_5], -0.5);

  Barrier();
  if (!me) { cout << "layer 1\n"; cout.flush(); }
  TaskPool tp1 = gtc_create(AUTO_BODY_SIZE, NUM_TASKS, NULL, MPI_COMM_WORLD);
  exec(SUM,0,tp1);
  exec(MUL,0,tp1);
  exec(MUL,1,tp1);
  exec(SUM,1,tp1);
  exec(MUL,2,tp1);
  exec(MUL,3,tp1);
  exec(SUM,2,tp1);
  exec(MUL,4,tp1);
  exec(SUM,3,tp1);
  exec(MUL,7,tp1);
  exec(SUM,4,tp1);
  exec(MUL,9,tp1);
  exec(SUM,5,tp1);
  exec(MUL,11,tp1);
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
  exec(MUL,5,tp2);
  exec(MUL,8,tp2);
  exec(MUL,10,tp2);
  exec(MUL,12,tp2);
#if TIMER
  MyTimer::start(TOT);
#endif
  gtc_process(tp2);
#if TIMER
  MyTimer::stop(TOT);
#endif


  Barrier();
  if (!me) { cout << "layer 3\n"; cout.flush(); }
  TaskPool tp3 = gtc_create(AUTO_BODY_SIZE, NUM_TASKS, NULL, MPI_COMM_WORLD);
  exec(MUL,6,tp3);
#if TIMER
  MyTimer::start(TOT);
#endif
  gtc_process(tp3);
#if TIMER
  MyTimer::stop(TOT);
#endif

#endif
}
