#include "grid.h"
#define RRANK 0
#define DEBUG_GRID 1
using namespace std;

// Constructor
Grid::Grid(int num_dims, int* phy_grid)
{
    grid_dims = num_dims;
    pgrid = new int[grid_dims+1];
    memcpy(pgrid, phy_grid, grid_dims * sizeof(int));

    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

#ifdef __bgq__
    MPIX_Hardware(&hw);

    bgq_torus_dims = hw.torus_dimension;
    actual_grid = new int[bgq_torus_dims];
    memcpy(actual_grid, hw.Size, bgq_torus_dims * sizeof(int));
    is_torus = new int[bgq_torus_dims]; 
    memcpy(is_torus, hw.isTorus, bgq_torus_dims * sizeof(int)); 
    actual_addr = new int[bgq_torus_dims];
    memcpy(actual_addr, hw.Coords, bgq_torus_dims * sizeof(int)); 

    create_gdmap();
    compute_exposed_addr(proc_addr, actual_addr);
#else
    get_proc_addr(rank, proc_addr);
#endif
}

#ifdef __bgq__
// Create a mapping from allocated grid dimensions to the exposed grid dimensions
void Grid::create_gdmap()
{
    gdmap = new int[bgq_torus_dims];
    igdmap = new list<int>[grid_dims];

    for(int i=0; i<bgq_torus_dims; i++)  gdmap[i] = -1;
    for(int i=0; i<grid_dims; i++) igdmap[i] = list<int>();

    int* temp = new int[grid_dims];
    for(int i=0; i<grid_dims; i++) temp[i] = 1;

    // Look for the dimensions having end processors connected with a link
    // and map them directly to the exposed grid dimensions, so that we
    // use these links
    for(int i=bgq_torus_dims-1; i>=0; i--)
    {
        if(is_torus[i] == 1)
        {
            for(int j=grid_dims-1; j>=0; j--)
            {
                if(actual_grid[i] == pgrid[j]  &&  temp[j] < pgrid[j])
                {
                    gdmap[i] = j;
                    igdmap[j].push_back(i);
                    temp[j] *= actual_grid[i];
                    break;
                }
            }
        }
    }
    /*if(rank == 0)
    {
        cout << "actual grid: "; print_tile_addr(bgq_torus_dims, actual_grid);
        cout << endl << "istorus: "; print_tile_addr(bgq_torus_dims, is_torus);
        cout << endl << "temp: "; print_tile_addr(grid_dims, temp);
        cout << endl << "GDMAP: "; print_tile_addr(bgq_torus_dims, gdmap);
        cout << endl;
    }*/

    // Now look for unsatisfied dimensions in the exposed grid and map
    // allocated grid dimensions to them
    for(int i=grid_dims-1; i>=0; i--)
    {
        if(pgrid[i] != temp[i])
        {
            for(int j=bgq_torus_dims-1; j>=0; j--)
            {
                if(gdmap[j] == -1 && pgrid[i] >= temp[i] * actual_grid[j])
                {
                    gdmap[j] = i;
                    igdmap[i].push_back(j);
                    temp[i] *= actual_grid[j];
                }
            }
        }
    }

    if(rank==RRANK && DEBUG_GRID)
    {
        cout << "actual grid: "; print_tile_addr(bgq_torus_dims, actual_grid);
        cout << endl << "istorus: "; print_tile_addr(bgq_torus_dims, is_torus);
        cout << endl << "temp: "; print_tile_addr(grid_dims, temp);
        cout << endl << "GDMAP: "; print_tile_addr(bgq_torus_dims, gdmap);
        cout << endl << "IGDMAP: ";
	cout << endl << "PGRID: " print_tile_addr(grid_dims,pgrid);
        for(int i=0; i<grid_dims; i++)
        {
            cout << " ";
            for(list<int>::iterator it = igdmap[i].begin(); it != igdmap[i].end(); it++)
                cout << *it << ",";
        }
        cout << endl;
    }

    // Assert that all dimensions are satisfied 
    for(int i=0; i<grid_dims; i++)
    {
        assert(pgrid[i] == temp[i]);
    }
}


// Compute processor address in exposed grid given its actual address
void Grid::compute_exposed_addr(int* &exposed_address, int* &actual_address)
{
    exposed_address = new int[grid_dims];
    for(int i=0; i<grid_dims; i++)
    {
        // If the exposed grid dimension is mapped to only one actual grid dimension,
        // just pick the index in the actual address
        if(igdmap[i].size() == 1)
        {
            exposed_address[i] = actual_address[igdmap[i].front()];
        }
        // Otherwise multiple dimensions of actual grid correspond to one of exposed.
        // Form an exposed index based on all the indices of mapped actual grid address dimensions
        else
        {
            int multiplier = 1;
            exposed_address[i] = 0;
            for(list<int>::iterator it = igdmap[i].begin(); it != igdmap[i].end(); it++)
            {
                exposed_address[i] += actual_address[*it] * multiplier;
                // If there is an element in the list after this one
                // and the next one is odd, then we have to reverse the order of enumeration
                if(distance(it,igdmap[i].begin()) < igdmap[i].size()-1  && 
                        actual_address[*(next_it(it,1))] % 2 == 1) 
                {
                    exposed_address[i] = exposed_address[i] - 2*actual_address[*it] + actual_grid[*it] - 1;
                }
                multiplier *= actual_grid[*it];
            }
        }
    }

    //cout << rank << " coords: "; print_tile_addr(bgq_torus_dims, actual_address);
    //cout << "  new coords: "; print_tile_addr(grid_dims, exposed_address); cout <<endl;
}

