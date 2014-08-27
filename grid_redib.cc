#include "grid_redib.h"
#include "tensor.h"
using namespace std;





//-------------------------------------------------------------------------------------------//


// Constructor
GridRedib::GridRedib(Tensor* tensor, int* new_index_map, Grid* new_g)
{
	T = tensor;
	new_grid = new_g;
	old_grid = T->g;

	dims = T->dims;
	grid_dims = new_grid->grid_dims;
	serial = grid_dims;
	old_serial=old_grid->grid_dims;

	old_idx_map = T->index_dimension_map;
	new_idx_map = new_index_map;

	old_pgrid = old_grid->pgrid;
	new_pgrid = new_grid->pgrid;

	// Recompute new proc_addr
	new_proc_addr = new int[dims];
	new_grid->get_proc_addr(T->rank, new_proc_addr);

	req_count = 0;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	req_arr = new MPI_Request[size * 2];

	// Assert that the specified new index map is correct for the configuration
	// If two dimensions are symmetric, ensure that the new physical grid has the same 
	// number of processors along those dimensions that these two tensor dimensions
	// will be distributed along
	for(int i=0; i<dims; i++)
	{
		for(int j=i+1; j<dims; j++)
		{
			if(T->SG_index_map[i] == T->SG_index_map[j]  && T->SG_index_map[j] < 2 && 
					new_index_map[i] != serial  &&  new_index_map[j] != serial)
			{
				assert(new_grid->pgrid[new_index_map[j]] == new_grid->pgrid[new_idx_map[i]]);
				//cout<< new_phy_grid[new_index_map[j]] << "  ==  " << new_phy_grid[new_idx_map[i]] << endl;
			}
		}
	}
}

GridRedib::~GridRedib()
{
	delete[] new_proc_addr;
	delete[] req_arr;
}


// Redistribute the tensor in the processor grid as per a new dimension mapping
void GridRedib::redistribute()
{
	
	// Re-compute num_max_tiles for the redistributed tensor
	T->num_max_tiles = T->compute_num_max_tiles(new_idx_map, new_pgrid);

	// Take care of serial dimensions here
	for(int i=0; i < T->dims; i++)
	{
		// If this dimension was serialized in the old map and distributed in the new one,
		// just delete the unnecessary block from the processors.
		if(old_idx_map[i] == old_serial && new_idx_map[i] != serial)
		{
		    int distr_grid_dim = new_idx_map[i];
		    deserialize(i, distr_grid_dim);
		}

		// If this dimension was distributed in the old map and serialized in the new one,
		// do an all-to-all broadcast to serialize it
		if(old_idx_map[i] != old_serial && new_idx_map[i] == serial)
		{	
			cout<<"Calling serialize with "<<i<<"  and   "<<old_idx_map[i]<<endl;
			int ser_grid_dim = old_idx_map[i];
			serialize(i, ser_grid_dim);
			//return;
			
		}
	}
	
	cout<<"aaaa";
	
	// Post send to the processor that is supposed to hold the blocks currently at this processor
	redistribute_send();

	// Post receive from the processor that currently holds the blocks that should be at this processor
	list<recv_data> recv_list;
	redistribute_recv(recv_list);

	// Check for completion of Isends
	MPI_Status* st_arr = new MPI_Status[req_count];
	MPI_Waitall(req_count, req_arr, st_arr);
	delete[] st_arr;
return;
	// After Isends are complete, delete send buffers
	for(list<double*>::iterator blks = rlz_blks_ptrs.begin();
			blks != rlz_blks_ptrs.end(); blks++)
	{
		delete[] *blks;
	}
	for(list<int*>::iterator addr = rlz_addr_ptrs.begin();
			addr != rlz_addr_ptrs.end(); addr++)
	{
		delete[] *addr;
	}

	//cout << T->rank << " Finished redistribute communication" << endl << fflush;

	// Reallocate memory for blocks and addresses after redistribution
	int block_count = 0;
	for(list<recv_data>::iterator it = recv_list.begin(); it != recv_list.end(); it++)
	{
		recv_data* rd = &(*it);
		block_count += rd->num_blocks;
	}
	T->num_actual_tiles = block_count;
	delete[] T->tensor_tiles;
	delete[] T->tile_address;
	T->tensor_tiles = new double[T->block_size * block_count];
	T->tile_address = new int[T->dims * block_count];

	// Once all sends and receives are complete for this processor,
	// Copy the received blocks and addresses in tensor_tiles and tile_addresses.
	// This copying is needed because blocks are received from different processors are
	// not in contiguous memory.
	double* tiles = T->tensor_tiles;
	int* tile_addrs = T->tile_address;
	int tile_offset = 0, tile_addrs_offset = 0;
	for(list<recv_data>::iterator it = recv_list.begin(); it != recv_list.end(); it++)
	{
		recv_data* rd = &(*it);

		memcpy(tiles, rd->blocks, rd->num_blocks * T->block_size * sizeof(double));
		tiles += rd->num_blocks * T->block_size;
		delete[] rd->blocks;

		memcpy(tile_addrs, rd->block_addrs, rd->num_blocks * dims * sizeof(int));
		tile_addrs += rd->num_blocks * dims;
		delete[] rd->block_addrs;
	}

	// Update proc address in tensor object
	memcpy(T->proc_addr, new_proc_addr, dims * sizeof(int));

	// Free old index table
	T->free_index_table();

	// Update idmap
	memcpy(T->index_dimension_map, new_idx_map, dims*sizeof(int));

	// Recompute index_table
	T->init_index_table();
	T->fill_index_table();
}


