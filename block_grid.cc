#include "block_grid.h"
namespace RRR{
using namespace std;

// Constructor
Block_Grid::Block_Grid(int num_dims, int* block_grid)
{
    grid_dims = num_dims;
    bgrid = new int[grid_dims+1];
    memcpy(bgrid, block_grid, grid_dims * sizeof(int));

    MPI_Comm_size(MPI_COMM_WORLD,&nblocks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    get_block_addr(rank, block_addr);

}



// Returns rank of a block given its multi-dimensional address
int Block_Grid::get_block_rank(int* &address)
{
    int* addr = address;
    int dims = grid_dims;
    int* grid = bgrid;
    int x = 1;
    int r = 0;
    for(int i=dims-1; i>=0; i--) 
    {
        r += addr[i] * x;
        x *= grid[i];
    }
    //cout << "rr = " << r << endl;
    return r;
}


// Finds out ranks of n blocks given their multi-dimensional addresses
void Block_Grid::get_block_ranks(int n, int** &addresses, int* &ranks)
{
    ranks = new int[n];
    for(int r=0; r<n; r++) 
    {
        ranks[r] = get_block_rank(addresses[r]);
    }
}


// Computes a block's multi-dimensional address given its rank
void Block_Grid::get_block_addr(int r, int* &address)
{
    address = new int[grid_dims];
    int dims = grid_dims;
    int* addr = address;
    int* grid = bgrid;
    int x = 1;
    for(int i=0; i<dims; i++)
        x *= grid[i]; 

    for(int i=0; i<dims; i++)
    {
        x /= grid[i];
        addr[i] = r/x;
        r -= addr[i] * x;
    }
}


// Computes n blocks' multi-dimensional addresses given their ranks
void Block_Grid::get_block_addrs(int n, int** &addresses, int* &ranks)
{
    for(int r=0; r<n; r++) 
    {
        get_block_addr(ranks[r], addresses[r]);
    }
}


// prints address of a block given its rank
void Block_Grid::print_block_addr(int r)
{
    cout << "node address is : " << get_block_addr_str(r) << endl;
}


// Returns the string of the block's multi-dimensional address, given its rank
string Block_Grid::get_block_addr_str(int r)
{
    stringstream block_addr_str;
    int* address;
    get_block_addr(r, address);
    
    block_addr_str << "[ ";
    for(int i=0; i<grid_dims; i++)
    {
        block_addr_str << address[i];
        if(i < grid_dims-1) 
            block_addr_str << ", ";
    }
    block_addr_str<<"]";
    return block_addr_str.str();
}

}
