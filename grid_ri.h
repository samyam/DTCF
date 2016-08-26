
#ifndef _GRID_H_
#define _GRID_H_
#include<iostream>
#include<cstring>
#include<list>
#include<map>
#include "mpi.h"
#include "helper.h"

#ifdef __bgq__
#include <mpix.h>
#endif

using namespace std;

class Grid
{
    private:
        // Multidimensional size of the actual processor grid
        int* actual_grid;

#ifdef __bgq__
        // Processor grid information on BGQ
        MPIX_Hardware_t hw;
#endif

        // BGQ Torus dimensionality
        int bgq_torus_dims;

        // Stores if each dimension has end processors linked to form a torus
        // 1 if link is present, 0 if no link
        int* is_torus;

        // Actual coordinates of this processor in the allocated grid
        int* actual_addr;

        // Maps each dimension of the allocated processor grid to one of the
        // dimensions of the exposed processor grid
        int *gdmap;

        // Inverse grid dimension map
        list<int>* igdmap;

    public:
        // Number of dimensions in the exposed processor grid
        int grid_dims;

        // Dimension sizes of the exposed processor grid
        int* pgrid;

        // Rank of this processor
        int rank;

        // Virtual address/coordinates of this processor in the exposed proc grid
        int* proc_addr;

        // Total size of the processor grid
        int nprocs;

        // Constructor
        Grid(int num_dims, int* phy_grid);

        // Create a mapping from allocated grid dimensions to the exposed grid dimensions
        void create_gdmap();

        // Compute processor address and rank in new grid
        void compute_exposed_addr(int* &exposed_address, int* &actual_address);

        // Compute processor address and rank in new grid
        void compute_actual_addr(int* &actual_address, int* &exposed_address);

        // Returns rank of a processor given its multi-dimensional address
        int get_proc_rank(int* &address);

        // Finds out ranks of n processors given their multi-dimensional addresses
        void get_proc_ranks(int n, int** &addresses, int* &ranks);

        // Computes a processor's multi-dimensional address given its rank
        void get_proc_addr(int rank, int* &address);

        // Computes n processors' multi-dimensional addresses given their ranks
        void get_proc_addrs(int n, int** &addresses, int* &ranks);

        // prints address of a processor given its rank
        void print_proc_addr(int r);

        // Returns the string of the processor's multi-dimensional address, given its rank
        string get_proc_addr_str(int r);
};
#endif
