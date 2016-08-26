#ifndef _HELPER_H_
#define _HELPER_H_

#include <stdlib.h>
#include <mpi.h>
#include <list>
#include <string>
#include <algorithm>
#include <map>


#include <string.h>
#include <sstream>
#include <assert.h>
#include "omp.h"

#define DEBUG 0

#ifdef HELPER_FILE
#define EXTERN
#else
#define EXTERN extern
#endif

// Find an element in the given array and return its index
// return -1 if not found
EXTERN int search_in_arr(int num, int* &arr, int arr_size);

// Returns rank of a processor given its multi-dimensional address
EXTERN int get_proc_rank(int* &address, int dims, int* &grid);

// Finds out ranks of n processors given their multi-dimensional addresses
EXTERN void get_proc_ranks(int n, int** &addresses, int dims, int* &grid, int* &ranks);

// Computes a processor's multi-dimensional address given its rank
EXTERN void get_proc_addr(int* &address, int dims, int* &grid, int rank);

// Computes n processors' multi-dimensional addresses given their ranks
EXTERN void get_proc_addrs(int n, int** &addresses, int dims, int* &grid, int* &ranks);

// Prints address of a processor given its rank
EXTERN void print_proc_addr(int dims, int* &grid, int rank);

// Returns the string of the processor's multi-dimensional address, given its rank
EXTERN std::string get_proc_addr_str(int dims, int* &grid, int rank);

// Prints the address of a tile
EXTERN void print_tile_addr(int dims, int* &addr);

// Prints the addresses of a series of tiles
EXTERN void print_tile_addrs(int dims, int* &addr, int num_tiles);

//Prints if the addresses in addr1 and addr2 are the same or not
EXTERN void are_tile_addrs_equal(int dims, int* &addr1, int* &addr2, int num_tiles);

// Prints the data in a tileOA
EXTERN void print_block_data(int block_size, double* &block_addr);

//returns 0 if the addresses are not the same, else returns 1
EXTERN int is_address_equal(int dims, int* addr1, int* addr2);

// Swaps two integers
EXTERN void swap(int* a, int* b);

// compute greatest common divisor
EXTERN int gcd(int a, int b);

// compute lowest common multiple
EXTERN int lcm(int a, int b);

/*points_per_dim is an array of arrays. The inner dimension
 * represents the dimension of the processor grid while the outer
 * dimension represents the processor addresses along that
 * dimension. The size_per_dim stores the number of processor
 * along each dimension of points_per_dim. The
 * create_cross_products fills output which is an array of
 * processor addresses. The addresses created by this function is
 * a tuple of dims, given by the cross products of addresses along
 * each dimension given by points_per_dims*/ 
EXTERN void create_cross_product(int** &points_per_dim, int* &size_per_dim, 
				 int &dims, int cur_dim, 
				 int &offset, int* &current_addr,
				 int** &output);

EXTERN void printGetTiles(double* data_block, int* tile_address, int blocksize, int num_blocks, int dims);

// Finds block number of a block in the block grid based on its virtual address
EXTERN int get_block_number(int* &block_addr, int dims, int* &vgrid);

// Finds block address from the block number in block grid
EXTERN void get_block_addr(int* &block_addr, int dims, int* &vgrid, int block_number);

// Assert if the number of nodes allocated and the size of pgrid defined in code are equal
EXTERN void assert_pgrid_size(int size, int dims, int* &pgrid);

//returns 1 if two blocks are equal
EXTERN int is_equal_blocks(int block_size, double* &block_addr1, double* &block_addr2);

//returns 1 if two blocks are equal
EXTERN int are_equal_blocks(int block_size, double* &block_addr1, double* &block_addr2, int num_blocks);



//transposes the block based on permutation map
EXTERN void transpose(double* &input_buffer, 
		      double* &output_buffer, 
		      int* permutation_map,
		      int dimension,
		      int* block_widths);

//transpose sets the offsets and calls recursive transpose which 
//performs the actual transpose recursively
EXTERN void recursive_transpose(double* &input_buffer, 
				double* &output_buffer, 
				int* &initial_offsets, 
				int* &permuted_offsets, 
				int* &data_index,
				int* &permutation_map,
				int recursion_level,
				int dims,
				int* block_widths,
				int num_last_matching_p_map);


					  
#endif
