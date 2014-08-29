#include "cc.h"

void ccd() {

#if CCD
  m.resize(NMULT);
  s.resize(NSUMM);

  Barrier();

  enum TensorName {
    H1, H2, H3, H4, H5, H6, H7, H8, H9, H10,
    H11, H12, H13, H14, H15, H16, H17, H18, H19, H20,
    H21, H22, S23, S24, S25, Z27, Z28, Z29, Z30_1,
    Z31_1, Z32_1, Z33_1, Z34_1, Z35_1,
    Z36_1, Z37_1, Z38_1, Z39_1, Z40_1, Z41_1
  };
  int numTensors = 40;
  vector<Tensor> T(numTensors);

  T[Z31_1] = Tensor2(HOL, HOL, Z31_1);
  T[Z30_1] = Tensor2(HOL, HOL, Z30_1);
  T[H1] = Tensor2(HOL, HOL, H1 );
  T[H4] = Tensor2(HOL, HOL, H4 );
  T[Z34_1] = Tensor2(PAR, PAR, Z34_1);
  T[Z32_1] = Tensor2(PAR, PAR, Z32_1);
  T[H15] = Tensor2(PAR, PAR, H15);
  T[H9] = Tensor2(PAR, PAR, H9 );
  T[Z27] = Tensor4(HOL, HOL, PAR, PAR, Z27);
  T[Z39_1] = Tensor4(HOL, PAR, PAR, HOL, Z39_1);
  T[H21] = Tensor4(PAR, PAR, HOL, HOL, H21);
  T[H20] = Tensor4(PAR, PAR, PAR, PAR, H20);
  T[Z29] = Tensor4(HOL, HOL, PAR, PAR, Z29);
  T[Z28] = Tensor4(HOL, HOL, PAR, PAR, Z28);
  T[Z36_1] = Tensor4(PAR, PAR, PAR, PAR, Z36_1);
  T[Z37_1] = Tensor4(HOL, PAR, PAR, HOL, Z37_1);
  T[H8] = Tensor4(HOL, HOL, PAR, PAR, H8 );
  T[Z35_1] = Tensor4(PAR, PAR, PAR, PAR, Z35_1);
  T[H18] = Tensor4(HOL, PAR, PAR, HOL, H18);
  T[H19] = Tensor4(PAR, PAR, HOL, HOL, H19);
  T[H10] = Tensor4(HOL, PAR, PAR, HOL, H10);
  T[H11] = Tensor4(HOL, PAR, PAR, HOL, H11);
  T[H12] = Tensor4(HOL, PAR, PAR, HOL, H12);
  T[H13] = Tensor4(PAR, PAR, HOL, HOL, H13);
  T[H14] = Tensor4(PAR, PAR, PAR, PAR, H14);
  T[Z33_1] = Tensor4(PAR, PAR, PAR, PAR, Z33_1);
  T[H16] = Tensor4(HOL, PAR, PAR, HOL, H16);
  T[S23] = Tensor4(HOL, HOL, PAR, PAR, S23);
  T[S24] = Tensor4(HOL, HOL, PAR, PAR, S24);
  T[S25] = Tensor4(HOL, HOL, PAR, PAR, S25);
  T[H17] = Tensor4(HOL, PAR, PAR, HOL, H17);
  T[H22] = Tensor4(PAR, PAR, PAR, PAR, H22);
  T[Z38_1] = Tensor4(HOL, PAR, PAR, HOL, Z38_1);
  T[H2] = Tensor4(HOL, HOL, HOL, HOL, H2 );
  T[H3] = Tensor4(HOL, HOL, PAR, PAR, H3 );
  T[H6] = Tensor4(HOL, HOL, PAR, PAR, H6 );
  T[H7] = Tensor4(HOL, HOL, HOL, HOL, H7 );
  T[H5] = Tensor4(HOL, HOL, HOL, HOL, H5 );
  T[Z41_1] = Tensor4(HOL, PAR, PAR, HOL, Z41_1);
  T[Z40_1] = Tensor4(HOL, PAR, PAR, HOL, Z40_1);

  setIndexName4(T[Z27], e1b, e2b, m1b, m2b);
  setIndexName4(T[H2], e1b, e2b, d1b, d2b);
  setIndexName4(T[S23], d1b, d2b, m1b, m2b);
  m[0] = Multiplication(T[Z27], T[H2], T[S23], 1.0);

  setIndexName4(T[Z28], e1a, e1b, m1a, m1b);
  setIndexName4(T[S24], e1b, d1a, m1a, l1b);
  setIndexName4(T[H11], e1a, l1b, m1b, d1a);
  m[1] = Multiplication(T[Z28], T[S24], T[H11], 1.0);

  setIndexName4(T[Z28], e1a, e1b, m1a, m1b);
  setIndexName4(T[H5], e1a, e1b, d1a, d1b);
  setIndexName4(T[S24], d1a, d1b, m1a, m1b);
  m[2] = Multiplication(T[Z28], T[H5], T[S24], 1.0);

  setIndexName4(T[Z28], e1a, e1b, m1a, m1b);
  setIndexName4(T[S23], e1b, d1b, m1b, l1b);
  setIndexName4(T[H12], e1a, l1b, m1a, d1b);
  m[3] = Multiplication(T[Z28], T[S23], T[H12], 1.0);

  setIndexName4(T[Z29], e1a, e2a, m1a, m2a);
  setIndexName4(T[H7], e1a, e2a, d1a, d2a);
  setIndexName4(T[S25], d1a, d2a, m1a, m2a);
  m[4] = Multiplication(T[Z29], T[H7], T[S25], 1.0);

  setIndexName2(T[Z30_1], e1b, d1b);
  setIndexName4(T[S23], e1b, d2b, l1b, l2b);
  setIndexName4(T[H13], l1b, l2b, d1b, d2b);
  m[5] = Multiplication(T[Z30_1], T[S23], T[H13], 1.0);

  setIndexName2(T[Z30_1], e1b, d1b);
  setIndexName4(T[S23], e1b, d2b, l1b, l2b);
  setIndexName4(T[H13], l1b, l2b, d1b, d2b);
  m[6] = Multiplication(T[Z30_1], T[S23], T[H13], 1.0);

  setIndexName2(T[Z30_1], e1b, d1b);
  setIndexName4(T[S24], e1b, d1a, l1a, l1b);
  setIndexName4(T[H19], l1a, l1b, d1b, d1a);
  m[7] = Multiplication(T[Z30_1], T[S24], T[H19], 1.0);

  setIndexName2(T[Z30_1], e1b, d1b);
  setIndexName4(T[S24], e1b, d1a, l1a, l1b);
  setIndexName4(T[H19], l1a, l1b, d1b, d1a);
  m[8] = Multiplication(T[Z30_1], T[S24], T[H19], 1.0);

  setIndexName2(T[Z31_1], e1a, d1a);
  setIndexName4(T[S24], e1a, d1b, l1a, l1b);
  setIndexName4(T[H19], l1a, l1b, d1b, d1a);
  m[9] = Multiplication(T[Z31_1], T[S24], T[H19], 1.0);

  setIndexName2(T[Z31_1], e1a, d1a);
  setIndexName4(T[S25], e1a, d2a, l1a, l2a);
  setIndexName4(T[H21], l1a, l2a, d1a, d2a);
  m[10] = Multiplication(T[Z31_1], T[S25], T[H21], 1.0);

  setIndexName2(T[Z31_1], e1a, d1a);
  setIndexName4(T[S24], e1a, d1b, l1a, l1b);
  setIndexName4(T[H19], l1a, l1b, d1b, d1a);
  m[11] = Multiplication(T[Z31_1], T[S24], T[H19], 1.0);

  setIndexName2(T[Z31_1], e1a, d1a);
  setIndexName4(T[S25], e1a, d2a, l1a, l2a);
  setIndexName4(T[H21], l1a, l2a, d1a, d2a);
  m[12] = Multiplication(T[Z31_1], T[S25], T[H21], 1.0);

  setIndexName2(T[Z32_1], l1b, m1b);
  setIndexName4(T[S23], d1b, d2b, m1b, l2b);
  setIndexName4(T[H13], l1b, l2b, d1b, d2b);
  m[13] = Multiplication(T[Z32_1], T[S23], T[H13], 1.0);

  setIndexName2(T[Z32_1], l1b, m1b);
  setIndexName4(T[S23], d1b, d2b, m1b, l2b);
  setIndexName4(T[H13], l1b, l2b, d1b, d2b);
  m[14] = Multiplication(T[Z32_1], T[S23], T[H13], 1.0);

  setIndexName2(T[Z32_1], l1b, m1b);
  setIndexName4(T[S24], d1a, d1b, m1b, l1a);
  setIndexName4(T[H19], l1b, l1a, d1a, d1b);
  m[15] = Multiplication(T[Z32_1], T[S24], T[H19], 1.0);

  setIndexName2(T[Z32_1], l1b, m1b);
  setIndexName4(T[S24], d1a, d1b, m1b, l1a);
  setIndexName4(T[H19], l1b, l1a, d1a, d1b);
  m[16] = Multiplication(T[Z32_1], T[S24], T[H19], 1.0);

  setIndexName4(T[Z33_1], l1b, l2b, m1b, m2b);
  setIndexName4(T[S23], d1b, d2b, m1b, m2b);
  setIndexName4(T[H13], l1b, l2b, d1b, d2b);
  m[17] = Multiplication(T[Z33_1], T[S23], T[H13], 1.0);

  setIndexName2(T[Z34_1], l1a, m1a);
  setIndexName4(T[S24], d1a, d1b, m1a, l1b);
  setIndexName4(T[H19], l1b, l1a, d1a, d1b);
  m[18] = Multiplication(T[Z34_1], T[S24], T[H19], 1.0);

  setIndexName2(T[Z34_1], l1a, m1a);
  setIndexName4(T[S25], d1a, d2a, m1a, l2a);
  setIndexName4(T[H21], l1a, l2a, d1a, d2a);
  m[19] = Multiplication(T[Z34_1], T[S25], T[H21], 1.0);

  setIndexName2(T[Z34_1], l1a, m1a);
  setIndexName4(T[S24], d1a, d1b, m1a, l1b);
  setIndexName4(T[H19], l1b, l1a, d1a, d1b);
  m[20] = Multiplication(T[Z34_1], T[S24], T[H19], 1.0);

  setIndexName2(T[Z34_1], l1a, m1a);
  setIndexName4(T[S25], d1a, d2a, m1a, l2a);
  setIndexName4(T[H21], l1a, l2a, d1a, d2a);
  m[21] = Multiplication(T[Z34_1], T[S25], T[H21], 1.0);

  setIndexName4(T[Z35_1], l1a, l1b, m1a, m1b);
  setIndexName4(T[S24], d1a, d1b, m1a, m1b);
  setIndexName4(T[H19], l1a, l1b, d1a, d1b);
  m[22] = Multiplication(T[Z35_1], T[S24], T[H19], 1.0);

  setIndexName4(T[Z36_1], l1a, l2a, m1a, m2a);
  setIndexName4(T[S25], d1a, d2a, m1a, m2a);
  setIndexName4(T[H21], l1a, l2a, d1a, d2a);
  m[23] = Multiplication(T[Z36_1], T[S25], T[H21], 1.0);

  setIndexName4(T[Z37_1], e1b, l1b, m1b, d1b);
  setIndexName4(T[S23], e1b, d2b, m1b, l2b);
  setIndexName4(T[H13], l1b, l2b, d1b, d2b);
  m[24] = Multiplication(T[Z37_1], T[S23], T[H13], 1.0);

  setIndexName4(T[Z37_1], e1b, l1b, m1b, d1b);
  setIndexName4(T[S23], e1b, d2b, m1b, l2b);
  setIndexName4(T[H13], l1b, l2b, d1b, d2b);
  m[25] = Multiplication(T[Z37_1], T[S23], T[H13], 1.0);

  setIndexName4(T[Z38_1], e1b, l1a, m1b, d1a);
  setIndexName4(T[S24], e1b, d2a, m1b, l2a);
  setIndexName4(T[H21], l1a, l2a, d1a, d2a);
  m[26] = Multiplication(T[Z38_1], T[S24], T[H21], 1.0);

  setIndexName4(T[Z38_1], e1b, l1a, m1b, d1a);
  setIndexName4(T[S23], e1b, d1b, m1b, l1b);
  setIndexName4(T[H19], l1b, l1a, d1b, d1a);
  m[27] = Multiplication(T[Z38_1], T[S23], T[H19], 1.0);

  setIndexName4(T[Z38_1], e1b, l1a, m1b, d1a);
  setIndexName4(T[S23], e1b, d1b, m1b, l1b);
  setIndexName4(T[H19], l1b, l1a, d1b, d1a);
  m[28] = Multiplication(T[Z38_1], T[S23], T[H19], 1.0);

  setIndexName4(T[Z38_1], e1b, l1a, m1b, d1a);
  setIndexName4(T[S24], e1b, d2a, m1b, l2a);
  setIndexName4(T[H21], l1a, l2a, d1a, d2a);
  m[29] = Multiplication(T[Z38_1], T[S24], T[H21], 1.0);

  setIndexName4(T[Z39_1], e1b, l1a, m1a, d1b);
  setIndexName4(T[S24], e1b, d1a, m1a, l1b);
  setIndexName4(T[H19], l1b, l1a, d1b, d1a);
  m[30] = Multiplication(T[Z39_1], T[S24], T[H19], 1.0);

  setIndexName4(T[Z40_1], e1a, l1b, m1a, d1b);
  setIndexName4(T[S24], e1a, d2b, m1a, l2b);
  setIndexName4(T[H13], l1b, l2b, d1b, d2b);
  m[31] = Multiplication(T[Z40_1], T[S24], T[H13], 1.0);

  setIndexName4(T[Z41_1], e1a, l1a, m1a, d1a);
  setIndexName4(T[S25], e1a, d2a, m1a, l2a);
  setIndexName4(T[H21], l1a, l2a, d1a, d2a);
  m[32] = Multiplication(T[Z41_1], T[S25], T[H21], 1.0);

  setIndexName4(T[Z41_1], e1a, l1a, m1a, d1a);
  setIndexName4(T[S24], e1a, d1b, m1a, l1b);
  setIndexName4(T[H19], l1b, l1a, d1b, d1a);
  m[33] = Multiplication(T[Z41_1], T[S24], T[H19], 1.0);

  setIndexName4(T[Z41_1], e1a, l1a, m1a, d1a);
  setIndexName4(T[S24], e1a, d1b, m1a, l1b);
  setIndexName4(T[H19], l1b, l1a, d1b, d1a);
  m[34] = Multiplication(T[Z41_1], T[S24], T[H19], 1.0);

  setIndexName4(T[Z27], e1b, e2b, m1b, m2b);
  setIndexName4(T[H3], e1b, e2b, m1b, m2b);
  s[0] = Summation(T[Z27], T[H3], 1.0);

  setIndexName4(T[Z28], e1a, e1b, m1a, m1b);
  setIndexName4(T[H6], e1a, e1b, m1a, m1b);
  s[1] = Summation(T[Z28], T[H6], 1.0);

  setIndexName4(T[Z29], e1a, e2a, m1a, m2a);
  setIndexName4(T[H8], e1a, e2a, m1a, m2a);
  s[2] = Summation(T[Z29], T[H8], 1.0);

  setIndexName2(T[Z30_1], e1b, d1b);
  setIndexName2(T[H1], e1b, d1b);
  s[3] = Summation(T[Z30_1], T[H1], 1.0);

  setIndexName2(T[Z30_1], e1b, d1b);
  setIndexName2(T[H1], e1b, d1b);
  s[4] = Summation(T[Z30_1], T[H1], 1.0);

  setIndexName2(T[Z31_1], e1a, d1a);
  setIndexName2(T[H4], e1a, d1a);
  s[5] = Summation(T[Z31_1], T[H4], 1.0);

  setIndexName2(T[Z31_1], e1a, d1a);
  setIndexName2(T[H4], e1a, d1a);
  s[6] = Summation(T[Z31_1], T[H4], 1.0);

  setIndexName2(T[Z32_1], l1b, m1b);
  setIndexName2(T[H9], l1b, m1b);
  s[7] = Summation(T[Z32_1], T[H9], 1.0);

  setIndexName2(T[Z32_1], l1b, m1b);
  setIndexName2(T[H9], l1b, m1b);
  s[8] = Summation(T[Z32_1], T[H9], 1.0);

  setIndexName4(T[Z33_1], l1b, l2b, m1b, m2b);
  setIndexName4(T[H14], l1b, l2b, m1b, m2b);
  s[9] = Summation(T[Z33_1], T[H14], 1.0);

  setIndexName2(T[Z34_1], l1a, m1a);
  setIndexName2(T[H15], l1a, m1a);
  s[10] = Summation(T[Z34_1], T[H15], 1.0);

  setIndexName2(T[Z34_1], l1a, m1a);
  setIndexName2(T[H15], l1a, m1a);
  s[11] = Summation(T[Z34_1], T[H15], 1.0);

  setIndexName4(T[Z35_1], l1a, l1b, m1a, m1b);
  setIndexName4(T[H20], l1a, l1b, m1a, m1b);
  s[12] = Summation(T[Z35_1], T[H20], 1.0);

  setIndexName4(T[Z36_1], l1a, l2a, m1a, m2a);
  setIndexName4(T[H22], l1a, l2a, m1a, m2a);
  s[13] = Summation(T[Z36_1], T[H22], 1.0);

  setIndexName4(T[Z37_1], e1b, l1b, m1b, d1b);
  setIndexName4(T[H10], e1b, l1b, m1b, d1b);
  s[14] = Summation(T[Z37_1], T[H10], 1.0);

  setIndexName4(T[Z37_1], e1b, l1b, m1b, d1b);
  setIndexName4(T[H10], e1b, l1b, m1b, d1b);
  s[15] = Summation(T[Z37_1], T[H10], 1.0);

  setIndexName4(T[Z38_1], e1b, l1a, m1b, d1a);
  setIndexName4(T[H16], e1b, l1a, m1b, d1a);
  s[16] = Summation(T[Z38_1], T[H16], 1.0);

  setIndexName4(T[Z38_1], e1b, l1a, m1b, d1a);
  setIndexName4(T[H16], e1b, l1a, m1b, d1a);
  s[17] = Summation(T[Z38_1], T[H16], 1.0);

  setIndexName4(T[Z39_1], e1b, l1a, m1a, d1b);
  setIndexName4(T[H17], e1b, l1a, m1a, d1b);
  s[18] = Summation(T[Z39_1], T[H17], 1.0);

  setIndexName4(T[Z40_1], e1a, l1b, m1a, d1b);
  setIndexName4(T[H12], e1a, l1b, m1a, d1b);
  s[19] = Summation(T[Z40_1], T[H12], 1.0);

  setIndexName4(T[Z41_1], e1a, l1a, m1a, d1a);
  setIndexName4(T[H18], e1a, l1a, m1a, d1a);
  s[20] = Summation(T[Z41_1], T[H18], 1.0);

  setIndexName4(T[Z41_1], e1a, l1a, m1a, d1a);
  setIndexName4(T[H18], e1a, l1a, m1a, d1a);
  s[21] = Summation(T[Z41_1], T[H18], 1.0);

  setIndexName4(T[Z27], e1b, e2b, m1b, m2b);
  setIndexName4(T[S23], e1b, d1b, m2b, l1b);
  setIndexName4(T[Z37_1], e2b, l1b, m1b, d1b);
  m[35] = Multiplication(T[Z27], T[S23], T[Z37_1], 1.0);

  setIndexName4(T[Z27], e1b, e2b, m1b, m2b);
  setIndexName4(T[S23], e1b, e2b, m2b, l1b);
  setIndexName2(T[Z32_1], l1b, m1b);
  m[36] = Multiplication(T[Z27], T[S23], T[Z32_1], 1.0);

  setIndexName4(T[Z27], e1b, e2b, m1b, m2b);
  setIndexName4(T[S23], e2b, d1b, m1b, l1b);
  setIndexName4(T[Z37_1], e1b, l1b, m2b, d1b);
  m[37] = Multiplication(T[Z27], T[S23], T[Z37_1], 1.0);

  setIndexName4(T[Z27], e1b, e2b, m1b, m2b);
  setIndexName4(T[S23], e2b, d1b, m1b, m2b);
  setIndexName2(T[Z30_1], e1b, d1b);
  m[38] = Multiplication(T[Z27], T[S23], T[Z30_1], 1.0);

  setIndexName4(T[Z27], e1b, e2b, m1b, m2b);
  setIndexName4(T[S24], e1b, d1a, m2b, l1a);
  setIndexName4(T[Z38_1], e2b, l1a, m1b, d1a);
  m[39] = Multiplication(T[Z27], T[S24], T[Z38_1], 1.0);

  setIndexName4(T[Z27], e1b, e2b, m1b, m2b);
  setIndexName4(T[S24], e2b, d1a, m1b, l1a);
  setIndexName4(T[Z38_1], e1b, l1a, m2b, d1a);
  m[40] = Multiplication(T[Z27], T[S24], T[Z38_1], 1.0);

  setIndexName4(T[Z27], e1b, e2b, m1b, m2b);
  setIndexName4(T[S23], e1b, d1b, m1b, l1b);
  setIndexName4(T[Z37_1], e2b, l1b, m2b, d1b);
  m[41] = Multiplication(T[Z27], T[S23], T[Z37_1], 1.0);

  setIndexName4(T[Z27], e1b, e2b, m1b, m2b);
  setIndexName4(T[S23], e1b, d1b, m1b, m2b);
  setIndexName2(T[Z30_1], e2b, d1b);
  m[42] = Multiplication(T[Z27], T[S23], T[Z30_1], 1.0);

  setIndexName4(T[Z27], e1b, e2b, m1b, m2b);
  setIndexName4(T[S23], e1b, e2b, l1b, l2b);
  setIndexName4(T[Z33_1], l1b, l2b, m1b, m2b);
  m[43] = Multiplication(T[Z27], T[S23], T[Z33_1], 1.0);

  setIndexName4(T[Z27], e1b, e2b, m1b, m2b);
  setIndexName4(T[S23], e1b, e2b, m1b, l1b);
  setIndexName2(T[Z32_1], l1b, m2b);
  m[44] = Multiplication(T[Z27], T[S23], T[Z32_1], 1.0);

  setIndexName4(T[Z27], e1b, e2b, m1b, m2b);
  setIndexName4(T[S23], e2b, d1b, m2b, l1b);
  setIndexName4(T[Z37_1], e1b, l1b, m1b, d1b);
  m[45] = Multiplication(T[Z27], T[S23], T[Z37_1], 1.0);

  setIndexName4(T[Z27], e1b, e2b, m1b, m2b);
  setIndexName4(T[S24], e1b, d1a, m1b, l1a);
  setIndexName4(T[Z38_1], e2b, l1a, m2b, d1a);
  m[46] = Multiplication(T[Z27], T[S24], T[Z38_1], 1.0);

  setIndexName4(T[Z27], e1b, e2b, m1b, m2b);
  setIndexName4(T[S24], e2b, d1a, m2b, l1a);
  setIndexName4(T[Z38_1], e1b, l1a, m1b, d1a);
  m[47] = Multiplication(T[Z27], T[S24], T[Z38_1], 1.0);

  setIndexName4(T[Z28], e1a, e1b, m1a, m1b);
  setIndexName4(T[S24], e1a, d1b, m1a, l1b);
  setIndexName4(T[Z37_1], e1b, l1b, m1b, d1b);
  m[48] = Multiplication(T[Z28], T[S24], T[Z37_1], 1.0);

  setIndexName4(T[Z28], e1a, e1b, m1a, m1b);
  setIndexName4(T[S24], e1a, d1b, m1a, m1b);
  setIndexName2(T[Z30_1], e1b, d1b);
  m[49] = Multiplication(T[Z28], T[S24], T[Z30_1], 1.0);

  setIndexName4(T[Z28], e1a, e1b, m1a, m1b);
  setIndexName4(T[S24], e1a, d1b, m1b, l1a);
  setIndexName4(T[Z39_1], e1b, l1a, m1a, d1b);
  m[50] = Multiplication(T[Z28], T[S24], T[Z39_1], 1.0);

  setIndexName4(T[Z28], e1a, e1b, m1a, m1b);
  setIndexName4(T[S24], e1a, e1b, l1a, l1b);
  setIndexName4(T[Z35_1], l1a, l1b, m1a, m1b);
  m[51] = Multiplication(T[Z28], T[S24], T[Z35_1], 1.0);

  setIndexName4(T[Z28], e1a, e1b, m1a, m1b);
  setIndexName4(T[S24], e1a, e1b, m1a, l1b);
  setIndexName2(T[Z32_1], l1b, m1b);
  m[52] = Multiplication(T[Z28], T[S24], T[Z32_1], 1.0);

  setIndexName4(T[Z28], e1a, e1b, m1a, m1b);
  setIndexName4(T[S24], e1a, e1b, m1b, l1a);
  setIndexName2(T[Z34_1], l1a, m1a);
  m[53] = Multiplication(T[Z28], T[S24], T[Z34_1], 1.0);

  setIndexName4(T[Z28], e1a, e1b, m1a, m1b);
  setIndexName4(T[S24], e1b, d1a, m1a, m1b);
  setIndexName2(T[Z31_1], e1a, d1a);
  m[54] = Multiplication(T[Z28], T[S24], T[Z31_1], 1.0);

  setIndexName4(T[Z28], e1a, e1b, m1a, m1b);
  setIndexName4(T[S24], e1b, d1a, m1b, l1a);
  setIndexName4(T[Z41_1], e1a, l1a, m1a, d1a);
  m[55] = Multiplication(T[Z28], T[S24], T[Z41_1], 1.0);

  setIndexName4(T[Z28], e1a, e1b, m1a, m1b);
  setIndexName4(T[S25], e1a, d1a, m1a, l1a);
  setIndexName4(T[Z38_1], e1b, l1a, m1b, d1a);
  m[56] = Multiplication(T[Z28], T[S25], T[Z38_1], 1.0);

  setIndexName4(T[Z29], e1a, e2a, m1a, m2a);
  setIndexName4(T[S24], e1a, d1b, m2a, l1b);
  setIndexName4(T[Z40_1], e2a, l1b, m1a, d1b);
  m[57] = Multiplication(T[Z29], T[S24], T[Z40_1], 1.0);

  setIndexName4(T[Z29], e1a, e2a, m1a, m2a);
  setIndexName4(T[S24], e2a, d1b, m1a, l1b);
  setIndexName4(T[Z40_1], e1a, l1b, m2a, d1b);
  m[58] = Multiplication(T[Z29], T[S24], T[Z40_1], 1.0);

  setIndexName4(T[Z29], e1a, e2a, m1a, m2a);
  setIndexName4(T[S25], e1a, d1a, m2a, l1a);
  setIndexName4(T[Z41_1], e2a, l1a, m1a, d1a);
  m[59] = Multiplication(T[Z29], T[S25], T[Z41_1], 1.0);

  setIndexName4(T[Z29], e1a, e2a, m1a, m2a);
  setIndexName4(T[S25], e1a, e2a, m2a, l1a);
  setIndexName2(T[Z34_1], l1a, m1a);
  m[60] = Multiplication(T[Z29], T[S25], T[Z34_1], 1.0);

  setIndexName4(T[Z29], e1a, e2a, m1a, m2a);
  setIndexName4(T[S25], e2a, d1a, m1a, l1a);
  setIndexName4(T[Z41_1], e1a, l1a, m2a, d1a);
  m[61] = Multiplication(T[Z29], T[S25], T[Z41_1], 1.0);

  setIndexName4(T[Z29], e1a, e2a, m1a, m2a);
  setIndexName4(T[S25], e2a, d1a, m1a, m2a);
  setIndexName2(T[Z31_1], e1a, d1a);
  m[62] = Multiplication(T[Z29], T[S25], T[Z31_1], 1.0);

  setIndexName4(T[Z29], e1a, e2a, m1a, m2a);
  setIndexName4(T[S24], e1a, d1b, m1a, l1b);
  setIndexName4(T[Z40_1], e2a, l1b, m2a, d1b);
  m[63] = Multiplication(T[Z29], T[S24], T[Z40_1], 1.0);

  setIndexName4(T[Z29], e1a, e2a, m1a, m2a);
  setIndexName4(T[S24], e2a, d1b, m2a, l1b);
  setIndexName4(T[Z40_1], e1a, l1b, m1a, d1b);
  m[64] = Multiplication(T[Z29], T[S24], T[Z40_1], 1.0);

  setIndexName4(T[Z29], e1a, e2a, m1a, m2a);
  setIndexName4(T[S25], e1a, d1a, m1a, l1a);
  setIndexName4(T[Z41_1], e2a, l1a, m2a, d1a);
  m[65] = Multiplication(T[Z29], T[S25], T[Z41_1], 1.0);

  setIndexName4(T[Z29], e1a, e2a, m1a, m2a);
  setIndexName4(T[S25], e1a, d1a, m1a, m2a);
  setIndexName2(T[Z31_1], e2a, d1a);
  m[66] = Multiplication(T[Z29], T[S25], T[Z31_1], 1.0);

  setIndexName4(T[Z29], e1a, e2a, m1a, m2a);
  setIndexName4(T[S25], e1a, e2a, l1a, l2a);
  setIndexName4(T[Z36_1], l1a, l2a, m1a, m2a);
  m[67] = Multiplication(T[Z29], T[S25], T[Z36_1], 1.0);

  setIndexName4(T[Z29], e1a, e2a, m1a, m2a);
  setIndexName4(T[S25], e1a, e2a, m1a, l1a);
  setIndexName2(T[Z34_1], l1a, m2a);
  m[68] = Multiplication(T[Z29], T[S25], T[Z34_1], 1.0);

  setIndexName4(T[Z29], e1a, e2a, m1a, m2a);
  setIndexName4(T[S25], e2a, d1a, m2a, l1a);
  setIndexName4(T[Z41_1], e1a, l1a, m1a, d1a);
  m[69] = Multiplication(T[Z29], T[S25], T[Z41_1], 1.0);

  //s/\v,(\u\w*),/,vt[\1],

  Barrier();

  // level 1
#if TASCEL
  TslFuncRegTbl frt;
  TslFunc tf = frt.add(TaskFunc);
  TaskLocal tl;
  TaskCollProps props;
  props.functions(tf, &frt).taskSize(sizeof(TaskDesc))
    .maxTasks(100000).localData(&tl,sizeof(TaskLocal));
  UniformTaskCollectionSplit* tp1 = new UniformTaskCollectionSplit(props, 0);
#else
  TaskPool tp1 = gtc_create(AUTO_BODY_SIZE, NUM_TASKS, NULL, MPI_COMM_WORLD);
#endif

  for (int i = 0; i <= 34; i++) {
    exec(MUL, i, tp1);
  }
  for (int i = 0; i <= 21; i++) {
    exec(SUM, i, tp1);
  }

  if (!me) {
    cout << "level 1\n";
    cout.flush();
  }

  Barrier();

#if TIMER
  MyTimer::start(TOT);
#endif

#if TASCEL
  tp1->process();
  delete tp1;
#else
  gtc_process(tp1);
#endif

#if TIMER
  MyTimer::stop(TOT);
#endif

  Barrier();

  // level 2
#if TASCEL
  UniformTaskCollectionSplit *tp2 = new UniformTaskCollectionSplit(props, 0);
#else
  gtc_t tp2 = gtc_create(AUTO_BODY_SIZE, NUM_TASKS, NULL, MPI_COMM_WORLD);
#endif

  for (int i = 35; i <= 69; i++) {
    exec(MUL, i,tp2);
  }

  if (!me) {
    cout << "level 2\n";
    cout.flush();
  }
  Barrier();

#if TIMER
  MyTimer::start(TOT);
#endif

#if TASCEL
  tp2->process();
  delete tp2;
#else
  gtc_process(tp2);
#endif

#if TIMER
  MyTimer::stop(TOT);
#endif

  Barrier();

#endif // CCD
}

