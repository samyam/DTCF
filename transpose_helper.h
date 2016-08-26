#ifndef _TRANSPOSE_HELPER_H_
#define _TRANSPOSE_HELPER_H_

#include <stdlib.h>
#include <mpi.h>
#include <list>
#include <string>
#include <algorithm>
#include <map>
#include "tensor.h"
#include "block_grid.h"
#include <string.h>
#include <sstream>
#include <assert.h>
#include "omp.h"

#define DEBUG 0
namespace RRR{
using namespace std;
/*creates a 2D matrix from a list of multidimensional tensor tiles
this function works in the following way. It takes each block and 
places its data in the appropriate position in the 2D matrix. Each 
tensor block has multiple indices, which can be divided into row and
column indices. The input tiles are arranges so that the row indices 
appear first and the column indices appear later. The input variable
num_row_dim gives the number of indices that corresponds to rows. 
Using this, a row rank and a column rank is generated for each tile
which is then used to indetify the correct position of the data in
each tile in the 2D matrix. */
extern void create_big_matrix(Tensor* &T, double* &tensor_tiles, int* &permuted_address, 
			      int* &p_map_T, int num_tiles, int row_per_block, 
			      int col_per_block, int num_row_dim, 
			      map<int, map<int, int> >* &big_matrix_map, double* &big_matrix);

extern void create_big_matrix_nl(Tensor* &T, double* &tensor_tiles, int* &permuted_address,
			  int* &p_map_T, int num_tiles,int row_per_block, int col_per_block, 
			  int num_row_dim, map<int, map<int, int> >* &big_matrix_map, 
				 double* &big_matrix, map<int,int>* &address_to_location);

//revert big matrix to multidimensional tiles
extern void revert_big_matrix(Tensor* &T, map<int, map<int, int> >* &big_matrix_map, 
			      double* &big_matrix, int row_per_block, int col_per_block, 
			      int* &p_map_T, double* &tensor_tiles);
}
#endif
