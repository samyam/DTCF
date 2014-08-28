#ifndef _TENSOR_H_
#define _TENSOR_H_

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <math.h>
#include <assert.h>
#include <iostream>
#include <algorithm>
#include <map>
#include <climits>
#include "helper.h"
#include "grid.h"

#define SYM_GROUP_0 0
#define SYM_GROUP_1 1
#define NON_SYM 3
#define CONTRACTED 4

using namespace std;
class Tensor
{

    private:

	// Declare TensorRedistributor as a friend class
	friend class TensorRedistributor;
	friend class Contraction;
	friend class cost;
	friend class GridRedistribute;

	Grid* g;
	int rank;	        // MPI rank
	int* proc_addr;     // Processor's multi-dimensional address 

	// Dimension of the tensor and the physical grid
	int dims, grid_dims;

	// Tensor string
	std::string tensor_str;

	// Tensor Size
	int* tensor_size;

	//stores the phase of each dimension of the tensor virtual blocks
	int* index_phase;

	//stores the range of local indices of each dimension
	int* local_range;

	//multi-dimensional size of block
	int* block_range;

	//stores the range of virtual indices of each dimension
	int* tensor_range;

	//holds the actual tensor data. 
	//stored as contiguous array of virtual blocks
	double* tensor_tiles;

	//stores the virtual index of each block stored by the class
	//example tile_address[d][k] gives the virtual index of
	//the d dimension for the k th block stored in tensor_tiles
	int* tile_address;

	//variable for correctness checking to be removed after debugging is over
	int* is_touched;
	
	//maximum blocks possible
	int num_max_tiles;

	//total blocks stored
	int num_actual_tiles;

	//maps a given dimension and local index to a list 
	//containing all the tiles with that dimension and local index
	//example for a 4d tensor index_table[1][2] 
	//gives a list with tiles whose indices are
	//[*,2*phase[1],*,*]
	std::list<int> **index_table;

	// Physical and Virtual grid sizes
	int *pgrid, *vgrid;

	int numSym;		// Number of symmetry groups
	int numNonSym;      // Number of non-Symmetric dimensions
	int sym_dims[2];	// Number of dimensions in each symmetry
	int sym_ratio[2];     // Length of symmetric dimension of local tensor in each symmetry


	// 1 if an index is contraction index 0 otherwise
	int *cntr_map; 

	// all the indices and belonging to the same symmetry group have the same value
	int *SG_index_map;

	// all the indices and belonging to the same symmetry group have the same value
	int *SG_index_map_permanent;

	// maps tensor indices to physical qrid
	int *index_dimension_map;

	//maps block address to the location of that block in the tensor
	std::map<int,int> block_addr_to_blk_num;

	// Contraction string set
	std::string* contr_dim_str;

	//initializes the index table which is an array of array of lists
	//storing offsets required to access local blocks for a given index
	void init_index_table();

	// fills up the index table
	void fill_index_table();

	// Return a value based on the index passed
	double default_get_value(int* &indices);

	// Find the max index in current dimension based on the symmetry it is involved in and current  indices of dimensions involved in the symmetry
	int get_bound(int cur_dim, int *cur_indices, int* &idmap, int* &phy_grid);

	// Generate data in tensor blocks
	void generate_data(int cur_dim, int* &cur_indices, int &offset);

	// Get number of tiles for a tensor at a particular rank and generate tile addresses for that
	int get_num_tiles(int cur_dim, int* &cur_indices, int &offset, int* &idmap, int* &proc_address, int* &addresses, int* &phy_grid, int &num_tiles); 

	// Fill data in the newly created sub-tensor
	void fill_data(double* &blocks, int* &addresses);

	//stores virtual address of the data block given a local data block
	void getVirtualAddress(int* &cur_indices, int* &virtual_address, int* &processor_addr, int* &idmap, int* &phy_grid);

	//returns true if the data block is unique given the symmetry
	bool isValid(int* &cur_indicies, int* &processor_addr, int* &idmap, int* &phy_grid);

