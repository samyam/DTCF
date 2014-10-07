#include "grid_redib.h"
#include "tensor.h"

#define DEBUG_CHECK_POINT 0
#define RRANK 14

#define DDbug 0

namespace RRR{
    using namespace std;


//-------------------------------------------------------------------------------------------//


// Constructor
    GridRedistribute::GridRedistribute(Tensor* tensor, int* new_index_map, Grid* new_g)
    {
        T = tensor;
        new_grid = new Grid(new_g->grid_dims,new_g->pgrid);
        old_grid = T->g;

        dims = T->dims;
        grid_dims = new_grid->grid_dims;
        serial = grid_dims;
        old_serial=old_grid->grid_dims;

        old_idx_map = T->index_dimension_map;
        new_idx_map = new_index_map;

        old_pgrid = old_grid->pgrid;
	old_pgrid[old_serial]=1;

        new_pgrid = new int[new_grid->grid_dims+1];
	for(int i=0;i<new_grid->grid_dims;i++)new_pgrid[i]=new_grid->pgrid[i];
	new_pgrid[new_grid->grid_dims]=1;

        // Recompute new proc_addr
        new_proc_addr = new int[grid_dims+1];
        new_grid->get_proc_addr(T->rank, new_proc_addr);
	new_proc_addr[grid_dims]=0;

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

    GridRedistribute::~GridRedistribute()
    {
        delete[] new_proc_addr;
        delete[] req_arr;
    }

int GridRedistribute::get_replicated_dims(int* &idmap, int* &repl_dims, int grid_dims)
{

	int* temp = new int[grid_dims];
	memset(temp, 0, grid_dims*sizeof(int));
	
	// Find all the replicated dimension numbers
	for(int i=0; i<dims; i++)
	{
	    temp[idmap[i]] = 1;
	}

	// Count the replicated dimensions
	int count = 0;
	for(int i=0; i<grid_dims; i++)
	{
	    if(temp[i] == 0)
		count++;
	}

	// Allocate memory and set dimension numbers in the array
	repl_dims = new int[count];
	int j = 0;
	for(int i=0; i<grid_dims; i++)
	{
	    if(temp[i] == 0)
	    {
			
		repl_dims[j] = i;
		j++;
	    }
	}

	delete[] temp;
	return count;
    }



// Redistribute the tensor in the processor grid as per a new dimension mapping
void GridRedistribute::redistribute()
{

    int rank = T->rank;
    int checkpoint = 0;
	//if(T->rank==14){cout<<"old grid size = "<<old_grid->grid_dims<<"    new grid size= "<<new_grid->grid_dims; print_tile_addr(old_grid->grid_dims,old_grid->pgrid); 
//print_tile_addr(new_grid->grid_dims,new_grid->pgrid);}

        // Re-compute num_max_tiles for the redistributed tensor
        T->num_max_tiles = T->compute_num_max_tiles(new_idx_map, new_pgrid);

        

	int* old_repl_dims;
	int* repl_dims;
	int repcount = get_replicated_dims(new_idx_map,repl_dims,grid_dims);
	int oldrepcount= get_replicated_dims(old_idx_map, old_repl_dims,old_grid->grid_dims);
	//cout<<repcount<<endl;
	if(DDbug == 1) cout<<"Rank1:"<<T->rank<<endl;	
        // Post send to the processor that is supposed to hold the blocks currently at this processor
        redistribute_send(old_repl_dims,oldrepcount);
	
	if(rank==RRANK && DEBUG_CHECK_POINT) cout << "Rank : "<<rank<<". Redistribute Checkpoint " <<(checkpoint++)<< endl; 
	
	
        // Post receive from the processor that currently holds the blocks that should be at this processor
        list<recv_data> recv_list;
        redistribute_recv(recv_list,repl_dims,repcount);
	
	//if(DDbug == 1) cout<<"Rank2:"<<T->rank<<endl;
         //Check for completion of Isends

	
        MPI_Status* st_arr = new MPI_Status[req_count];
        MPI_Waitall(req_count, req_arr, st_arr);
        delete[] st_arr;
	if(rank==RRANK && DEBUG_CHECK_POINT) cout << "Rank : "<<rank<<". Redistribute Checkpoint " <<(checkpoint++)<< endl; 

	
        // After Isends are complete, delete send buffers
        for(list<double*>::iterator blks = rlz_blks_ptrs.begin();
	    blks != rlz_blks_ptrs.end(); blks++)
        {
	    delete[] *blks;
        }
	if(rank==RRANK && DEBUG_CHECK_POINT) cout << "Rank : "<<rank<<". Redistribute Checkpoint " <<(checkpoint++)<< endl; 

        for(list<int*>::iterator addr = rlz_addr_ptrs.begin();
	    addr != rlz_addr_ptrs.end(); addr++)
        {
                delete[] *addr;
}
if(DDbug == 1) cout<<"Rank3:"<<T->rank<<endl;

        //cout << T->rank << " Finished redistribute communication" << endl << fflush;

        // Reallocate memory for blocks and addresses after redistribution
        int block_count = 0;
        for(list<recv_data>::iterator it = recv_list.begin(); it != recv_list.end(); it++)
        {
	    recv_data* rd = &(*it);
	    block_count += rd->num_blocks;
        }
	if(rank==RRANK && DEBUG_CHECK_POINT) cout << "Rank : "<<rank<<". Redistribute Checkpoint " <<(checkpoint++)<< endl; 

        T->num_actual_tiles = block_count;
        delete[] T->tensor_tiles;
        delete[] T->tile_address;
        T->tensor_tiles = new double[T->block_size * block_count];
        T->tile_address = new int[T->dims * block_count];
if(DDbug == 1) cout<<"Rank4:"<<T->rank<<endl;	

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
if(DDbug == 1) cout<<"Rank5:"<<T->rank<<endl;
        // Update proc address in tensor object
        memcpy(T->proc_addr, new_proc_addr, dims * sizeof(int));

	for(int i=0; i<repcount; i++)
{	
	replicate(repl_dims[i]);
}
	if(DDbug == 1) cout<<"Rank6:"<<T->rank<<endl;

        // Free old index table
        T->free_index_table();
	

        // Update idmap
        memcpy(T->index_dimension_map, new_idx_map, dims*sizeof(int));
	
	//updates pgrid
	T->update_pgrid(new_grid->grid_dims,new_grid->pgrid);

        // Recompute index_table
        T->init_index_table();
if(DDbug == 1) cout<<"Rank7:"<<T->rank<<endl;
        T->fill_index_table();
	
}


// Extracts the blocks that should be sent to the processor with rank rank[index]
// Returns number of blocks
    int GridRedistribute::get_blocks_for_proc(int index, int* &ranks, bool* &sent, double* &blocks, int* &block_addrs)
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
    int GridRedistribute::get_num_blocks_from_proc(int index, int* &ranks, int num_tiles)
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
    int GridRedistribute::get_num_recv_blocks(int* &ranks, int num_tiles, int* &map)
    {
        // Find total number of processors
        int num_procs = 1;
        for(int i=0; i<old_serial; i++)
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
void GridRedistribute::redistribute_send(int* &repl_dims, int repcount)
{

        req_count = 0;
        // Allocate memory for new processor address for each block
        int* new_proc_addr = new int[T->num_actual_tiles * (grid_dims+1)];
	memset(new_proc_addr,0,(T->num_actual_tiles*(grid_dims+1)*sizeof(int)));
        int* new_proc_rank = new int[T->num_actual_tiles];
	//if(T->rank==14) cout<<"T->num_actual_tiles"<<T->num_actual_tiles<<endl;
        // Create sent flags for each block and initialize them to false
        bool* sent = new bool[T->num_actual_tiles];
        for (int i=0; i < T->num_actual_tiles; i++)
        {
	    sent[i] = false;
        }

        // Compute new processor address for each block
        for (int i=0; i < T->num_actual_tiles; i++)
        {
	    int offset = i * dims;
	    int offset2 = i*(grid_dims+1);
	    int* new_proc_address = new_proc_addr + offset2;
	    for(int j=0; j<dims; j++)
	    {
		new_proc_address[new_idx_map[j]] = (T->tile_address + offset)[j] % new_pgrid[new_idx_map[j]];
	    }
	    new_proc_rank[i] = new_grid->get_proc_rank(new_proc_address);
        }
	
	bool flag=true;
	if(repcount>0)
	{	

	int* proc_add=new int[old_grid->grid_dims+1];
	old_grid->get_proc_addr(T->rank, proc_add);
	for(int i=0;i<repcount;i++)
	{
	if(proc_add[repl_dims[i]]!=0) flag=false;
	}
		//cout<<flag<<endl;
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
 			//cout<<"aaaaa"<<i<<endl;
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
                                else if(flag)
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
					//if(T->rank == 14 || new_proc_rank[i]==14) cout<<"Sending from "<<T->rank<<" to "<< new_proc_rank[i]<<endl;
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
// and post receives for them
    void GridRedistribute::redistribute_recv(list<recv_data> &recv_list, int* &repl_dims, int repcount)
    {
	
        int* local_indices = new int[dims];
        int offset = 0;
        int* addresses = new int[T->num_max_tiles * dims];
                
        // Generate addresses that should reside at this processor w.r.t. the new index map
        //int num_tiles = T->num_actual_tiles;
	int num_tiles = 0;
	
        T->get_num_tiles(0, local_indices, offset, new_idx_map, new_proc_addr, addresses, new_pgrid, num_tiles);
        
	//cout<<"New Num tiles = "<<num_tiles;
        // Allocate memory for processor addresses for each block
        //int* old_proc_addr = new int[num_tiles * old_serial];
        int* old_proc_ranks = new int[num_tiles];

        // Compute old processor address for each block
        for (int i=0; i < num_tiles; i++)
        {
	    int offset = i * dims;
	    int* old_proc_address = new int[old_serial];
	    memset(old_proc_address, 0, old_serial * sizeof(int));
	    for(int j=0; j<dims; j++)
	    {	
		old_proc_address[old_idx_map[j]] = (addresses + offset)[j] % old_pgrid[old_idx_map[j]];
	    }
	    old_proc_ranks[i] = old_grid->get_proc_rank(old_proc_address);
		
        }

       
	bool flag=true;



	if(repcount>0)
	{	

	    int* proc_add=new int[grid_dims+1];
	    new_grid->get_proc_addr(T->rank, proc_add);
	    for(int i=0;i<repcount;i++)
	    {
		if(proc_add[repl_dims[i]]!=0) flag=false;
	    }
	    //cout<<flag<<endl;
	}
        // Find how many blocks will be received from which processor
        int* map;
        int num_procs = get_num_recv_blocks(old_proc_ranks, num_tiles, map);

        // Post receives from each processor identified above
        recv_list = list<recv_data>();
	//cout<<"numtiles"<<num_tiles<<"  rank"<<T->rank<<endl;
               

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
		else if(flag)
		{	
				
                                recv_data rd;
                                rd.num_blocks = num_blocks;
                                rd.blocks = new double[num_blocks * T->block_size];
                                rd.block_addrs = new int[num_blocks * dims];

                                MPI_Request req1, req2;

                                MPI_Irecv(rd.blocks, num_blocks * T->block_size, MPI_DOUBLE, i, 3, MPI_COMM_WORLD, &req1);  
                                MPI_Irecv(rd.block_addrs, num_blocks * dims, MPI_INT, i, 4, MPI_COMM_WORLD, &req2); 
				//if(i == 14 || T->rank==14) cout<<"Recieving from "<<i <<" to "<<T->rank<<endl;
                                recv_list.push_back(rd);
				req_arr[req_count++] = req1;
                                req_arr[req_count++] = req2;

				
		}
	    }
        }


        // Free allocated memory
	//   delete[] map;
	// delete[] local_indices;
	// delete[] addresses;
	//  delete[] old_proc_addr;
	//  delete[] old_proc_ranks;
    }




    void GridRedistribute::replicate(int rep_dim)
    {
	int rank  = T->rank;
	int checkpoint = 0;

	if(rank==RRANK && DEBUG_CHECK_POINT) cout << "Rank : "<<rank<<". Replicate Redistribute Checkpoint " <<(checkpoint++)<< endl; 
	int* new_proc_add = new int[grid_dims+1];
        new_grid->get_proc_addr(T->rank, new_proc_add);
	
	if(rank==RRANK && DEBUG_CHECK_POINT) cout << "Rank : "<<rank<<". Replicate Redistribute Checkpoint " <<(checkpoint++)<< endl; 
	new_proc_add[grid_dims]=0;
if(DDbug == 1) cout<<"Replication dimension is"<<rep_dim;


        // Create a new MPI_Communicator in the serialization dimension
        int num_procs = new_pgrid[rep_dim];
	////cout<<"repdimcount"<<num_procs<<endl;
        int* dim_group_ranks = new int[num_procs];
       
	if(rank==RRANK && DEBUG_CHECK_POINT) cout << "Rank : "<<rank<<". Replicate Redistribute Checkpoint " <<(checkpoint++)<< endl; 
        for(int i=0; i < num_procs; i++)
        {
	    new_proc_add[rep_dim] = i;
	    dim_group_ranks[i] = new_grid->get_proc_rank(new_proc_add);
        }
	
	if(rank==RRANK && DEBUG_CHECK_POINT) cout << "Rank : "<<rank<<". Replicate Redistribute Checkpoint " <<(checkpoint++)<< endl; 
	////cout<<"Before group create"<< dim_group_ranks[0]<<"<<Leader---follower>>"<<dim_group_ranks[1]<<endl;
	
	MPI_Group orig_group, new_group; 
	MPI_Comm new_comm; 

	MPI_Comm_group(MPI_COMM_WORLD, &orig_group); 
	
	MPI_Group_incl(orig_group, num_procs, dim_group_ranks, &new_group);

	if(rank==RRANK && DEBUG_CHECK_POINT) cout << "Rank : "<<rank<<". Replicate Redistribute Checkpoint " <<(checkpoint++)<< endl; 	
	MPI_Comm_create(MPI_COMM_WORLD, new_group, &new_comm); 


	// MPI::Group world_group = MPI::COMM_WORLD.Get_group();
        //MPI::Group dim_group = world_group.Incl(num_procs, dim_group_ranks);
        //MPI::Intracomm dim_comm = MPI::COMM_WORLD.Create(dim_group);

	//   int dim_rank = dim_comm.Get_rank();
	//   int dim_size = dim_comm.Get_size();
        //assert(dim_rank == T->proc_addr[ser_grid_dim]);
	//  assert(dim_size == num_procs);
	////cout<<"Assertion done"<<endl;


	int count = T->num_actual_tiles;

        MPI_Bcast(&count, 1, MPI_INT,dim_group_ranks[0],new_comm);

	//  //cout<<count<<"at rank"<<T->rank<<endl;

	if(count>0){
	    if(T->num_actual_tiles==0)
	    {
		delete[] T->tensor_tiles;
		delete[] T->tile_address;
		T->tensor_tiles=new double[T->block_size * count];
		T->tile_address=new int[dims * count];
		T->num_actual_tiles=count;
	    }
	    

	    
        
	    // Gather addresses and blocks in this dimension
	    MPI_Bcast( T->tile_address, T->num_actual_tiles * dims, MPI_INT,dim_group_ranks[0], new_comm);
	    MPI_Bcast( T->tensor_tiles, T->num_actual_tiles * T->block_size, MPI_DOUBLE,dim_group_ranks[0], new_comm);
	}
	if(rank==RRANK && DEBUG_CHECK_POINT) cout << "Rank : "<<rank<<". Replicate Redistribute Checkpoint " <<(checkpoint++)<< endl; 
/*
// Free old index table
T->free_index_table();



// Create new index map
T->init_index_table();
T->fill_index_table();*/

        delete[] dim_group_ranks;
        delete[] new_proc_add;
	if(rank==RRANK && DEBUG_CHECK_POINT) cout << "Rank : "<<rank<<". Replicate Redistribute Checkpoint " <<(checkpoint++)<< endl; 
    }
    
}



