#ifndef _TRANSP_KERNEL_H_
#define _TRANSP_KERNEL_H_

void transpose_ij(const double * in, double * out, int di, int dj);
void transpose_ji(const double * in, double * out, int di, int dj);
void transpose_ijkl(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_ijlk(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_ikjl(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_iklj(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_iljk(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_ilkj(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_jikl(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_jilk(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_jkil(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_jkli(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_jlik(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_jlki(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_kijl(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_kilj(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_kjil(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_kjli(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_klij(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_klji(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_lijk(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_likj(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_ljik(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_ljki(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_lkij(const double * in, double * out, int di, int dj, int dk, int dl);
void transpose_lkji(const double * in, double * out, int di, int dj, int dk, int dl);

#endif