	// Assert if the idmap is valid while creating the tensor
	void assert_idmap_validity();

	void redistribute_broadcast(int* &new_idx_map,
				    int bcast_proc_count,
				    int* bcast_dims_sizes,
				    int* &old_repl_dims, 
				    int* &new_repl_dims,
				    int old_repl_dims_count, 
				    int new_repl_dims_count, 
				    int* &bcast_dims, 
				    int bcast_dims_count,
				    int* common_repl_dims,
				    int common_repl_dims_count,
				    int* non_common_repl_dims,
				    int non_common_repl_dims_count);



	void redistribute_point_to_point(int* &new_idx_map,
						 int bcast_proc_count,
						 int* bcast_dims_sizes,
						 int* &old_repl_dims, 
						 int* &new_repl_dims,
						 int old_repl_dims_count, 
						 int new_repl_dims_count, 
						 int* &bcast_dims, 
						 int bcast_dims_count,
						 int* common_repl_dims,
						 int common_repl_dims_count,
						 int* non_common_repl_dims,
						 int non_common_repl_dims_count);

	map<int, int> compile_senders_data(map<int, list<int> > proc_block_map,
					   double** &send_blocks,
					   int** &send_addr,
					   int* &num_send_blocks,
					   int* &old_repl_dims,
					   int old_repl_dims_count);
    
    public:

	//number of elements per block
	int block_size;

	// Constructor
	Tensor(std::string tnsr_str, int* &idx_map, int* &tnsr_size, int* &virt_grid, Grid* &grid);

	// Destructor
	~Tensor();

	
	// Compute the number of maximum possible tiles in the tensor at this processor
	int compute_num_max_tiles(int* &idmap, int* &phy_grid);

	//sets function to the get_value function
	void set_get_value(double (*value_function)(int* &indices));

	// Initializes the tensor (fills in values)
	void initialize();

	// Returns the number of tiles with address value indx_id in dimension dim
	// Stores the tiles in tile_block and addresses in virt_addr
	int getTiles(int dim, int indx_id, double* &tile_block, int* &virt_addr);

	int getTile(int* tile_address);
	
	void printTiles(int dim, int indx_id);


	// Marks the index as already contracted
	void removecntrIndex(int indx);

	// gives a list of indices that need to be bounced
	// it is used by get bouncer to receive addresses of 
	//the nodes that will send bouncing data
	std::list<int>* get_bounce_indx(int indx);

	//outputs symmetry group and contraction index information
	//contraction index is 1 if it is part of some symmetry group
	//if it is not then it is either 1 or 2. The numbers are used
	//to identify the contraction index. In case of symmetry,
	//it is not important to distinguish them
	void printInfo();

	// Print all tile addresses 
	void print_all_tile_addr();

	// Extract a sub-tensor from the parent tensor w.r.t given contracting dimension
	Tensor* generate_tensor(int contr_dim, double* &data_blocks, int* &block_addresses, int num_tiles);


	//Generate a new tensor from a parent tensor with different data block and addresses
	Tensor* generate_tensor_with_new_tiles(double* &data_blocks,
					       int* &block_addresses,
					       int num_tiles);

        // Generate a deserialized subtensor
	Tensor* generate_deserialized_tensor(int num_des_dims, int* &des_tensor_dims, int* &des_grid_dims, double* &data_blocks, int* &block_addresses, int num_tiles);

        // Generates a value for the given address
	double get_value(int* &indices);

	// Free the memory allocated for index table
	void free_index_table();

	//returns number of processor addresses from which data needs to be 
	//bounced.THe array of addresses are put in bouncers. my_address holds
	//the invoking nodes address, index_dimension_map gives the mapping 
	//from index to processor dimension
	int get_bouncers(int index, int** &bouncers);

	// Get the number of blocks held by a processor
	int get_tensor_size(int cur_dim, int* &cur_indices, int* &processor_addr);