list<int>::iterator Grid::next_it(list<int>::iterator it, int n)
{
    list<int>::iterator iter = it;
    for(int i=0; i<n; i++)
        iter++;
    return iter;
}

// Compute processor address and rank in new grid
void Grid::compute_actual_addr(int* &actual_address, int* &exposed_address)
{
    actual_address = new int[bgq_torus_dims];
    bool* computed = new bool[bgq_torus_dims];
    for(int i=0; i<bgq_torus_dims; i++) computed[i] = false;

    for(int i=0; i<bgq_torus_dims; i++)
    {
        if(!computed[i])
        {
            // If the actual grid dimension is mapped to only one exposed grid dimension,
            // just pick the index in the exposed address
            if(igdmap[gdmap[i]].size() == 1)
            {
                actual_address[i] = exposed_address[gdmap[i]];
                computed[i] = true;
            }
            // Otherwise multiple dimensions of actual grid correspond to one of exposed.
            // Form the actual indices based on the index of mapped exposed grid address dimensions
            else
            {
                int k = exposed_address[gdmap[i]];
                int n = igdmap[gdmap[i]].size();

                // Extract the problem into smaller and easier to manipulate set of variables
                int* size = new int[n];
                int* addr = new int[n];
                int x = 1, j = 0;
                // Extract size of the problem sub-grid
                for(list<int>::iterator it = igdmap[gdmap[i]].begin(); it != igdmap[gdmap[i]].end(); it++)
                {
                    size[j] = actual_grid[*it];
                    x *= size[j];
                    j++;
                }
                // Compute address 
                for(int j=n-1; j>=0; j--)
                {
                    x /= size[j];
                    addr[j] = k/x;
                    k -= addr[j] * x;

                    if(j<n-1 && addr[j+1]%2 == 1)
                    {
                        addr[j] = size[j] - addr[j] - 1;
                    }
                }
                // Fill the address back in in the backward direction
                // (because while computing exposed address, we computed that way)
                j = 0;
                for(list<int>::iterator it = igdmap[gdmap[i]].begin(); it != igdmap[gdmap[i]].end(); it++)
                {
                    actual_address[*it] = addr[j++];
                    computed[*it] = true;
                }
            }
        }
    }
    //cout << rank << " actual address: "; print_tile_addr(bgq_torus_dims, actual_addr);
    //cout << "   Recomputed actual address: "; print_tile_addr(bgq_torus_dims, actual_address); cout<<endl; 
}
#endif


// Returns rank of a processor given its multi-dimensional address
int Grid::get_proc_rank(int* &address)
{
    int* addr = address;
    int dims = grid_dims;
    int* grid = pgrid;
#ifdef __bgq__
    compute_actual_addr(addr, address);
    //cout << " actual_addr: "; print_tile_addr(bgq_torus_dims, addr); cout << endl;
    dims = bgq_torus_dims;
    grid = actual_grid;
#endif
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


// Finds out ranks of n processors given their multi-dimensional addresses
void Grid::get_proc_ranks(int n, int** &addresses, int* &ranks)
{
    ranks = new int[n];
    for(int r=0; r<n; r++) 
    {
        ranks[r] = get_proc_rank(addresses[r]);
    }
}


// Computes a processor's multi-dimensional address given its rank
void Grid::get_proc_addr(int r, int* &address)
{
#ifndef __bgq__
    address = new int[grid_dims];
#endif
    int dims = grid_dims;
    int* addr = address;
    int* grid = pgrid;
#ifdef __bgq__
    dims = bgq_torus_dims;
    addr = new int[bgq_torus_dims];
    grid = actual_grid;
#endif
    int x = 1;
    for(int i=0; i<dims; i++)
        x *= grid[i]; 

    for(int i=0; i<dims; i++)
    {
        x /= grid[i];
        addr[i] = r/x;
        r -= addr[i] * x;
    }
#ifdef __bgq__
    compute_exposed_addr(address, addr);
#endif
}


// Computes n processors' multi-dimensional addresses given their ranks
void Grid::get_proc_addrs(int n, int** &addresses, int* &ranks)
{
    for(int r=0; r<n; r++) 
    {
        get_proc_addr(ranks[r], addresses[r]);
    }
}


// prints address of a processor given its rank
void Grid::print_proc_addr(int r)
{
    cout << "node address is : " << get_proc_addr_str(r) << endl;
}


// Returns the string of the processor's multi-dimensional address, given its rank
string Grid::get_proc_addr_str(int r)
{
    stringstream proc_addr_str;
    int* address;
    get_proc_addr(r, address);
    
    proc_addr_str << "[ ";
    for(int i=0; i<grid_dims; i++)
    {
        proc_addr_str << address[i];
        if(i < grid_dims-1) 
            proc_addr_str << ", ";
    }
    proc_addr_str<<"]";
    return proc_addr_str.str();
}