// Extracts the blocks that should be sent to the processor with rank rank[index]
// Returns number of blocks
int GridRedib::get_blocks_for_proc(int index, int* &ranks, bool* &sent, double* &blocks, int* &block_addrs)
{
	double* t_blocks = T->tensor_tiles;
	int* t_block_addrs = T->tile_address;
	int count = 0;

	// Count the number of blocks to copy
	for (int i=0; i < T->num_actual_tiles; i++)
	{
		if(ranks[i] == ranks[index])
		{
			count++;
		}
	}

	// Allocate memory for blocks and addresses
	blocks = new double[count * T->block_size];
	block_addrs = new int[count * dims];

	// Copy the required blocks in the allocated memory
	double* out_blocks = blocks;
	int* out_block_addrs = block_addrs;
	for (int i=0, c=0; i < T->num_actual_tiles; i++)
	{
		if(ranks[i] == ranks[index]) // if correct rank, copy block
		{
			// Copy block
			memcpy(out_blocks, t_blocks, T->block_size * sizeof(double));
			out_blocks += T->block_size;

			// Copy address
			memcpy(out_block_addrs, t_block_addrs, dims * sizeof(int));
			out_block_addrs += dims;

			sent[i] = true;
			c++;
		}
		t_blocks += T->block_size;
		t_block_addrs += dims;
	}

	return count;
}


// Finds number of blocks to be received from a processor
int GridRedib::get_num_blocks_from_proc(int index, int* &ranks, int num_tiles)
{
	int count = 0;

	// Count the number of blocks to copy
	for (int i=0; i < num_tiles; i++)
	{
		if(ranks[i] == ranks[index])
		{
			count++;
		}
	}
	return count;
}


// Get number of blocks to be received from each rank
// Return size of map, i.e. number of processors
int GridRedib::get_num_recv_blocks(int* &ranks, int num_tiles, int* &map)
{
	// Find total number of processors
	int num_procs = 1;
	for(int i=0; i<dims; i++)
	{
		num_procs *= old_pgrid[i];
	}

	// Declare and initialize map of size of number of processors
	map = new int[num_procs];
	for(int i=0; i<num_procs; i++)
	{
		map[i] = 0;
	}

	// Fill in map
	for(int i=0; i<num_tiles; i++)
	{
		map[ranks[i]]++;
	} 

	return num_procs;
}