	// Gets receiver addresses based on current contraction dimension and index
	int get_receivers(int contr_dim, int contr_idx, int** &receivers, int* &matching_indices);

	//returns block number using block address
	//int get_block_number(int* &block_address);

	//returns the pointer to the data block specified by block_address;
	int get_tile_offset(int* &block_address);

	// Check if the given tensor has the same tile addresses as this tensor
	// in this processor
	bool check_same_tiles(Tensor* &T);

	// Check if the given tile exists in this tensor and return its index
	int tile_exists(int* &search_addr);

	//set the boolean for this address to true
	//only for debugging. remove later
	//asserts if address not found
	int touch_address(int* &search_addr);

	//sets boolean for all address to false
	void untouch_all_address();
	
	//checks if all addresses have been touched
	int is_all_touched();

	// Compare two tile addresses
	bool compare_addresses(int* &addr_1, int* &addr_2);

	// Compare two tile addresses
	bool compare_addresses(int dims, int* &addr_1, int* &addr_2);

	// Update physical grid
	void update_pgrid(int d, int* phy_grid);

	// Get a list of replicated dimensions in the given index dimension map
	int get_replicated_dims(int* &idmap, int* &repl_dims);

	// Redistribute this tensor as per the provided new index dimension map
	void redistribute(int* &new_idx_map);

	// Create generic proc address for broadcast receivers from the sender for redistribution
	map<int, list<int> > get_generic_proc_addresses(
		int* &tile_addresses, 
		int num_tiles, 
		int* &new_idx_map,
		int non_common_repl_dims_count, 
		int* &non_common_repl_dims);


	// Find senders for all the blocks this processor will hold after redistribution
	// and return a map of sender rank with a list of block offsets that will be received from it
	map<int, list<int> > get_recv_proc_block_map(
		int* &new_idx_map,
		int old_repl_dims_count,
		int* &old_repl_dims,
		int &num_new_tiles);

	// Create broadcast group for redistribution from the sender
	void get_bcast_groups(
		map<int, list<int> > proc_block_map, 
		map<int, list<int> > recv_proc_block_map, 
		int old_repl_dims_count,
		int* &old_repl_dims,
		int bcast_dims_count,
		int* &bcast_dims,
		MPI_Comm &recv_comm,
		MPI_Comm* &bcast_send_comm,
		MPI_Comm* &bcast_recv_comm);

// Post MPI_Intercomm_Create from senders
	void post_send_creates(
		map<int, list<int> > recv_leader_block_map,
		int bcast_dims_count,
		int* &bcast_dims,
		MPI_Comm &self_comm,
		MPI_Comm &recv_comm,
		MPI_Comm* &bcast_send_comm);

// Post sends for redistribute
	void post_broadcast_sends(
		map< int, list<int> > &proc_block_map,
		MPI_Comm* &bcast_send_comm,
		MPI_Comm &recv_intra_comm,
		int intra_comm_rank,
		double** &bcast_blocks,
		int** &bcast_addr,
		double* &bcast_recv_blocks,
		int* &bcast_recv_addr,
		int &offset);

	void copy_bcast_send_data(map< int, list<int> > proc_block_map, double** &bcast_blocks, int** &bcast_addr);

	// Check if the address satisfies tensor symmetry criterion
	bool satisfies_sym(int* &addr);


	// Getter functions

	int get_block_size()                    { return block_size;}
	int* get_block_range()                  { return block_range;}
	int  get_dims() 			{ return dims; }
	int* get_SG_index_map() 		{ return SG_index_map; }
	int* get_index_dimension_map() 	        { return index_dimension_map; }
	int* get_vgrid() 			{ return vgrid; }
	int get_num_actual_tiles()              { return num_actual_tiles;}
	void set_num_actual_tiles(int num_tiles){ num_actual_tiles = num_tiles; }
	string get_tensor_string()              { return tensor_str;}
	double* get_tensor_tiles()              { return tensor_tiles;}
	int* get_tile_addresses()               { return tile_address;}
	int get_rank()                          { return rank;}
};

#endif
