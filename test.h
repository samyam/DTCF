#ifndef _TEST_H_
#define _TEST_H_

#include <string.h>
#include <sstream>
#include <iostream>
#include <mpi.h>
#include <assert.h>
#include "tensor.h"
#include "contraction.h"
#include "redistribute.h"
using namespace RRR;

#define DEBUG 0

#ifdef TEST_FILE
#define TEST_EXTERN
#else
#define TEST_EXTERN extern
#endif

TEST_EXTERN void contr_rotate(Grid* &g, int vgrid_side, int N);
TEST_EXTERN void rotation_test(Grid* &g, int vgrid_side, int N);
TEST_EXTERN void two_point_five_D_SUMMA(Grid* &g, int vgrid_side, int N);
TEST_EXTERN void redistribute_test(Grid* &g, int vgrid_side, int N);
TEST_EXTERN void reduction_test(Grid* &g);
TEST_EXTERN void big_matrix_transpose_test(Grid* &g, int vgrid_side, int N);
TEST_EXTERN void contraction_1(Grid* &g, int vgrid_side, int N);
TEST_EXTERN void contraction_2(int* &pgrid_input, int vgrid_side, int N);
TEST_EXTERN void contraction_3(int* &pgrid_input, int vgrid_side, int N);
TEST_EXTERN void contraction_4(int* &pgrid_input, int vgrid_side, int N);
TEST_EXTERN void contraction_5(int* &pgrid_input, int vgrid_side, int N);
TEST_EXTERN void contraction_6(int* &pgrid_input, int vgrid_side, int N);
TEST_EXTERN void contraction_7(int* &pgrid_input, int vgrid_side, int N);
TEST_EXTERN void contraction_4_1(int* &pgrid_input, int vgrid_side, int N);
TEST_EXTERN void contraction_5_1(int* &pgrid_input, int vgrid_side, int N);

TEST_EXTERN void contraction_2d_4d();

TEST_EXTERN void contraction_2d_2d();

//tests transpose function
TEST_EXTERN void test_transpose(int N, int ii0, int i1, int i2, int i3);

TEST_EXTERN void test_contract_4d_4d();

TEST_EXTERN void serialize_test();

TEST_EXTERN void multiple_contr_test();

TEST_EXTERN void rect_grid_instigation(Grid* &g);

TEST_EXTERN void rect_identify_receivers(Grid* &g);

#endif
