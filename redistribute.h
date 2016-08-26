#ifndef _TENSOR_REDIST_H_
#define _TENSOR_REDIST_H_

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <iostream>
#include "tensor.h"
#include "helper.h"
#include "grid.h"
namespace RRR{
class TensorRedistributor
{
    private:
	int size;
	int req_count; 
	MPI_Request* req_arr;

	// Tensor being redistributed
	Tensor* T;

	int dims, grid_dims;
	int* new_proc_addr;

	int serial;

	// Old and new index maps
	int* old_idx_map;
	int* new_idx_map;

	// Old and new physical grid sizes
	int* old_pgrid;
	int* new_pgrid;

    Grid* new_grid;
    Grid* old_grid;

	// Stores received data from one processor
	typedef struct 
	{
	    double* blocks;
	    int* block_addrs;
	    int num_blocks;
	} recv_data;

	// List of pointers to release after sends are complete
	list<double*> rlz_blks_ptrs;
	list<int*> rlz_addr_ptrs;

	// For packaging blocks that should reside locally (not to be sent to some other processor)
	recv_data local_data;

	// Finds out new virtual address of the block given the old address, old mapping and new mapping
	void get_new_virt_addr(int* &old_virt_addr, int* &new_virt_addr);

	// Extracts the blocks that should be sent to the processor with rank rank[index]
	// Returns number of blocks
	int get_blocks_for_proc(int index, int* &ranks, bool* &sent, double* &blocks, int* &block_addrs);

	// Finds number of blocks to be received from a processor
	int get_num_blocks_from_proc(int index, int* &ranks, int num_tiles);

	// Get number of blocks to be received from each rank
	// Return size of map, i.e. number of processors
	int get_num_recv_blocks(int* &ranks, int num_tiles, int* &map);

	// Find the processors that should hold the blocks at this processor and send the data to them
	void redistribute_send();

	// Find the processors that have the blocks that are supposed to be held at this processor
	// and post receives for them
	void redistribute_recv(std::list<recv_data> &recv_list);

	// Serialize a dimension
	// Performs an allgather along this dimension
	void serialize(int distr_tensor_dim, int distr_grid_dim);

	// Deserialize a dimension (distribute it)
	// Deletes unnecessary blocks along this dimension
	void deserialize(int distr_tensor_dim, int distr_grid_dim);

    public:

	// Constructor
	TensorRedistributor(Tensor* tensor, int* new_index_map, Grid* new_g);

    // Destructor 
	~TensorRedistributor();

	// Redistribute the tensor in the processor grid as per a new dimension mapping
	void redistribute();

};
}
#endif
