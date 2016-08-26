
#ifndef _BLOCK_GRID_H_
#define _BLOCK_GRID_H_
#include<iostream>
#include<cstring>
#include<list>
#include<map>
#include "mpi.h"
#include "helper.h"

namespace RRR{
using namespace std;

class Block_Grid
{
    private:
        // Multidimensional size of the actual block grid
        int* actual_grid;




        // Actual coordinates of this block in the allocated grid
        int* actual_addr;

        // Maps each dimension of the allocated block grid to one of the
        // dimensions of the exposed block grid
        int *gdmap;

        // Inverse grid dimension map
        list<int>* igdmap;


    public:
        // Number of dimensions in the exposed block grid
        int grid_dims;

        // Dimension sizes of the exposed block grid
        int* bgrid;

        // Rank of this block
        int rank;

        // Virtual address/coordinates of this block in the exposed proc grid
        int* block_addr;

        // Total size of the block grid
        int nblocks;

        // Constructor
        Block_Grid(int num_dims, int* block_grid);

        // Returns rank of a block given its multi-dimensional address
        int get_block_rank(int* &address);

        // Finds out ranks of n blocks given their multi-dimensional addresses
        void get_block_ranks(int n, int** &addresses, int* &ranks);

        // Computes a block's multi-dimensional address given its rank
        void get_block_addr(int rank, int* &address);

        // Computes n blocks' multi-dimensional addresses given their ranks
        void get_block_addrs(int n, int** &addresses, int* &ranks);

        // prints address of a block given its rank
        void print_block_addr(int r);

        // Returns the string of the block's multi-dimensional address, given its rank
        string get_block_addr_str(int r);
};
}
#endif