// Find the processors that should hold the blocks at this processor and send the data to them
void GridRedib::redistribute_send()
{
	req_count = 0;
	// Allocate memory for new processor address for each block
	int* new_proc_addr = new int[T->num_actual_tiles * dims];
	int* new_proc_rank = new int[T->num_actual_tiles];

	// Create sent flags for each block and initialize them to false
	bool* sent = new bool[T->num_actual_tiles];
	for (int i=0; i < T->num_actual_tiles; i++)
	{
		sent[i] = false;
	}
cout<<T->num_actual_tiles<<endl;
	// Compute new processor address for each block
	for (int i=0; i < T->num_actual_tiles; i++)
	{
		int offset = i * dims;
		int* new_proc_address = new_proc_addr + offset;
		for(int j=0; j<dims; j++)
		{
			new_proc_address[new_idx_map[j]] = (T->tile_address + offset)[j] % new_pgrid[new_idx_map[j]];
		}
		new_proc_rank[i] = new_grid->get_proc_rank(new_proc_address);
	}

	// Post sends to each processor identified above
	//list<MPI_Request> req_list = list<MPI_Request>();
	for (int i=0; i < T->num_actual_tiles; i++)
	{
		if(sent[i] == false)
		{
			double* blocks;
			int* block_addrs;
			int num_blocks = get_blocks_for_proc(i, new_proc_rank, sent, blocks, block_addrs);

			if(num_blocks > 0)
			{
				// receiver is the same processor
				if(T->rank == new_proc_rank[i])
				{
					local_data.blocks = blocks;
					local_data.block_addrs = block_addrs;
					local_data.num_blocks = num_blocks;
				}
				// receiver is a different processor
				else
				{
					/*if(T->rank == 3)
					  {
					  cout << get_proc_addr_str(dims, old_pgrid, T->rank) << " send to   " << get_proc_addr_str(dims, new_pgrid, new_proc_rank[i]) << "  #blocks = " << num_blocks << endl;
					  cout << "Addresses sent: ";
					  for(int j=0; j<num_blocks; j++)
					  {
					  int* addr = block_addrs + j*dims;
					  print_tile_addr(dims, addr); 
					  }
					  cout << endl << endl;
					  }*/

					MPI_Request req1, req2;
					MPI_Isend(blocks, num_blocks * T->block_size, MPI_DOUBLE, new_proc_rank[i], 3, MPI_COMM_WORLD, &req1);  
					MPI_Isend(block_addrs, num_blocks * dims, MPI_INT, new_proc_rank[i], 4, MPI_COMM_WORLD, &req2);  
					rlz_blks_ptrs.push_back(blocks);
					rlz_addr_ptrs.push_back(block_addrs);
					req_arr[req_count++] = req1;
					req_arr[req_count++] = req2;
				}
			}
		}
	}

	delete[] new_proc_addr;
	delete[] new_proc_rank;
}


// Find the processors that have the blocks that are supposed to be held at this processor
// and post receives for them
void GridRedib::redistribute_recv(list<recv_data> &recv_list)
{
	int* local_indices = new int[dims];
	int offset = 0;
	int* addresses = new int[T->num_max_tiles * dims];

	// Generate addresses that should reside at this processor w.r.t. the new index map
	int num_tiles = 0;
	T->get_num_tiles(0, local_indices, offset, new_idx_map, new_proc_addr, addresses, new_pgrid, num_tiles);

	// Allocate memory for processor addresses for each block
	int* old_proc_addr = new int[num_tiles * dims];
	int* old_proc_ranks = new int[num_tiles];

	// Compute old processor address for each block
	for (int i=0; i < num_tiles; i++)
	{ 
		int offset = i * dims;
		int* old_proc_address = old_proc_addr + offset;
		for(int j=0; j<dims; j++)
		{
			old_proc_address[old_idx_map[j]] = (addresses + offset)[j] % old_pgrid[old_idx_map[j]];
		}
		old_proc_ranks[i] = old_grid->get_proc_rank(old_proc_address);
	}

	// Find how many blocks will be received from which processor
	int* map;
	int num_procs = get_num_recv_blocks(old_proc_ranks, num_tiles, map);

	// Post receives from each processor identified above
	recv_list = list<recv_data>();

	for (int i=0; i < num_procs; i++)
	{
		int num_blocks = map[i];
		if(num_blocks > 0)
		{
			// Sender is the same processor
			if(T->rank == i)
			{
				recv_list.push_back(local_data);
			}
			// Sender is a different processor
			else
			{
				recv_data rd;
				rd.num_blocks = num_blocks;
				rd.blocks = new double[num_blocks * T->block_size];
				rd.block_addrs = new int[num_blocks * dims];

				//MPI_Request req1, req2;
				MPI_Status st1, st2;
				MPI_Recv(rd.blocks, num_blocks * T->block_size, MPI_DOUBLE, i, 3, MPI_COMM_WORLD, &st1); // &req1);  
				MPI_Recv(rd.block_addrs, num_blocks * dims, MPI_INT, i, 4, MPI_COMM_WORLD, &st2); // &req2); 
				recv_list.push_back(rd);  
			}
		}
	}

	// Free allocated memory
	delete[] map;
	delete[] local_indices;
	delete[] addresses;
	delete[] old_proc_addr;
	delete[] old_proc_ranks;
}


