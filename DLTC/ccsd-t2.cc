#include "cc.h"
using namespace DLTC;
void ccsd_t2() {

#if CCSD

  enum TensorName{
    R, F_OO,F_OV,F_VV, T_VO, T_VVOO,
    V_OOOO,V_OOOV,V_OOVV,V_OVOO,V_OVOV,V_OVVV,V_VVOO,V_VVOV,V_VVVV,
    I_1,I_2,I_3,I_4,I_5,I_6,I_7,I_8,I_9,I_10,I_11,I_12,I_13};

  numTensors = 28;
  numMuls = 30;
  numSums = 14;

  T.resize(numTensors);
  m.resize(numMuls);
  s.resize(numSums);
  nget.resize(numTensors, 0);
  ngaget.resize(numTensors, 0);
  tget.resize(numTensors, 0.0);

  Atget.resize(numMuls, 0.0);
  Btget.resize(numMuls, 0.0);

  T[I_13] = Tensor4(HOL, PAR, HOL, PAR, I_13);
  T[F_VV] = Tensor2(PAR, PAR, F_VV);
  T[I_11] = Tensor4(HOL, HOL, HOL, HOL, I_11);
  T[I_10] = Tensor2(PAR, PAR, I_10);
  T[V_OOOO] = Tensor4(HOL, HOL, HOL, HOL, V_OOOO);
  T[V_OOVV] = Tensor4(HOL, HOL, PAR, PAR, V_OOVV);
  T[V_OOOV] = Tensor4(HOL, HOL, HOL, PAR, V_OOOV);
  T[T_VVOO] = Tensor4(PAR, PAR, HOL, HOL, T_VVOO);
  T[I_12] = Tensor4(HOL, HOL, HOL, PAR, I_12);
  T[F_OV] = Tensor2(HOL, PAR, F_OV);
  T[V_OVOV] = Tensor4(HOL, PAR, HOL, PAR, V_OVOV);
  T[F_OO] = Tensor2(HOL, HOL, F_OO);
  T[V_OVOO] = Tensor4(HOL, PAR, HOL, HOL, V_OVOO);
  T[V_VVOV] = Tensor4(PAR, PAR, HOL, PAR, V_VVOV);
  T[I_9] = Tensor2(HOL, PAR, I_9);
  T[R] = Tensor4(PAR, PAR, HOL, HOL, R);
  T[V_VVOO] = Tensor4(PAR, PAR, HOL, HOL, V_VVOO);
  T[V_VVVV] = Tensor4(PAR, PAR, PAR, PAR, V_VVVV);
  T[V_OVVV] = Tensor4(HOL, PAR, PAR, PAR, V_OVVV);
  T[T_VO] = Tensor2(PAR, HOL, T_VO);
  T[I_8] = Tensor2(HOL, HOL, I_8);
  T[I_7] = Tensor4(PAR, PAR, HOL, PAR, I_7);
  T[I_6] = Tensor4(HOL, HOL, HOL, PAR, I_6);
  T[I_5] = Tensor2(HOL, PAR, I_5);
  T[I_4] = Tensor4(HOL, PAR, HOL, PAR, I_4);
  T[I_3] = Tensor4(HOL, HOL, HOL, PAR, I_3);
  T[I_2] = Tensor4(HOL, HOL, HOL, HOL, I_2);
  T[I_1] = Tensor4(HOL, PAR, HOL, HOL, I_1);

  setIndexName4(T[R], p3, p4, h1, h2);
  setIndexName4(T[V_VVOO], p3, p4, h1, h2);
  s[0] = Summation(T[R], T[V_VVOO], 1.0);
  setIndexName4(T[T_VVOO], p5, p6, h1, h2);
  setIndexName4(T[V_VVVV], p3, p4, p5, p6);
  setIndexName4(T[R], p3, p4, h1, h2);
  m[0] = Multiplication(T[R], T[T_VVOO], T[V_VVVV], 0.5);
  setIndexName4(T[I_1], h10, p3, h1, h2);
  setIndexName4(T[V_OVOO], h10, p3, h1, h2);
  s[1] = Summation(T[I_1], T[V_OVOO], 1.0);
  setIndexName4(T[T_VVOO], p5, p6, h1, h2);
  setIndexName4(T[V_OVVV], h10, p3, p5, p6);
  setIndexName4(T[I_1], h10, p3, h1, h2);
  m[1] = Multiplication(T[I_1], T[T_VVOO], T[V_OVVV], 0.5);
  setIndexName4(T[I_2], h10, h11, h1, h2);
  setIndexName4(T[V_OOOO], h10, h11, h1, h2);
  s[2] = Summation(T[I_2], T[V_OOOO], -1.0);
  setIndexName4(T[T_VVOO], p7, p8, h1, h2);
  setIndexName4(T[V_OOVV], h10, h11, p7, p8);
  setIndexName4(T[I_2], h10, h11, h1, h2);
  m[2] = Multiplication(T[I_2], T[T_VVOO], T[V_OOVV], -0.5);
  setIndexName4(T[I_3], h10, h11, h1, p5);
  setIndexName4(T[V_OOOV], h10, h11, h1, p5);
  s[3] = Summation(T[I_3], T[V_OOOV], 1.0);
  setIndexName2(T[T_VO], p6, h1);
  setIndexName4(T[V_OOVV], h10, h11, p5, p6);
  setIndexName4(T[I_3], h10, h11, h1, p5);
  m[3] = Multiplication(T[I_3], T[T_VO], T[V_OOVV], -0.5);
  setIndexName2(T[T_VO], p5, h1);
  setIndexName4(T[I_3], h10, h11, h2, p5);
  setIndexName4(T[I_2], h10, h11, h1, h2);
  m[4] = Multiplication(T[I_2], T[T_VO], T[I_3], 1.0);
  setIndexName2(T[T_VO], p3, h11);
  setIndexName4(T[I_2], h10, h11, h1, h2);
  setIndexName4(T[I_1], h10, p3, h1, h2);
  m[5] = Multiplication(T[I_1], T[T_VO], T[I_2], 0.5);
  setIndexName4(T[I_4], h10, p3, h1, p5);
  setIndexName4(T[V_OVOV], h10, p3, h1, p5);
  s[4] = Summation(T[I_4], T[V_OVOV], 1.0);
  setIndexName2(T[T_VO], p6, h1);
  setIndexName4(T[V_OVVV], h10, p3, p5, p6);
  setIndexName4(T[I_4], h10, p3, h1, p5);
  m[6] = Multiplication(T[I_4], T[T_VO], T[V_OVVV], -0.5);
  setIndexName2(T[T_VO], p5, h1);
  setIndexName4(T[I_4], h10, p3, h2, p5);
  setIndexName4(T[I_1], h10, p3, h1, h2);
  m[7] = Multiplication(T[I_1], T[T_VO], T[I_4], -1.0);
  setIndexName2(T[I_5], h10, p5);
  setIndexName2(T[F_OV], h10, p5);
  s[5] = Summation(T[I_5], T[F_OV], 1.0);
  setIndexName2(T[T_VO], p6, h7);
  setIndexName4(T[V_OOVV], h7, h10, p5, p6);
  setIndexName2(T[I_5], h10, p5);
  m[8] = Multiplication(T[I_5], T[T_VO], T[V_OOVV], -1.0);
  setIndexName4(T[T_VVOO], p3, p5, h1, h2);
  setIndexName2(T[I_5], h10, p5);
  setIndexName4(T[I_1], h10, p3, h1, h2);
  m[9] = Multiplication(T[I_1], T[T_VVOO], T[I_5], -1.0);
  setIndexName4(T[I_6], h7, h10, h1, p9);
  setIndexName4(T[V_OOOV], h7, h10, h1, p9);
  s[6] = Summation(T[I_6], T[V_OOOV], 1.0);
  setIndexName2(T[T_VO], p5, h1);
  setIndexName4(T[V_OOVV], h7, h10, p5, p9);
  setIndexName4(T[I_6], h7, h10, h1, p9);
  m[10] = Multiplication(T[I_6], T[T_VO], T[V_OOVV], 1.0);
  setIndexName4(T[T_VVOO], p3, p9, h1, h7);
  setIndexName4(T[I_6], h7, h10, h2, p9);
  setIndexName4(T[I_1], h10, p3, h1, h2);
  m[11] = Multiplication(T[I_1], T[T_VVOO], T[I_6], 1.0);
  setIndexName2(T[T_VO], p3, h10);
  setIndexName4(T[I_1], h10, p4, h1, h2);
  setIndexName4(T[R], p3, p4, h1, h2);
  m[12] = Multiplication(T[R], T[T_VO], T[I_1], -1.0);
  setIndexName4(T[I_7], p3, p4, h1, p5);
  setIndexName4(T[V_VVOV], p3, p4, h1, p5);
  s[7] = Summation(T[I_7], T[V_VVOV], 1.0);
  setIndexName2(T[T_VO], p6, h1);
  setIndexName4(T[V_VVVV], p3, p4, p5, p6);
  setIndexName4(T[I_7], p3, p4, h1, p5);
  m[13] = Multiplication(T[I_7], T[T_VO], T[V_VVVV], -0.5);
  setIndexName2(T[T_VO], p5, h1);
  setIndexName4(T[I_7], p3, p4, h2, p5);
  setIndexName4(T[R], p3, p4, h1, h2);
  m[14] = Multiplication(T[R], T[T_VO], T[I_7], -1.0);
  setIndexName2(T[I_8], h9, h1);
  setIndexName2(T[F_OO], h9, h1);
  s[8] = Summation(T[I_8], T[F_OO], 1.0);
  setIndexName2(T[T_VO], p6, h7);
  setIndexName4(T[V_OOOV], h7, h9, h1, p6);
  setIndexName2(T[I_8], h9, h1);
  m[15] = Multiplication(T[I_8], T[T_VO], T[V_OOOV], -1.0);
  setIndexName4(T[T_VVOO], p6, p7, h1, h8);
  setIndexName4(T[V_OOVV], h8, h9, p6, p7);
  setIndexName2(T[I_8], h9, h1);
  m[16] = Multiplication(T[I_8], T[T_VVOO], T[V_OOVV], -0.5);
  setIndexName2(T[I_9], h9, p8);
  setIndexName2(T[F_OV], h9, p8);
  s[9] = Summation(T[I_9], T[F_OV], 1.0);
  setIndexName2(T[T_VO], p6, h7);
  setIndexName4(T[V_OOVV], h7, h9, p6, p8);
  setIndexName2(T[I_9], h9, p8);
  m[17] = Multiplication(T[I_9], T[T_VO], T[V_OOVV], 1.0);
  setIndexName2(T[T_VO], p8, h1);
  setIndexName2(T[I_9], h9, p8);
  setIndexName2(T[I_8], h9, h1);
  m[18] = Multiplication(T[I_8], T[T_VO], T[I_9], 1.0);
  setIndexName4(T[T_VVOO], p3, p4, h1, h9);
  setIndexName2(T[I_8], h9, h2);
  setIndexName4(T[R], p3, p4, h1, h2);
  m[19] = Multiplication(T[R], T[T_VVOO], T[I_8], -1.0);
  setIndexName2(T[I_10], p3, p5);
  setIndexName2(T[F_VV], p3, p5);
  s[10] = Summation(T[I_10], T[F_VV], 1.0);
  setIndexName2(T[T_VO], p6, h7);
  setIndexName4(T[V_OVVV], h7, p3, p5, p6);
  setIndexName2(T[I_10], p3, p5);
  m[20] = Multiplication(T[I_10], T[T_VO], T[V_OVVV], -1.0);
  setIndexName4(T[T_VVOO], p3, p6, h7, h8);
  setIndexName4(T[V_OOVV], h7, h8, p5, p6);
  setIndexName2(T[I_10], p3, p5);
  m[21] = Multiplication(T[I_10], T[T_VVOO], T[V_OOVV], -0.5);
  setIndexName4(T[T_VVOO], p3, p5, h1, h2);
  setIndexName2(T[I_10], p4, p5);
  setIndexName4(T[R], p3, p4, h1, h2);
  m[22] = Multiplication(T[R], T[T_VVOO], T[I_10], 1.0);
  setIndexName4(T[I_11], h9, h11, h1, h2);
  setIndexName4(T[V_OOOO], h9, h11, h1, h2);
  s[11] = Summation(T[I_11], T[V_OOOO], -1.0);
  setIndexName4(T[T_VVOO], p5, p6, h1, h2);
  setIndexName4(T[V_OOVV], h9, h11, p5, p6);
  setIndexName4(T[I_11], h9, h11, h1, h2);
  m[23] = Multiplication(T[I_11], T[T_VVOO], T[V_OOVV], -0.5);
  setIndexName4(T[I_12], h9, h11, h1, p8);
  setIndexName4(T[V_OOOV], h9, h11, h1, p8);
  s[12] = Summation(T[I_12], T[V_OOOV], 1.0);
  setIndexName2(T[T_VO], p6, h1);
  setIndexName4(T[V_OOVV], h9, h11, p6, p8);
  setIndexName4(T[I_12], h9, h11, h1, p8);
  m[24] = Multiplication(T[I_12], T[T_VO], T[V_OOVV], 0.5);
  setIndexName2(T[T_VO], p8, h1);
  setIndexName4(T[I_12], h9, h11, h2, p8);
  setIndexName4(T[I_11], h9, h11, h1, h2);
  m[25] = Multiplication(T[I_11], T[T_VO], T[I_12], 1.0);
  setIndexName4(T[T_VVOO], p3, p4, h9, h11);
  setIndexName4(T[I_11], h9, h11, h1, h2);
  setIndexName4(T[R], p3, p4, h1, h2);
  m[26] = Multiplication(T[R], T[T_VVOO], T[I_11], -0.5);
  setIndexName4(T[I_13], h6, p3, h1, p5);
  setIndexName4(T[V_OVOV], h6, p3, h1, p5);
  s[13] = Summation(T[I_13], T[V_OVOV], 1.0);
  setIndexName2(T[T_VO], p7, h1);
  setIndexName4(T[V_OVVV], h6, p3, p5, p7);
  setIndexName4(T[I_13], h6, p3, h1, p5);
  m[27] = Multiplication(T[I_13], T[T_VO], T[V_OVVV], -1.0);
  setIndexName4(T[T_VVOO], p3, p7, h1, h8);
  setIndexName4(T[V_OOVV], h6, h8, p5, p7);
  setIndexName4(T[I_13], h6, p3, h1, p5);
  m[28] = Multiplication(T[I_13], T[T_VVOO], T[V_OOVV], -0.5);
  setIndexName4(T[T_VVOO], p3, p5, h1, h6);
  setIndexName4(T[I_13], h6, p4, h2, p5);
  setIndexName4(T[R], p3, p4, h1, h2);
  m[29] = Multiplication(T[R], T[T_VVOO], T[I_13], -1.0);

  Barrier();
  if (!me) { cout << "layer 1\n"; cout.flush(); }
  TaskPool tp1 = gtc_create(AUTO_BODY_SIZE, NUM_TASKS, NULL, MPI_COMM_WORLD);
  exec(SUM,0,tp1);
  exec(MUL,0,tp1);
  exec(SUM,1,tp1);
  exec(MUL,1,tp1);
  exec(SUM,2,tp1);
  exec(MUL,2,tp1);
  exec(SUM,3,tp1);
  exec(MUL,3,tp1);
  exec(SUM,4,tp1);
  exec(MUL,6,tp1);
  exec(SUM,5,tp1);
  exec(MUL,8,tp1);
  exec(SUM,6,tp1);
  exec(MUL,10,tp1);
  exec(SUM,7,tp1);
  exec(MUL,13,tp1);
  exec(SUM,8,tp1);
  exec(MUL,15,tp1);
  exec(MUL,16,tp1);
  exec(SUM,9,tp1);
  exec(MUL,17,tp1);
  exec(SUM,10,tp1);
  exec(MUL,20,tp1);
  exec(MUL,21,tp1);
  exec(SUM,11,tp1);
  exec(MUL,23,tp1);
  exec(SUM,12,tp1);
  exec(MUL,24,tp1);
  exec(SUM,13,tp1);
  exec(MUL,27,tp1);
  exec(MUL,28,tp1);
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
  exec(MUL,4,tp2);
  exec(MUL,7,tp2);
  exec(MUL,9,tp2);
  exec(MUL,11,tp2);
  exec(MUL,14,tp2);
  exec(MUL,18,tp2);
  exec(MUL,22,tp2);
  exec(MUL,25,tp2);
  exec(MUL,29,tp2);
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
  exec(MUL,5,tp3);
  exec(MUL,19,tp3);
  exec(MUL,26,tp3);
#if TIMER
  MyTimer::start(TOT);
#endif
  gtc_process(tp3);
#if TIMER
  MyTimer::stop(TOT);
#endif


  Barrier();
  if (!me) { cout << "layer 4\n"; cout.flush(); }
  TaskPool tp4 = gtc_create(AUTO_BODY_SIZE, NUM_TASKS, NULL, MPI_COMM_WORLD);
  exec(MUL,12,tp4);
#if TIMER
  MyTimer::start(TOT);
#endif
  gtc_process(tp4);
#if TIMER
  MyTimer::stop(TOT);
#endif


#endif // CCSD
}

