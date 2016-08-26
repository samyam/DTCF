#include "redistribute.h"
#include "tensor.h"
namespace RRR{
using namespace std;


// Get a list of replicated dimensions in the given index dimension map
int Tensor::get_replicated_dims(int* &idmap, int* &repl_dims)
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


// Create generic proc address for broadcast receivers from the sender for redistribution.
// Return a map of this generic address to the block offsets that are to be broadcasted to this group.
map<int, list<int> > Tensor::get_generic_proc_addresses(int* &tile_addresses, 
		int num_tiles, 
		int* &new_idx_map,
		int non_common_repl_dims_count, 
		int* &non_common_repl_dims)
{
	//print_tile_addr(g->grid_dims, g->proc_addr); cout << " now contains: "; print_tile_addrs(dims, tile_address, num_tiles); cout << endl;

	map<int, list<int> > proc_block_map;

	// Compute new processor address for each block
	for (int i=0; i < num_tiles; i++)
	{
		int offset = i * dims;
		int* new_proc_address = new int[grid_dims];
		memset(new_proc_address, 0, grid_dims * sizeof(int));
		for(int j=0; j<dims; j++)
		{
			new_proc_address[new_idx_map[j]] = (tile_addresses + offset)[j] % g->pgrid[new_idx_map[j]];
		}

		// For the old replicated dims that are not replicated in the new mapping,
		// check if the old and new processor addresses in that dimension are same
		// Set new proc addr in the dimensions that are replicated in the new map to zero, so that we can get
		// a generic proc id to broadcast/send to
		bool match = true;
		for(int j=0; j<non_common_repl_dims_count; j++)
		{
			if(new_proc_address[non_common_repl_dims[j]] != proc_addr[non_common_repl_dims[j]])
			{
				match = false;
				break;
			}
		}

		// Find the generic rank(id) for this proc address and map it to the block id 
		// that is being sent
		if(match == true)
		{
			int r = g->get_proc_rank(new_proc_address);
			if(proc_block_map.find(r) == proc_block_map.end())
			{
				proc_block_map[r] = list<int>();
			}
			proc_block_map[r].push_back(i);
			//if(rank==8) cout << rank << " wants to send " << i<< " to " << r << endl;
		}
	}

	return proc_block_map;
}


// Find senders for all the blocks this processor will hold after redistribution
// and return a map of sender rank with a list of block offsets that will be received from it
map<int , list<int> > Tensor::get_recv_proc_block_map(
		int* &new_idx_map,
		int old_repl_dims_count,
		int* &old_repl_dims,
		int &num_new_tiles)
{
	// Find out whom to receive from, the receive data size and broadcast groups
	// new_addresses stores block addresses that needs to be stored on this processor based
	// on the new index map
	int *local_indices = new int[dims];
	int offset = 0;
	int* new_addresses = new int[compute_num_max_tiles(new_idx_map, pgrid) * dims];
	num_new_tiles = 0;
	get_num_tiles(0, local_indices, offset, new_idx_map, proc_addr, new_addresses, pgrid, num_new_tiles); 

	//print_tile_addr(g->grid_dims, g->proc_addr); cout << " wants: "; print_tile_addrs(dims, new_addresses, num_new_tiles); cout << endl;

	//Maps the processors from which data is to be recieved to list of blocks which needs to be recieved. 
	//Here the processor address used for the map is not generic since a reciever needs to find a unique
	//sender to recieve data from
	map<int, list<int> > recv_proc_block_map;

	// Compute sender processor address for each block
	for (int i=0; i < num_new_tiles; i++)
	{
		offset = i * dims;
		int* old_proc_address = new int[grid_dims];
		memset(old_proc_address, 0, grid_dims * sizeof(int));

		for(int j=0; j<dims; j++)
		{
			old_proc_address[index_dimension_map[j]] = (new_addresses + offset)[j] % g->pgrid[index_dimension_map[j]];
		}

		//find unique sender that is closest to this receiver
		for(int j=0; j<old_repl_dims_count; j++)
		{
			old_proc_address[old_repl_dims[j]] = proc_addr[old_repl_dims[j]];
		}

		int r = g->get_proc_rank(old_proc_address);
		if(recv_proc_block_map.find(r) == recv_proc_block_map.end())
		{
			recv_proc_block_map[r] = list<int>();
		}
		recv_proc_block_map[r].push_back(i);

		//if(rank == 4) cout << rank << " expects " << i << " from " << r << endl;
	}

	return recv_proc_block_map;
}


// Create broadcast groups and communicators for redistribution
void Tensor::get_bcast_groups(
		map<int, list<int> > proc_block_map, 
		map<int, list<int> > recv_proc_block_map, 
		int old_repl_dims_count,
		int* &old_repl_dims,
		int bcast_dims_count,
		int* &bcast_dims,
		MPI_Comm &recv_comm,
		MPI_Comm* &bcast_send_comm,
		MPI_Comm* &bcast_recv_comm)
{
	// Create self communicators
	MPI_Comm self_comm;
	MPI_Comm_split(MPI_COMM_WORLD, rank, rank, &self_comm);

	// Create broadcast receiver groups and intra-communicators
	// Color the processors
	int* color_code = new int[grid_dims];
	memcpy(color_code, proc_addr, grid_dims * sizeof(int));
	for(int i=0; i<bcast_dims_count; i++)
	{
		color_code[bcast_dims[i]] = 0;
	}	
	int color = g->get_proc_rank(color_code);
	// Split the communicator into recv groups
	MPI_Comm_split(MPI_COMM_WORLD, color, rank, &recv_comm);

	// Form intercommunicators
	//int *recv_leaders = new int[proc_block_map.size()];
	map<int, list<int> > recv_leader_block_map;
	for(map<int, list<int> >::iterator it=proc_block_map.begin(); it != proc_block_map.end(); ++it)
	{
		// Find the leader for the broadcast receive group
		int recv_leader = it->first;
		int* recv_leader_addr = new int[grid_dims];
		g->get_proc_addr(recv_leader, recv_leader_addr);
		for(int i=0; i<old_repl_dims_count; i++)
		{
			recv_leader_addr[old_repl_dims[i]] = proc_addr[old_repl_dims[i]];
		}
		recv_leader = g->get_proc_rank(recv_leader_addr);
		recv_leader_block_map[recv_leader] = it->second;
		//cout << rank << " sending to " << recv_leader << endl;//print_tile_addr(g->grid_dims, recv_leader_addr); cout <<endl;
	}



	// Call creates for the inter-communicators from the broadcast senders and receivers.
	// If sender is already in the receive group, then use the intra-communicator.
	// The order of calling creates is ascending w.r.t. ranks
	bcast_send_comm = new MPI_Comm[recv_leader_block_map.size()];
	bcast_recv_comm = new MPI_Comm[recv_proc_block_map.size()];
	bool send_creates_done = false;
	for(map<int, list<int> >::iterator r_it=recv_proc_block_map.begin(); r_it != recv_proc_block_map.end(); ++r_it)
	{

		int sender = r_it->first;
		int index = distance(recv_proc_block_map.begin(), r_it);
		//if(rank==4)cout << rank << " receiving from " << r_it->first << endl;

		//if(sender == rank)
		if(sender >= rank && send_creates_done == false)
		{
			post_send_creates(recv_leader_block_map, bcast_dims_count, bcast_dims, self_comm, recv_comm, bcast_send_comm);
			send_creates_done = true;
		}
		//else {
		if(sender != rank)
		{
			// Create the inter-communicator from receiver only if the sender is not a part of recv_comm
			int* sender_addr = new int[grid_dims];
			g->get_proc_addr(sender, sender_addr);
			for(int i=0; i<bcast_dims_count; i++)
			{
				sender_addr[bcast_dims[i]] = 0;
			}
			int recv_leader = color; // Color is the smallest rank, i.e. leader in this recv_group
			if(g->get_proc_rank(sender_addr) == recv_leader) // sender is in recv group
			{
				//if(rank==8) cout << rank << " sender " << sender << " is in the recv group" << endl;
				bcast_recv_comm[index] = recv_comm;
			}
			else
			{
				MPI_Comm intercomm; // = bcast_recv_comm[index];
				//cout << g->get_proc_addr_str(rank) << " creates intercomm with sender = " << g->get_proc_addr_str(sender) << endl;
				MPI_Intercomm_create(recv_comm, 0, MPI_COMM_WORLD, sender, 0, &intercomm);
				bcast_recv_comm[index] = intercomm;
			}
		}
	}

	if(!send_creates_done)
	{
		post_send_creates(recv_leader_block_map, bcast_dims_count, bcast_dims, self_comm, recv_comm, bcast_send_comm);
		send_creates_done = true;
	}

}


// Post MPI_Intercomm_Create from senders
void Tensor::post_send_creates(
		map<int, list<int> > recv_leader_block_map,
		int bcast_dims_count,
		int* &bcast_dims,
		MPI_Comm &self_comm,
		MPI_Comm &recv_comm,
		MPI_Comm* &bcast_send_comm)
{
		for(map<int, list<int> >::iterator s_it=recv_leader_block_map.begin(); s_it != recv_leader_block_map.end(); ++s_it)
		{
			int recv_leader = s_it->first;
			int index = distance(recv_leader_block_map.begin(), s_it);

			// Find out if the sender is a part of the recv group
			int* addr = new int[grid_dims];
			memcpy(addr, proc_addr, grid_dims*sizeof(int));
			for(int i=0; i<bcast_dims_count; i++)
			{
				addr[bcast_dims[i]] = 0;
			}
			if(g->get_proc_rank(addr) == recv_leader) // sender is in recv group
			{
				//if(rank==0) cout << rank << " as a sender is in the recv group" << endl;
				bcast_send_comm[index] = recv_comm;
			}
			else // sender is not in recv group
			{
				// Create the inter-communicator from sender
				MPI_Comm intercomm; //= bcast_send_comm[index];
				//cout << g->get_proc_addr_str(rank) << " creates intercomm with recvleader = " << g->get_proc_addr_str(recv_leader) << endl;
				MPI_Intercomm_create(self_comm, 0, MPI_COMM_WORLD, recv_leader, 0, &intercomm);
				bcast_send_comm[index] = intercomm;
			}
		}
}

// Create send buffers for the blocks to broadcast from this processor for redistributing the tensor
void Tensor::copy_bcast_send_data(map< int, list<int> > proc_block_map, double** &bcast_blocks, int** &bcast_addr)
{
	bcast_blocks = new double*[proc_block_map.size()];
	bcast_addr = new int*[proc_block_map.size()];
	for(map<int, list<int> >::iterator it=proc_block_map.begin(); it != proc_block_map.end(); ++it)
	{
		int index = distance(proc_block_map.begin(), it);
		bcast_blocks[index] = new double[(it->second).size() * block_size];
		bcast_addr[index]   = new int[(it->second).size() * dims];
		int offset = 0;
		for(list<int>::iterator l_it = (it->second).begin(); l_it != (it->second).end(); l_it++)
		{
			memcpy(bcast_blocks[index] + offset*block_size, tensor_tiles + (*l_it)*block_size, block_size * sizeof(double)); 
			memcpy(bcast_addr[index] + offset*dims, tile_address + (*l_it)*dims, dims * sizeof(int)); 
			offset++;
		}
	}
}


// Post sends for redistribute
void Tensor::post_broadcast_sends(
		map< int, list<int> > &proc_block_map,
		MPI_Comm* &bcast_send_comm,
		MPI_Comm &recv_intra_comm,
		int intra_comm_rank,
		double** &bcast_blocks,
		int** &bcast_addr,
		double* &bcast_recv_blocks,
		int* &bcast_recv_addr,
		int &offset
)
{
		for(map<int, list<int> >::iterator send_it = proc_block_map.begin(); send_it != proc_block_map.end(); ++send_it)
		{
			int send_index = distance(proc_block_map.begin(), send_it);
			int recv_leader = send_it->first;
			MPI_Comm comm = bcast_send_comm[send_index];

			int me = MPI_ROOT;
			if(comm == recv_intra_comm) // sender is in the recv intracomm
				me = intra_comm_rank;

			MPI_Bcast(bcast_blocks[send_index], (send_it->second).size() * block_size, MPI_DOUBLE, me, comm);
			MPI_Bcast(bcast_addr[send_index], (send_it->second).size() * dims, MPI_INT, me, comm);

			// If sender is in the recv intracomm, retain the blocks sent in bcast_recv_blocks
			if(comm == recv_intra_comm)
			{
				memcpy(bcast_recv_blocks + offset*block_size, bcast_blocks[send_index], (send_it->second).size() * block_size * sizeof(double));
				memcpy(bcast_recv_addr + offset*dims, bcast_addr[send_index], (send_it->second).size() * dims * sizeof(int));
				offset += (send_it->second).size();
			}
		}
}


void Tensor::redistribute_broadcast(int* &new_idx_map,
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
		int non_common_repl_dims_count)
{
	//Maps the generic processor address stored as an integer to a list containing 
	//all the blocks that needs to be sent to this processor
	map< int, list<int> > proc_block_map = get_generic_proc_addresses(tile_address, 
			num_actual_tiles, new_idx_map, non_common_repl_dims_count, non_common_repl_dims);

	int* num_send_blocks;// = new int[proc_block_map.size()];
	int num_new_tiles = 0;

	map<int, list<int> > recv_proc_block_map = get_recv_proc_block_map(new_idx_map, old_repl_dims_count, old_repl_dims, num_new_tiles);

	MPI_Comm *bcast_send_comm, *bcast_recv_comm, recv_intra_comm;

	//bcast_send_comm is an array of communicators to which the data needs to be broadcasted
	//remember that proc_block_map maps generic addresses to the blocks. This generic address
	//is used to compute actual addresses to be broadcasted to, using replicated dimensions
	//in the new index map that are not replicated in the old index map
	get_bcast_groups(proc_block_map, recv_proc_block_map, old_repl_dims_count, old_repl_dims,
			bcast_dims_count, bcast_dims, recv_intra_comm, bcast_send_comm, bcast_recv_comm);


	// Broadcast

	//stores data and addresses to be sent to all the processors
	double** bcast_blocks;
	int** bcast_addr;
	copy_bcast_send_data(proc_block_map, bcast_blocks, bcast_addr);

	double* bcast_recv_blocks = new double[num_new_tiles * block_size];
	int* bcast_recv_addr = new int[num_new_tiles * dims];
	int offset = 0;

	int intra_comm_rank;
	MPI_Comm_rank(recv_intra_comm, &intra_comm_rank);

	double test = 1.0;

	// All receives happen in the increasing order of sender ranks in MPI_COMM_WORLD.
	// This processor sends all its sends in the increasing order of receiver group leader ranks.
	// All the sends are embedded together in the list of receives such that all the receives that happen
	// before the sends are to the receive leaders with ranks less than this processor.
	bool sends_done = false;
	for(map<int, list<int> >::iterator it=recv_proc_block_map.begin(); it != recv_proc_block_map.end(); ++it)
	{
		int recv_index = distance(recv_proc_block_map.begin(), it);
		int sender = it->first;

		// Find sender processor's rank in its recv intra comm
		MPI_Group world_group, recv_group;
		MPI_Comm_group(MPI_COMM_WORLD, &world_group); 
		MPI_Comm_group(recv_intra_comm, &recv_group); 
		int* world_ranks = new int[1]; world_ranks[0] = sender;
		int* recv_ranks = new int[1]; 
		MPI_Group_translate_ranks(world_group, 1, world_ranks, recv_group, recv_ranks);
		int sender_intra_comm_rank = recv_ranks[0];

		// Send broadcast
		//if(sender == rank)
		if(sender >= rank && !sends_done)
		{
			post_broadcast_sends(proc_block_map, bcast_send_comm, recv_intra_comm, intra_comm_rank,
				bcast_blocks, bcast_addr, bcast_recv_blocks, bcast_recv_addr, offset);
			sends_done = true;
		}
		//else
		if(sender != rank)
		{
			// Receive broadcast
			MPI_Comm comm = bcast_recv_comm[recv_index];
			int num_recv_blocks = (it->second).size();
			int bcast_sender = 0;
			if(comm == recv_intra_comm) // Sender is in the recv intracomm
				bcast_sender = sender_intra_comm_rank;

			MPI_Bcast(bcast_recv_blocks + offset*block_size, num_recv_blocks * block_size, MPI_DOUBLE, bcast_sender, comm);
			MPI_Bcast(bcast_recv_addr + offset*dims, num_recv_blocks * dims, MPI_INT, bcast_sender, comm);
			offset += num_recv_blocks;
		}
	}

	if(!sends_done)
	{
		post_broadcast_sends(proc_block_map, bcast_send_comm, recv_intra_comm, intra_comm_rank,
				bcast_blocks, bcast_addr, bcast_recv_blocks, bcast_recv_addr, offset);
		sends_done = true;
	}

	//Correctness checking code
	//{
	//	int *local_indices = new int[dims];
	//	int offset = 0;
	//	int* new_addresses = new int[compute_num_max_tiles(new_idx_map, pgrid) * dims];
	//	int num_new_tiles = 0;
	//	get_num_tiles(0, local_indices, offset, new_idx_map, proc_addr, new_addresses, pgrid, num_new_tiles); 
	//	are_tile_addrs_equal(dims, bcast_recv_addr, new_addresses, num_new_tiles);
	//}

	// Update Tensor tiles and addresses
	delete[] tensor_tiles;
	delete[] tile_address;

	tensor_tiles = bcast_recv_blocks;
	tile_address = bcast_recv_addr;
	num_actual_tiles = num_new_tiles;
}


void Tensor::redistribute_point_to_point(int* &new_idx_map,
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
		int non_common_repl_dims_count)
{

	//stores data and addresses to be sent to all the processors
	double** send_blocks;
	int** send_addr;
	int* num_send_blocks;// = new int[proc_block_map.size()];

	//Maps the generic processor address stored as an integer to a list containing 
	//all the blocks that needs to be sent to this processor
	map< int, list<int> > proc_block_map = get_generic_proc_addresses(tile_address, 
			num_actual_tiles, 
			new_idx_map, 
			non_common_repl_dims_count, 
			non_common_repl_dims);

	//fills send_blocks and send_addr with blocks and addresses to be sent. 
	//num_send_blocks holds the number of data blocks to be sent to receivers
	map<int, int> proc_block_num_map = compile_senders_data(proc_block_map,
			send_blocks,
			send_addr,
			num_send_blocks,
			old_repl_dims,
			old_repl_dims_count);



	// Find out whom to receive from, the receive data size and broadcast groups
	//new_addresses stores block addresses that needs to be stored on this processor based
	//on the new index map
	int *local_indices = new int[dims];
	int offset = 0;
	int* new_addresses = new int[compute_num_max_tiles(new_idx_map, pgrid) * dims];;
	int num_new_tiles = 0;
	get_num_tiles(0, local_indices, offset, new_idx_map, proc_addr, new_addresses, pgrid, num_new_tiles); 


	//Maps the processors from which data is to be recieved to list of blocks which needs to be recieved. 
	//Here the processor address used for the map is not generic since a reciever needs to find a unique
	//sender to recieve data from
	map<int, list<int> > recv_proc_block_map;

	// Compute sender processor address for each block
	for (int i=0; i < num_new_tiles; i++)
	{
		offset = i * dims;
		int* old_proc_address = new int[grid_dims];
		memset(old_proc_address, 0, grid_dims * sizeof(int));

		for(int j=0; j<dims; j++)
		{
			old_proc_address[index_dimension_map[j]] = (new_addresses + offset)[j] % g->pgrid[index_dimension_map[j]];
		}

		//find unique sender that is closest to this receiver
		for(int j=0; j<old_repl_dims_count; j++)
		{
			old_proc_address[old_repl_dims[j]] = proc_addr[old_repl_dims[j]];
		}


		int r = g->get_proc_rank(old_proc_address);
		if(recv_proc_block_map.find(r) == recv_proc_block_map.end())
		{
			recv_proc_block_map[r] = list<int>();
		}
		recv_proc_block_map[r].push_back(i);
	}

	double* recv_blocks = new double[num_new_tiles * block_size];
	int* recv_addr = new int[num_new_tiles * dims];

	//offset keeps track of how many data blocks have been received
	offset = 0;

	//number of sends and receives that are actually performed
	int send_counter = 0;
	int recv_counter = 0;

	MPI_Request* req_data = new MPI_Request[proc_block_num_map.size() + recv_proc_block_map.size()];
	MPI_Request* req_addr = new MPI_Request[proc_block_num_map.size() + recv_proc_block_map.size()];

	//Send Data to all the receivers
	for(map<int,int>::iterator it = proc_block_num_map.begin(); it != proc_block_num_map.end(); ++it)
	{
		int receiver_rank = it->first;
		int block_offset = it->second;
		int num_blocks = num_send_blocks[block_offset];

		//if the receiver and sender are the same then just do a memcpy
		if(receiver_rank == rank)
		{
			memcpy(recv_blocks, 
					send_blocks[block_offset], 
					num_blocks * sizeof(double)*block_size);

			memcpy(recv_addr, 
					send_addr[block_offset], 
					num_blocks * sizeof(int)*dims);

			offset = num_blocks;
		}

		//if the receiver and senders are different then perform an ISend
		else
		{
			MPI_Isend(send_blocks[block_offset], 		   
					num_blocks *block_size, 
					MPI_DOUBLE, receiver_rank, 3, MPI_COMM_WORLD, 
					&req_data[send_counter]);  

			MPI_Isend(send_addr[block_offset], 
					num_blocks * dims, 
					MPI_INT, receiver_rank, 4, MPI_COMM_WORLD, 
					&req_addr[send_counter]);

			send_counter++;
		}
	}

	//Receive data from all the senders
	for(map<int,list<int> >::iterator it = recv_proc_block_map.begin(); it != recv_proc_block_map.end(); ++it)
	{
		int sender_rank = it->first;
		int num_blocks = (it->second).size();

		double* recv_data_buffer = recv_blocks + offset * block_size;
		int* recv_addr_buffer = recv_addr + offset * dims;


		if(sender_rank != rank)
		{
			MPI_Irecv(recv_data_buffer , 		   
					num_blocks * sizeof(double)*block_size, 
					MPI_DOUBLE, sender_rank, 3, MPI_COMM_WORLD, 
					&req_data[send_counter+recv_counter]);  

			MPI_Irecv(recv_addr_buffer, 
					num_blocks* sizeof(double)*dims, 
					MPI_INT, sender_rank, 4, MPI_COMM_WORLD, 
					&req_addr[send_counter+recv_counter]);

			offset += num_blocks;
			recv_counter++;
		}
	}

	int num_comm = send_counter + recv_counter;

	MPI_Status* status_data = new MPI_Status[num_comm];
	MPI_Status* status_addr = new MPI_Status[num_comm];

	//wait for all the send and receives to complete
	int st_data = MPI_Waitall(num_comm, req_data, status_data);
	int st_addr = MPI_Waitall(num_comm, req_addr, status_addr);

	// Correctness checking
	//print_proc_addr(grid_dims, pgrid, rank);
	//are_tile_addrs_equal(dims, recv_addr, new_addresses, num_new_tiles);

	delete[] status_data;
	delete[] status_addr;
	delete[] req_data;
	delete[] req_addr;
	delete[] send_blocks;
	delete[] send_addr;			 

	// Update Tensor tiles and addresses
	delete[] tensor_tiles;
	delete[] tile_address;

	tensor_tiles = recv_blocks;
	tile_address = recv_addr;
	num_actual_tiles = num_new_tiles;
}

// Redistribute this tensor as per the provided new index dimension map
void Tensor::redistribute(int* &new_idx_map)
{
	//cout<< g->get_proc_addr_str(rank) << " holds: "; printGetTiles(tensor_tiles, tile_address, block_size, num_actual_tiles, dims);
	//cout << endl;

	bool send = false;
	int bcast_proc_count = 1;
	int* bcast_dims_sizes;

	// Get lists of old as well as new replicated dimensions
	int *old_repl_dims, *new_repl_dims;
	int old_repl_dims_count = get_replicated_dims(index_dimension_map, old_repl_dims);
	int new_repl_dims_count = get_replicated_dims(new_idx_map, new_repl_dims);

	// Allocate memory for storing broadcast dimensions
	int* bcast_dims = new int[new_repl_dims_count];
	int bcast_dims_count = 0;
	// Dimensions that are replicated in both old and new distribution
	int* common_repl_dims = new int[old_repl_dims_count];
	int common_repl_dims_count = 0;
	// Dimensions that are replicated only in the old distribution but not in the new one
	int* non_common_repl_dims = new int[old_repl_dims_count];
	int non_common_repl_dims_count = 0;
	// Number of MPI ranks in each bcast dimension
	bcast_dims_sizes = new int[new_repl_dims_count];

	// Find if the new replicated dims are also replicated in old. If yes, it means we don't need broadcast.
	// If it is not replicated in old, a broadcast is needed. Save the broadcast dimension (the new replicated dim)
	for(int i=0; i<new_repl_dims_count; i++)
	{
		bool found = false;
		for(int j=0; j<old_repl_dims_count; j++)
		{	
			if(new_repl_dims[i] == old_repl_dims[j])
			{
				found = true;
				break;
			}
		}
		if(found == false)
		{
			bcast_dims[bcast_dims_count] = new_repl_dims[i];
			bcast_dims_sizes[bcast_dims_count] = g->pgrid[new_repl_dims[i]];
			bcast_dims_count++;
			bcast_proc_count *= g->pgrid[new_repl_dims[i]];
		}
	}

	// Find the common replicated dimensions in old and new
	// And the non-common replicated dims in old index map
	for(int i=0; i<old_repl_dims_count; i++)
	{
		bool found = false;
		for(int j=0; j<new_repl_dims_count; j++)
		{
			if(new_repl_dims[j] == old_repl_dims[i])
			{
				found = true;
				common_repl_dims[common_repl_dims_count] = old_repl_dims[i];
				common_repl_dims_count++;
				break;
			}
		}
		if(found == false)
		{
			non_common_repl_dims[non_common_repl_dims_count] = old_repl_dims[i];
			non_common_repl_dims_count++;
		}
	}

	if(bcast_dims_count > 0)
	{
		redistribute_broadcast( new_idx_map,
				bcast_proc_count,
				bcast_dims_sizes,
				old_repl_dims, 
				new_repl_dims,
				old_repl_dims_count, 
				new_repl_dims_count, 
				bcast_dims, 
				bcast_dims_count,
				common_repl_dims,
				common_repl_dims_count,
				non_common_repl_dims,
				non_common_repl_dims_count);

	}
	else
	{
		redistribute_point_to_point(new_idx_map,
				bcast_proc_count,
				bcast_dims_sizes,
				old_repl_dims, 
				new_repl_dims,
				old_repl_dims_count, 
				new_repl_dims_count, 
				bcast_dims, 
				bcast_dims_count,
				common_repl_dims,
				common_repl_dims_count,
				non_common_repl_dims,
				non_common_repl_dims_count);

	}

	// Free old index table
	free_index_table();

	// Update idmap
	//cout << "New idx map: ";
	//print_tile_addr(dims, new_idx_map); cout << endl;
	memcpy(index_dimension_map, new_idx_map, dims*sizeof(int));
	//cout << "New idx map copied: ";
	//print_tile_addr(dims, index_dimension_map); cout << endl;

	// Recompute index_table
	init_index_table();
	fill_index_table();

}


//Fills send_blocks and send_addr with the blocks and associated addresses which
//will be sent to receivers. Returns a map from receivers, to the offset at which
//the blocks and addresses to be sent to the receiver is stored
map<int, int> Tensor::compile_senders_data(map<int, list<int> > proc_block_map,
		double** &send_blocks,
		int** &send_addr,
		int* &num_send_blocks,
		int* &old_repl_dims,
		int old_repl_dims_count)
{

	//total number of receivers is the size of proc_block_map
	num_send_blocks = new int[proc_block_map.size()];
	send_blocks = new double*[proc_block_map.size()];
	send_addr = new int*[proc_block_map.size()];

	//maps rank of receivers to the offset at which the blocks and addresses to be sent is stored
	map<int, int> proc_block_num_map;

	for(map<int, list<int> >::iterator it=proc_block_map.begin(); it != proc_block_map.end(); ++it)
	{
		int index = distance(proc_block_map.begin(), it);

		// Generates the recievers address from the generic addrress
		int *receiver_addr;

		g->get_proc_addr(it->first, receiver_addr);

		for(int i=0; i<old_repl_dims_count; i++)
		{
			receiver_addr[old_repl_dims[i]] = proc_addr[old_repl_dims[i]];
		}

		//holds the rank of the receving node
		int receiver_rank = g->get_proc_rank(receiver_addr);

		proc_block_num_map[receiver_rank] = index;

		//allocates memory for blocks and addresses to be sent
		send_blocks[index] = new double[(it->second).size() * block_size];
		send_addr[index] = new int[(it->second).size() * dims];
		num_send_blocks[index] = (it->second).size();


		//fills blocks and addresses that will be sent to receiver rank
		for(list<int>::iterator l_it = (it->second).begin(); l_it != (it->second).end(); l_it++)
		{
			int offset = distance((it->second).begin(), l_it);

			memcpy(send_blocks[index] + offset*block_size, 
					tensor_tiles + *l_it * block_size, 
					sizeof(double)*block_size);

			memcpy(send_addr[index] + offset*dims, 
					tile_address + *l_it * dims, 
					sizeof(int)*dims);

		}
	}

	return proc_block_num_map;

}



//-------------------------------------------------------------------------------------------//


// Constructor
TensorRedistributor::TensorRedistributor(Tensor* tensor, int* new_index_map, Grid* new_g)
{
	T = tensor;
	new_grid = new_g;
	old_grid = T->g;

	dims = T->dims;
	grid_dims = new_grid->grid_dims;
	serial = grid_dims;

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

TensorRedistributor::~TensorRedistributor()
{
	delete[] new_proc_addr;
	delete[] req_arr;
}


// Redistribute the tensor in the processor grid as per a new dimension mapping
void TensorRedistributor::redistribute()
{
	// Re-compute num_max_tiles for the redistributed tensor
	T->num_max_tiles = T->compute_num_max_tiles(new_idx_map, new_pgrid);

	// Take care of serial dimensions here
	for(int i=0; i < T->dims; i++)
	{
		// If this dimension was serialized in the old map and distributed in the new one,
		// just delete the unnecessary block from the processors.
		//if(old_idx_map[i] == serial && new_idx_map[i] != serial)
		//{
		//    int distr_grid_dim = new_idx_map[i];
		//    deserialize(i, distr_grid_dim);
		//}

		// If this dimension was distributed in the old map and serialized in the new one,
		// do an all-to-all broadcast to serialize it
		if(old_idx_map[i] != serial && new_idx_map[i] == serial)
		{
			int ser_grid_dim = old_idx_map[i];
			serialize(i, ser_grid_dim);
			return;
		}
	}


	// Post send to the processor that is supposed to hold the blocks currently at this processor
	redistribute_send();

	// Post receive from the processor that currently holds the blocks that should be at this processor
	list<recv_data> recv_list;
	redistribute_recv(recv_list);

	// Check for completion of Isends
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
int TensorRedistributor::get_blocks_for_proc(int index, int* &ranks, bool* &sent, double* &blocks, int* &block_addrs)
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
int TensorRedistributor::get_num_blocks_from_proc(int index, int* &ranks, int num_tiles)
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
int TensorRedistributor::get_num_recv_blocks(int* &ranks, int num_tiles, int* &map)
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
void TensorRedistributor::redistribute_send()
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
void TensorRedistributor::redistribute_recv(list<recv_data> &recv_list)
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
void TensorRedistributor::serialize(int ser_tensor_dim, int ser_grid_dim)
{
	//if(T->rank==0) cout << " Serializing tensor_dim = " << ser_tensor_dim << " grid_dim = " << ser_grid_dim << endl;

	// Create a new MPI_Communicator in the serialization dimension
	int num_procs = new_pgrid[ser_grid_dim];
	int* dim_group_ranks = new int[num_procs]; 
	int* processor_addr = new int[grid_dims];
	memcpy(processor_addr, T->proc_addr, grid_dims * sizeof(int));
	for(int i=0; i < num_procs; i++)
	{
		processor_addr[ser_grid_dim] = i;
		dim_group_ranks[i] = new_grid->get_proc_rank(processor_addr);
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
void TensorRedistributor::deserialize(int distr_tensor_dim, int distr_grid_dim)
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

}