// Serialize a dimension
// Performs an allgather along this dimension
void GridRedib::serialize(int ser_tensor_dim, int ser_grid_dim)
{
	if(T->rank==0) cout << " Serializing tensor_dim = " << ser_tensor_dim << " grid_dim = " << ser_grid_dim << endl;

	// Create a new MPI_Communicator in the serialization dimension
	int num_procs = old_pgrid[ser_grid_dim];
	
	int* dim_group_ranks = new int[num_procs]; 
	int* processor_addr = new int[old_serial];
	memcpy(processor_addr, T->proc_addr, old_serial * sizeof(int));
	
	for(int i=0; i < num_procs; i++)
	{
		processor_addr[ser_grid_dim] = i;
		dim_group_ranks[i] = old_grid->get_proc_rank(processor_addr);
	}
	
	MPI::Group world_group = MPI::COMM_WORLD.Get_group();
	MPI::Group dim_group = world_group.Incl(num_procs, dim_group_ranks);
	MPI::Intracomm dim_comm = MPI::COMM_WORLD.Create(dim_group);
	
	int dim_rank = dim_comm.Get_rank();
	int dim_size = dim_comm.Get_size();
	assert(dim_rank == T->proc_addr[ser_grid_dim]);
	assert(dim_size == num_procs);

	// Find out the number of tiles this processor should contain after serializing
	int num_serialized_tiles = 0;
	 
	int* num_gather_tiles = new int[num_procs];
	
	MPI_Allgather(&T->num_actual_tiles, 1, MPI_INT, num_gather_tiles, 1, MPI_INT, dim_comm);
	
	for(int i=0; i < num_procs; i++)
	{
		num_serialized_tiles += num_gather_tiles[i];
	}

	// Allocate memory for gathered data
	
	double* blocks = new double[T->block_size * num_serialized_tiles];
	int* addresses = new int[dims * num_serialized_tiles];

	
	 
	// Compute displacements and element counts in recv buffer
	int* block_displacements = new int[num_procs];
	int* address_displacements = new int[num_procs];
	int* block_counts = new int[num_procs];
	int* addr_counts = new int[num_procs];
	int block_num_offset = 0;
	for(int i=0; i < num_procs; i++)
	{
		block_displacements[i] = block_num_offset * T->block_size;
		address_displacements[i] = block_num_offset * dims;
		block_num_offset += num_gather_tiles[i];

		block_counts[i] = num_gather_tiles[i] * T->block_size;
		addr_counts[i] = num_gather_tiles[i] * dims;
	}

	// Gather addresses and blocks in this dimension
	MPI_Allgatherv( T->tile_address, T->num_actual_tiles * dims, MPI_INT,
			addresses, addr_counts, address_displacements, MPI_INT, dim_comm);
	MPI_Allgatherv( T->tensor_tiles, T->num_actual_tiles * T->block_size, MPI_DOUBLE,
			blocks, block_counts, block_displacements, MPI_DOUBLE, dim_comm);

	// Free old blocks and addresses memory
	delete[] T->tensor_tiles;
	delete[] T->tile_address;

	// Update tensor member variables
	T->tensor_tiles = blocks;
	T->tile_address = addresses;
	T->num_actual_tiles = num_serialized_tiles;

	// Free old index table
	T->free_index_table();

	// Update idmap
	T->index_dimension_map[ser_tensor_dim] = grid_dims;

	// Create new index map
	T->init_index_table();
	T->fill_index_table();

	delete[] dim_group_ranks;
	delete[] processor_addr;
}


// Deserialize a dimension (distribute it)
// Deletes unnecessary blocks along this dimension
void GridRedib::deserialize(int distr_tensor_dim, int distr_grid_dim)
{
	// Find number of blocks to retain and mark them for retention
	bool* retain = new bool[T->num_actual_tiles];
	for(int i=0; i < T->num_actual_tiles; i++) retain[i] = false;
	int retain_count = 0;
	for(int i=0; i < T->num_actual_tiles; i++)
	{
		int* addr = T->tile_address + i*dims;
		// Check if this address is meant to be retained
		if(addr[distr_tensor_dim] % new_pgrid[distr_grid_dim] == new_proc_addr[distr_grid_dim])
		{
			retain[i] = true;
			retain_count++;
		}
	}

	// Copy blocks to retain to a new buffer
	double* blocks = new double[T->block_size * retain_count];
	int* addresses = new int[dims * retain_count];
	int block_offset = 0, addr_offset = 0;
	for(int i=0; i < T->num_actual_tiles; i++)
	{
		if(retain[i] == true)
		{
			memcpy(blocks + block_offset, T->tensor_tiles + i * T->block_size, T->block_size * sizeof(double));
			block_offset += T->block_size;

			memcpy(addresses + addr_offset, T->tile_address + i*dims, dims * sizeof(int));
			addr_offset += dims;
		}
	}

	// Free old memory
	delete[] T->tensor_tiles;
	delete[] T->tile_address;
	delete[] retain;

	// Update tensor member variables
	T->tensor_tiles = blocks;
	T->tile_address = addresses;
	T->num_actual_tiles = retain_count;

	// Free old index table
	T->free_index_table();

	// Update idmap
	T->index_dimension_map[distr_tensor_dim] = distr_grid_dim;

	// Create new index map
	T->init_index_table();
	T->fill_index_table();
}


