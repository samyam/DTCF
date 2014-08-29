#include "grid_redib.h"
#include "tensor.h"
namespace RRR{
using namespace std;


//-------------------------------------------------------------------------------------------//


// Constructor
GridRedistribute::GridRedistribute(Tensor* tensor, int* new_index_map, Grid* new_g)
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
        new_pgrid = new int[new_grid->grid_dims+1];
	for(int i=0;i<new_grid->grid_dims;i++)new_pgrid[i]=new_grid->pgrid[i];
	new_pgrid[new_grid->grid_dims]=1;

        // Recompute new proc_addr
        new_proc_addr = new int[grid_dims];
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

GridRedistribute::~GridRedistribute()
{
        delete[] new_proc_addr;
        delete[] req_arr;
}

// Redistribute the tensor in the processor grid as per a new dimension mapping
void GridRedistribute::redistribute()
{

        // Re-compute num_max_tiles for the redistributed tensor
        T->num_max_tiles = T->compute_num_max_tiles(new_idx_map, new_pgrid);

        // Post send to the processor that is supposed to hold the blocks currently at this processor
        redistribute_send();



        // Post receive from the processor that currently holds the blocks that should be at this processor
        list<recv_data> recv_list;
        redistribute_recv(recv_list);

         //Check for completion of Isends
	
        MPI_Status* st_arr = new MPI_Status[req_count];
        MPI_Waitall(req_count, req_arr, st_arr);
        delete[] st_arr;
	
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
void GridRedistribute::redistribute_send()
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
 			cout<<"aaaaa"<<i<<endl;
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
					cout<<"Sending from "<<T->rank<<" to "<< new_proc_rank[i]<<endl;
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
void GridRedistribute::redistribute_recv(list<recv_data> &recv_list)
{
        int* local_indices = new int[dims];
        int offset = 0;
        int* addresses = new int[T->num_max_tiles * dims];
                
        // Generate addresses that should reside at this processor w.r.t. the new index map
        //int num_tiles = T->num_actual_tiles;
	int num_tiles = 0;
        T->get_num_tiles(0, local_indices, offset, new_idx_map, new_proc_addr, addresses, new_pgrid, num_tiles);
        
	
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

                                MPI_Request req1, req2;

                                MPI_Irecv(rd.blocks, num_blocks * T->block_size, MPI_DOUBLE, i, 3, MPI_COMM_WORLD, &req1);  
                                MPI_Irecv(rd.block_addrs, num_blocks * dims, MPI_INT, i, 4, MPI_COMM_WORLD, &req2); 
				cout<<"Recieving from "<<i <<" to "<<T->rank<<endl;
                                recv_list.push_back(rd);
				req_arr[req_count++] = req1;
                                req_arr[req_count++] = req2;
				
                        }
                }
        }


        // Free allocated memory
        delete[] map;
        delete[] local_indices;
        delete[] addresses;
      //  delete[] old_proc_addr;
        delete[] old_proc_ranks;
}



}
