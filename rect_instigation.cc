#include "tensor.h"
#include "contraction.h"
#define DEBUG_I 1
#define DEBUG_T 0

#define RRANK 5
namespace RRR{
    using namespace std;
//returns number of processor addresses from which data needs to be
//bounced.The array of addresses are put in bouncers. my_address holds
//the invoking nodes address, index_dimension_map gives the mapping
//from index to processor dimension
    int Tensor::get_rect_bouncers(int contr_dim, int contr_idx,  int** &senders)
    {


	//if this processor is not an instigator
	if(contr_idx % pgrid[index_dimension_map[contr_dim]] != proc_addr[index_dimension_map[contr_dim]])
	    return 0;

	// Get the list of all dimensions which belong to the same symmetry group
	// as the contracting index

	list<int>* br_index = get_bounce_indx(contr_dim);
	int num_matches = br_index->size();

	int* match = new int[grid_dims];
	memset(match,0,sizeof(int)*grid_dims);

	//get the matching indices belonging to the same symmetry
	//group as an array.  The rest of the code uses an array
	//so I had to change the list to array.  Cou;d have stored
	//it in the array to begin with. Too lazy to change the
	//implementation of get_bounce_indx
	for(list<int>::iterator it = br_index->begin(); it!=br_index->end(); ++it)
	{
	    match[*it] = 1;

	}

	//stores the permutation used for altering the block
	//address due to symmetry
	int** sender_map = new int*[num_matches];
    

	//for each matching index, the following variables keeps track of
	//the processor id along each dimension holds processor id along
	//each dimension of the processor grid
	/////////////////////////////////////////////////////////////////
	int*** proc_id_per_dim = new int**[num_matches];

	//holds processor id along each dimension
	int** size_per_dim = new int*[num_matches];
	/////////////////////////////////////////////////////////////////


	//initialize
	int* matching_indices = new int[num_matches];
	int num_senders = 0;
	int senders_for_matching_index = 0;

	// Identify the receiver addresses
	int count = 0;
	
	for(int i =0; i < dims; i++)
	{

	
	    // If a receiver is identified for this dimension
	    if(match[i])
	    {

		senders_for_matching_index = 1;
		sender_map[count] = new int[grid_dims];
		
		//initialize the map
		for(int j = 0; j<grid_dims; j++)
		{
		    sender_map[count][j] = j;
		}

		sender_map[count][index_dimension_map[contr_dim]] = index_dimension_map[i];			  
			
		// Get the physical address permutation map. It tells that
		//block dimension distributed along index_dimension_map[j]
		//will be distribution along a different dimension to obtain
		//the correct block address. The permutation map is used to
		//identify multiple receiers due to permutation specially
		//when the size of the phhysical grid along the intial
		//dimensions and the new dimension where the block will be
		//sent to are different
		if(i<contr_dim)

		    for(int j = i; j< contr_dim; j++)
			sender_map[count][index_dimension_map[j]] = index_dimension_map[j+1];

		if(i>contr_dim)

		    for(int j = i; j> contr_dim; j--)
			sender_map[count][index_dimension_map[j]] = index_dimension_map[j-1];

		
		proc_id_per_dim[count] = new int*[grid_dims];
		size_per_dim[count]= new int[grid_dims];
		
		for(int l = 0; l < grid_dims; l++){
		    
		    int sender_dim  = sender_map[count][l];
		    //cout<<"Count : "<<count<<". Dimension : "<<l<<". Sender Dim : "<<sender_dim<<endl;
		    // only a single contraction index is being sent so
		    // there is only one receiving processor along this
		    // dimension
		    //multiple receivers can exist along the dimension
		    //to which the current dimenison is getting mapped
		    //to. The upper limit is given by Lowest Common
		    //Multiple divided by the number of processors along
		    //this dimension
		    if(index_dimension_map[contr_dim] == l){

			proc_id_per_dim[count][sender_dim] = new int[1];
			//cout<<proc_id_per_dim[count][sender_dim]<<endl;
			proc_id_per_dim[count][sender_dim][0] = contr_idx % pgrid[sender_dim];
			size_per_dim[count][sender_dim] = 1;
			
		    }else{
			int num_sender_along_dim = lcm(pgrid[l], pgrid[sender_dim]) / pgrid[l];
			//if(rank == 7 && contr_dim == 3) cout<<"Num_sender_along_dim : "<<num_sender_along_dim<<endl;
			size_per_dim[count][sender_dim] = num_sender_along_dim;
			proc_id_per_dim[count][sender_dim] = new int[num_sender_along_dim];

			for(int c =0; c< num_sender_along_dim; c++){

			    int block_addr_along_l = c*pgrid[l] + proc_addr[l];
			    //if (rank ==7 && contr_dim == 3) cout<<"Block Addr is "<<block_addr_along_l<<" tensor_range " << tensor_range[reverse_index_map[l]]<<"Reverse Index Map is "<<reverse_index_map[l]<<" and l is "<<l<<endl;

			    //if the block address does not exist then no need
			    //to receive from the processor that holds this
			    //block. We assume that the dimensionality of the
			    //tensor is always smaller than 99
			    if(reverse_index_map[l] < 99 && block_addr_along_l >= tensor_range[reverse_index_map[l]] && SG_index_map[reverse_index_map[l]] != CONTRACTED )
			    {
				//if(rank == 7 && contr_dim == 3) cout<<"In for c : "<<c<<" count "<<count<<" l "<<l<<endl;
				num_sender_along_dim = c;
				size_per_dim[count][sender_dim] = num_sender_along_dim;
				break;
			    }

			    int processor_addr = block_addr_along_l % pgrid[sender_dim];
			    proc_id_per_dim[count][sender_dim][c] = processor_addr;

			}
			senders_for_matching_index *= num_sender_along_dim;		    
		    }
		    

		}
		num_senders += senders_for_matching_index;
		
		matching_indices[count] = i;
		
		count++;
	    }
	    
	}
	

	senders = new int*[num_senders];
	int offset = 0;
	int cur_dim = 0;
	int* current_address = new int[grid_dims];
	memset(current_address,0.0,sizeof(int)*grid_dims);
	
//fill up the address for the receiver
	for(int i = 0; i< count; i++)
	{
	    create_cross_product(proc_id_per_dim[i], size_per_dim[i], 
				 dims, cur_dim, 
				 offset, current_address,
				 senders);	
	}
	
	delete[] current_address;
	delete[] match;
	return num_senders;
	//return 0;
    }
    
    
    
// Gets receiver addresses for current contracting dimension and index
    int Tensor::get_matching_indices_rect(
	int contr_dim,
	int contr_idx,
	int* &matching_indices //out	   
	)
    {
	// matching indices are the ones that are symmetric to the contraction indices
	int* match = new int[dims];     
    
	memset(match, 0, dims*sizeof(int));

	// Counts the number of processors that receive from this processor
	// and finds the indices that are symmetric to the contracting index
	int count = 0;

	for(int i =0; i < dims; i++)
	{
	    // index i is in the same symmetry group as the contracting dimension
	    if (((SG_index_map[i] == SG_index_map[contr_dim] &&
		  // Make sure that the index we are checking is a part of some symmetry group
		  SG_index_map[i] != NON_SYM)  ||  
		 // Or ensure this is the contracting dimension (then self sends)
		 i == contr_dim)  &&  
		// Make sure that this processor holds the contraction index
		proc_addr[index_dimension_map[i]] == contr_idx % pgrid[index_dimension_map[i]])
	    {
		// If there are multiple contraction indices belonging to the same symmetry group,
		// this condition maintains the order between these contraction indices
		// works only in CCSD when the size of symmetry group is 2
		if(contr_dim < i && cntr_map[i] > 0 )
		{
		    break;
		}

		match[i] = 1;
		count++;
	    }
	}

	int num_matches = count;

	//initialize
	matching_indices = new int[count];

	// Identify the receiver addresses
	count = 0;
	for(int i =0; i < dims; i++)
	{
	    // If a receiver is identified for this dimension
	    if(match[i])
	    {
		matching_indices[count] = i;
		count++;
	    }
	}
	delete[] match;
	return num_matches;
    }


    // send data to instigator
    void Contraction::send_to_instigator_rect(
	Tensor* &X,
	int contr_dim, 
	int contr_idx,
	int &count_addr_sends,
	int &count_data_sends
	) 
    {

	if(rank == RRANK && DEBUG_T) cout<<" The contraction dimension is "<<contr_dim<< " and the contraction index  is "<<contr_idx<<"."<<endl;
	if(rank == RRANK && DEBUG_T) cout<<" My address is "<<grid->get_proc_addr_str(rank)<<endl;
	if(rank == RRANK && DEBUG_T) X->printInfo();
	
	
	//initialize count variables that are either output variables or
	//private/public variables to this class
	count_addr_sends = 0;
	count_data_sends = 0;
	num_self_sends = 0;

	// Check if this node is responsible for sending bounce data
	int* matching_indices;
	int num_matches = 0;
	int** receivers;
	int num_receivers;

	// Find matching indices that are part of the same symmetry group
	// that hold the current contraction index along dimension contr_dim
	num_receivers = X->get_receivers_rect(contr_dim, contr_idx, receivers, matching_indices, num_matches);

	
	//for each reciever rank, it stores a list of all the blocks that
	//will be sent to the receiver. The first part of the pair stores
	//matching index( not really the index itselfbut just a counter for
	//identifying the index) and the second part of the store is used
	//for identifying the block. So the blocks are identified as
	//block_addrs[i][j]
	map< int, list< pair< int, int > > > msg_receiver_count; 
    
	//variable used for debugging. It is a grid storing receivers for each rank
	int* receiver_grid = new int[num_procs*num_procs];
	memset(receiver_grid,0,sizeof(int)*num_procs*num_procs);

	//initialize the list of tiles to be sent to each of the
	//instigators this list of receivers may be larger than
	//the list of receivers resulting from checking where each
	//of the tiles with the matching contr_dim and contr_indx
	//generated below. This is because, the list initialized
	//here does not account for symmetry. Due to symmetry,
	//some of tiles may not be present and the receivers
	//corresponding to those tiles need not recieve
	//anything. However, the receivers do not know this and
	//will post a receiver anyways. THerefore it is necessary
	//for the senders to send message of size 0 to these
	//receivers
	for (int i = 0; i< num_receivers; i++)
	{
	    int receiver_rank = grid->get_proc_rank(receivers[i]);
	    msg_receiver_count[receiver_rank] = list <pair <int, int> > ();
	    
	    //Debug Reduce
	    receiver_grid[rank * num_procs + receiver_rank] = 1;

	    if(rank == rank && DEBUG_I && contr_dim == 3 && contr_idx == 5) cout<<"My rank is "<<rank<<"and my Receivers are "<<grid->get_proc_addr_str(grid->get_proc_rank(receivers[i]))<<endl;
	    
	}
	
	
	int t = MPI_Reduce(receiver_grid, receiver_grid_buffer, 16*16, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	//if(DEBUG_T && rank == 0)
	//{
	//    cout<<endl;
	//    for(int i =-1; i< 16; i++){
	//	cout<<i<<" :"<<setw(10);
	//	for(int j = 0; j < 16; j++){
	//	    if(i == -1)
	//		cout<<j<<setw(5);
	//	    else
	//		cout<<receiver_grid_buffer[i*16+j]<<setw(5);
	//	}
	//	cout<<endl;
	//    
	//    }
	//}
	

	if(rank == RRANK && DEBUG_T) cout<<"May rank is "<<rank<<" and I have "<<msg_receiver_count.size()<<" receivers."<<endl;

	if(num_matches == 0)
	    return;

	int* paddr_along_matching_indices = new int[num_matches];
	
	

	int** tmp_block_addrs = new int*[num_matches];
	int** block_location = new int*[num_matches];


	int* num_tiles = new int[num_matches];
    
	//multiple matching indices can end up producing the same
	//tile to send. We keep track of the number of tiles with
	//a particular address and only keep unique copies of each
	//tile
	map < int , int > block_rank_count;
	
	//tile grid
	Grid* tgrid = new Grid(X->get_dims(), X->vgrid);

	int tdims = X->get_dims();
	//For each of the matching contraction dimension, get all the
	// matching addressess. Then permute each of the address to obtain
	// the address that needs to be sent. Also keep track of the
	// location of these tiles so that the data blocks could be copied
	// later.
	for(int i=0; i < num_matches; i++)
	{
	
	    //set the receiver address to current processor address
	    int* receiver_address = new int[grid_dims];
	    memcpy(receiver_address,my_address,sizeof(int)*grid_dims);
	    receiver_address[X->index_dimension_map[contr_dim]] = contr_idx % pgrid[X->index_dimension_map[contr_dim]];

	    //if(rank == 0) cout<<"matching index of reciever "<<i<<"is "<<matching_indices[i]<<endl;
	    //if(rank==0)X->print_all_tile_addr();
	
	    // Retrieve data to be sent 
	    num_tiles[i] = X->getTileAddresses(matching_indices[i], contr_idx, block_location[i], tmp_block_addrs[i]);

	    // Permute virtual block address to match the contraction dimension.  Also find the receivers address
	    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	    ///////////////////////////////////////////////////////////////////////////////////////////////////////////

	    //Matching index is comes before the contraction dimension
	    if(matching_indices[i]  <= contr_dim)
	    {
		for(int j = 0; j< num_tiles[i]; j++)
		{
			
		    //if (matching_indices[i] < contr_dim){
		    for(int d = matching_indices[i]; d<contr_dim; d++)
		    {
			int new_b = tmp_block_addrs[i][j * grid_dims + d + 1] ;
			tmp_block_addrs[i][j * tdims + d] = new_b;
			receiver_address[X->index_dimension_map[d]] = new_b % pgrid[X->index_dimension_map[d]];
		    }
		    tmp_block_addrs[i][j * tdims + contr_dim] = contr_idx;
			    
		    //discard a tile that has already been copied
		    int* taddr = tmp_block_addrs[i] + j*tdims; 
		    int trank = tgrid->get_proc_rank(taddr);
		    if (block_rank_count.find(trank) != block_rank_count.end()) continue;
		    block_rank_count[trank] = 1;
				
			
		    int receiver_rank = grid->get_proc_rank(receiver_address);
			
		    //sort each block to be sent to the correct receiver using
		    //the msg_receiver_count map
		    if(msg_receiver_count.find(receiver_rank) == msg_receiver_count.end())
		    {
			if(DEBUG_T && rank == RRANK) cout<<"The uninitialized receiver rank is "<<receiver_rank<<" with address "<<grid->get_proc_addr_str(receiver_rank)<<endl;
			
			assert(1 != 1);
			msg_receiver_count[receiver_rank] = list<pair<int,int>>();
			msg_receiver_count[receiver_rank].push_back(pair<int,int>(i,j));
		    }
		    else
		    {
			msg_receiver_count[receiver_rank].push_back(pair<int,int>(i,j));
		    }
		    
		}
	    }
	    
	    //Matching index comes after the contraction dimension
	    if(matching_indices[i]  > contr_dim)
	    {
		for(int j = 0; j< num_tiles[i]; j++)
		{
		
		
		    for(int d = matching_indices[i]; d>contr_dim; d--)
		    {
			int new_b = tmp_block_addrs[i][j * grid_dims + d - 1] ;
			tmp_block_addrs[i][j * tdims + d] = new_b;
			receiver_address[X->index_dimension_map[d]] = new_b % pgrid[X->index_dimension_map[d]];
		    }
		    tmp_block_addrs[i][j * tdims + contr_dim] = contr_idx;

		    //discard a tile that has already been copied
		    int* taddr = tmp_block_addrs[i] + j*tdims; 
		    int trank = tgrid->get_proc_rank(taddr);
		    if (block_rank_count.find(trank) != block_rank_count.end()) continue;
		    block_rank_count[trank] = 1;
		
		    int receiver_rank = grid->get_proc_rank(receiver_address);


		    //sort each block to be sent to the correct receiver using
		    //the msg_receiver_count map
		    if(msg_receiver_count.find(receiver_rank) == msg_receiver_count.end())
		    {
			assert(1 != 1);
			msg_receiver_count[receiver_rank] = list<pair<int,int>>();
			msg_receiver_count[receiver_rank].push_back(pair<int,int>(i,j));
		    }
		    else
		    {
			msg_receiver_count[receiver_rank].push_back(pair<int,int>(i,j));
		    }
		}
	    }
	    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	    ///////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	}

	for(map<int, list<pair<int,int>>>::iterator it = msg_receiver_count.begin(); it != msg_receiver_count.end(); ++it)
	{
	    // Find receiver rank
	    int receiver = it->first;

	    if(receiver == rank)
	    {
		num_self_sends++;
	    }
	    else
	    {
		count_addr_sends++;
	    }
	}

	//if(rank == rank) cout << rank << " num_self_sends = " << num_self_sends << endl << fflush;

	// Allocate MPI_Requests so that we can wait on them later
	send_req_addr = new MPI_Request[count_addr_sends];
	send_req_data = new MPI_Request[count_addr_sends];

	// Allocate memory for data that is already present in this processor
	if(num_self_sends != 0)
	{
	    self_adr_sizes = new   int[num_self_sends];
	    self_buffers = new double*[num_self_sends];
	    self_addresses = new  int*[num_self_sends];
	    self_data_sizes = new  int[num_self_sends];
	}
	else
	{
	    self_adr_sizes = 0; 
	    self_buffers = 0;
	    self_addresses = 0;
	    self_data_sizes = 0;
	}

	//reset
	num_self_sends = 0;
	count_addr_sends = 0;

	//tags for MPI messages
	int addr_tag = 1;
	int data_tag = 2;



	for(map<int, list<pair<int,int>>>::iterator it = msg_receiver_count.begin(); it != msg_receiver_count.end(); ++it)
	{
    
	    // Find receiver rank
	    int receiver = it->first;
	    
	    
	    int num_tiles = it->second.size();
	
	    int data_size, addr_size;
	    MPI_Comm comm = MPI_COMM_WORLD;    
	

	    // Compute size of addresses
	    addr_size = num_tiles * X->get_dims() ;

	    // Size of data
	    data_size = num_tiles * X->block_size;

	    int* block_addrs = new int[addr_size];
	    double* blocks = new double[data_size];
	

	    //fill address buffers with the correct tile addresses and the data buffer with correct data
	    if(addr_size > 0)
	    {
		if(rank == RRANK && DEBUG_T)
		    cout<<endl<<" Receiver Rank : "<<receiver<<" Receiver addr : "<<grid->get_proc_addr_str(receiver)<<endl;
		
		int offset = 0;

		for(list<pair<int,int>>::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
		{
		    int m = jt->first;
		    int t = jt->second;

		    if(rank == RRANK && DEBUG_T){
			int* addrr = tmp_block_addrs[m] + t * X->get_dims();
			print_tile_addr(X->get_dims(), addrr );

		    }
		    int* dest_addr = block_addrs +  X->get_dims() * offset;
		    double* dest_data = blocks + X->block_size * offset;
		    double* source_data = X->tensor_tiles + block_location[m][t] * X->block_size;
		
		    //copy tile addresses
		    memcpy(dest_addr, &tmp_block_addrs[m][t * X->get_dims()],X->get_dims()*sizeof(int));
		
		    //copy tile data
		    memcpy(dest_data,source_data, X->block_size*sizeof(double));

		    offset ++;
		
		}

	    }
	
	    //cout << rank << " receiver = "<<receiver<< endl;

	    // Send addresses and blocks if receivers are not this processor (sender)
	    if(receiver != rank)
	    {
		// Send addresses
		//cout << grid->get_proc_addr_str(rank) << "  sending addr_size " << addr_size << " to " << grid->get_proc_addr_str(receiver) << endl;
		//cout << rank << "  sending addr_size " << addr_size << " to " << receiver << endl;
		MPI_Isend(block_addrs, addr_size, MPI_INT, receiver, addr_tag, comm, &send_req_addr[count_addr_sends]);
		count_addr_sends++;

		// Send Data
		if(addr_size)
		{
		    //cout << grid->get_proc_addr_str(rank) << "  sending data_size " << data_size << " to " << grid->get_proc_addr_str(receiver) << endl;
		    //cout << rank << "  sending data_size " << data_size << " to " << receiver << endl;
		    MPI_Isend(blocks, data_size, MPI_DOUBLE, receiver, data_tag, comm, &send_req_data[count_data_sends]);
		    count_data_sends++;
		}
	    }
	    else // if receiver is this processor (sender)
	    { 
		self_buffers[num_self_sends] = blocks;
		self_addresses[num_self_sends] = block_addrs;
		self_data_sizes[num_self_sends]  = data_size;
		self_adr_sizes[num_self_sends] = addr_size;

		//if(rank==0){ printGetTiles(blocks, block_addrs, X->block_size, self_adr_sizes[num_self_sends]*sizeof(int)/X->dims, X->dims); }
		num_self_sends++;
	    }
	}
	delete[] matching_indices;
    }


// Receive data at instigator
    int Contraction::recv_at_instigator_rect (
	Tensor* &X, 
	int contr_dim, 
	int contr_idx, 
	double* &blocks,    // out
	int* &block_addrs,  // out
	int send_addr_count, 
	int send_data_count) 
    {
	 	//cout<<"Cdim "<<contr_dim<<" cid "<<contr_idx<<endl;
	// Find the processor dimension along which the contracting dim of tensor is distributed
	int pindex = X->index_dimension_map[contr_dim];
	

	// If this processor is not an instigator, 
	// then wait for all the sends from this processor to finish
	if(contr_idx % pgrid[pindex] != my_address[pindex])
	{
	    int* sender_grid = new int[num_procs*num_procs];
	    memset(sender_grid, 0, sizeof(int) * num_procs*num_procs);

	    MPI_Reduce(sender_grid, sender_grid_buffer, num_procs * num_procs, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	 
	    
	    if(rank == 0){
		for(int r = 0; r<num_procs; r++)
		{
		    for(int s = 0; s<num_procs; s++)
		    {
		
			if(sender_grid_buffer[r*num_procs + s] != receiver_grid_buffer[s*num_procs +r])
			{
			    if(sender_grid_buffer[r*num_procs + s])
				cout<<" Receiver : "<<grid->get_proc_addr_str(r)<<" is receiving from Sender : "<<grid->get_proc_addr_str(s)<<r<<"<-"<<s<<endl;
			    else
				cout<<" Sender : "<<grid->get_proc_addr_str(s)<<" is sending to Receiver : "<<grid->get_proc_addr_str(r)<<s<<"->"<<r<<endl;
			}   
			else
			{
			    //cout<<sender_grid_buffer[r*num_procs + s]<<" == "<<receiver_grid_buffer[s*num_procs +r]<<endl;
			}
		    
		    
		    
		    } 
		
		} 
		cout<<"Passed for Contr Dim "<<contr_dim<<" and contr indx "<<contr_idx<<endl;
	    }
	    
	    // Wait for address sends
	    MPI_Status* statuses_addr = new MPI_Status[send_addr_count];
	    MPI_Waitall(send_addr_count, send_req_addr, statuses_addr);
	 
	    // Wait for data blocks sends
	    MPI_Status* statuses_data = new MPI_Status[send_data_count];
	    MPI_Waitall(send_data_count, send_req_data, statuses_data);
	 
	    // Free memory
	    if(send_addr_count > 0)
	    {
	 	delete[] send_req_addr;
	 	delete[] statuses_addr;
	    }
	    if(send_data_count > 0)
	    {
	 	delete[] send_req_data;
	 	delete[] statuses_data;
	    }

	    return 0;
	}

	// Find all the processors to get the data from
	int** bouncers;
	int* senders;
	num_senders = X->get_rect_bouncers(contr_dim, contr_idx, bouncers);
	    
	grid->get_proc_ranks(num_senders, bouncers, senders);

	// Free memory allocated for bouncer addresses, now that ranks are stored
	for(int i=0; i<num_senders; i++) 
	{
	    delete[] bouncers[i];
	}   
	delete[] bouncers;

	//there mught be redundant ranks in senders. By storing in a set, we
	//only keep unique ranks
	set<int> unique_senders (senders, senders+num_senders);
	num_senders = unique_senders.size();
	senders = new int[num_senders];
	
	int* sender_grid = new int[num_procs*num_procs];
	memset(sender_grid, 0, sizeof(int) * num_procs*num_procs);
	//if(rank == 7) cout<<num_senders<<" Contr Dim "<<contr_dim<<"contr indx "<<contr_idx<<endl;
	for(set<int>::iterator it = unique_senders.begin(); it != unique_senders.end(); ++it)
	{
	    senders[distance(unique_senders.begin(),it)] = *it;

	    sender_grid[rank * num_procs + *it] = 1; 
	    
	    if(DEBUG_I && rank == 7 && contr_dim == 3 && contr_idx == 0)
	    {
		cout<<grid->get_proc_addr_str(*it);
		
	    }

	}

	MPI_Reduce(sender_grid, sender_grid_buffer, num_procs * num_procs, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	if(rank == 0){
	    for(int r = 0; r<num_procs; r++)
	    {
		for(int s = 0; s<num_procs; s++)
		{
		


		    if(sender_grid_buffer[r*num_procs + s] != receiver_grid_buffer[s*num_procs +r])
		    {
			    if(sender_grid_buffer[r*num_procs + s])
				cout<<" Receiver : "<<grid->get_proc_addr_str(r)<<" is receiving from Sender : "<<grid->get_proc_addr_str(s)<<r<<"<-"<<s<<endl;
			    else
				cout<<" Sender : "<<grid->get_proc_addr_str(s)<<" is sending to Receiver : "<<grid->get_proc_addr_str(r)<<s<<"->"<<r<<endl;
		    }   
		    
		    
		    
		} 
		
	    } 
	    cout<<"Passed for Contr Dim "<<contr_dim<<" and  contr idx "<<contr_idx<<endl;
	}

	current_senders = senders;
	// Sizes of addresses and blocks to be received
	int addr_size[num_senders];
	int data_size[num_senders];
	memset(addr_size, num_senders, 0);
	memset(data_size, num_senders, 0);
	
	// Temporarily hold data and addresses from each senders at locations pointed by these
	int** temp_block_addrs = new int*[num_senders];
	double** temp_blocks = new double*[num_senders];
	
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Status status;
	int tag;
	
	// Probe the address messages sent and find out the number of blocks that are coming
	for(int i=0; i<num_senders; i++) 
	{
	    if(senders[i] != rank)
	    {
		//if(rank == rank) cout << rank << "  Probing for msg from " << senders[i] << endl;
		MPI_Probe(senders[i], 1, MPI_COMM_WORLD, &status);
		MPI_Get_count(&status, MPI_INT, &addr_size[i]);
	
		// Allocate memory for blocks and addresses using this count
		if(addr_size[i] != MPI_UNDEFINED)
		{
		    data_size[i] = (addr_size[i]/X->dims)*X->block_size;
		    temp_block_addrs[i] = new int[addr_size[i]];
		    temp_blocks[i] = new double[data_size[i]];
		}
	    }
	}  
	
	// Receive
	for(int i=0; i<num_senders; i++) 
	{
	    MPI_Comm comm = MPI_COMM_WORLD;
	    if(senders[i] != rank)
	    {
		// Receive addresses
		if(addr_size[i] != MPI_UNDEFINED)
		{
		    tag = 1; 
		    //cout << grid->get_proc_addr_str(rank) << " receiving addr_size " << addr_size[i] << " to " << grid->get_proc_addr_str(senders[i]) << endl;
		    //cout << rank << "  receiving addr_size " << addr_size[i] << " from " << senders[i] << endl;
		    MPI_Recv(temp_block_addrs[i], addr_size[i], MPI_INT, senders[i], tag, comm, &status);
		}
	
		// Receive data blocks
		if(addr_size[i] != MPI_UNDEFINED & data_size[i] != 0)
		{
		    tag = 2;
		    //cout << grid->get_proc_addr_str(rank) << " receiving data_size " << data_size[i] << " to " <<grid->get_proc_addr_str(senders[i]) << endl;
		    //cout << rank << "  receiving data_size " << data_size[i] << " from " << senders[i] << endl;
		    MPI_Recv(temp_blocks[i], data_size[i], MPI_DOUBLE, senders[i], tag, comm, &status);
		}
	    }
	}  
	
	// Wait for sends from this processor to finish
	MPI_Status* statuses_addr = new MPI_Status[send_addr_count];
	MPI_Waitall(send_addr_count, send_req_addr, statuses_addr);
	delete[] statuses_addr;
	
	MPI_Status* statuses_data = new MPI_Status[send_data_count];
	MPI_Waitall(send_data_count, send_req_data, statuses_data);
	delete[] statuses_data;
	
	// Count the total size of blocks and addresses received at the instigator (this processor)
	int total_block_size = 0;
	int total_address_size = 0;
	
	for(int i = 0; i<num_senders; i++)
	{
	    if(senders[i] != rank)
	    {
		total_block_size+=data_size[i];    
		total_address_size+=addr_size[i];
	    }
	}
	
	// Merge the blocks that resided locally into the same set, removing duplicates.
	// Duplicates are found in different block sets because same blocks can be picked up
	// by multiple calls to get_tiles() at diagonals.
	double* out_blocks;
	int* out_addr;
	
	timer2 -= MPI_Wtime();
	int total_self_tiles = merge_self_send_blocks(X, out_blocks, out_addr);
	timer2 += MPI_Wtime();
	
	// Add these number of blocks to what we received
	total_block_size+= total_self_tiles * X->block_size;
	total_address_size+= total_self_tiles * X->dims;
	
	// Allocate memory for the total data collected at the instigator (this processor)
	blocks = new double[total_block_size];
	block_addrs = new int[total_address_size];
	
	// Temporary pointers for copying
	double* dest = blocks;
	int* dest_addr = block_addrs;
	
	timer3 -= MPI_Wtime();
	// Combine all the data recieved from remote processors
	for(int i = 0; i <num_senders; i++)
	{
	    if(senders[i] != rank)
	    {
		memcpy(dest, temp_blocks[i], data_size[i]*sizeof(double));
		memcpy(dest_addr, temp_block_addrs[i], addr_size[i]*sizeof(int));
		dest += data_size[i];
		dest_addr += addr_size[i];
	    }
	}
	
	
	// Add the local (self-send) blocks to the combined data
	memcpy(dest, out_blocks, total_self_tiles * X->block_size * sizeof(double));
	memcpy(dest_addr, out_addr, total_self_tiles * X->dims*sizeof(int));
	dest += total_self_tiles * X->block_size;
	dest_addr += total_self_tiles * X->dims;
	timer3 += MPI_Wtime();
	/*
	  if(rank==0) 
	  {
	  printGetTiles(blocks, block_addrs, X->block_size, total_block_size/X->block_size , X->dims);
	  cout<<total_block_size/X->block_size;
	  }
	*/
	
	// Free temp blocks
	for(int i=0; i<num_senders; i++) 
	{
	    if(senders[i] != rank  &&  addr_size[i] != MPI_UNDEFINED)
	    {
		delete[] temp_block_addrs[i]; 
		delete[] temp_blocks[i]; 
	    }
	}
	delete[] temp_block_addrs;
	delete[] temp_blocks;
	
	// Return total number of blocks collected at the instigator
	return total_block_size/X->block_size;
	//return 0;
    }


// Gets receiver addresses for current contracting dimension and index
    int Tensor::get_receivers_rect(
	int contr_dim,
	int contr_idx,
 	int** &receivers, //out
 	int* &matching_indices, //out
 	int &num_matches
 	)
    {
	// matching indices are the ones that are symmetric to the contraction indices
 	int* match = new int[dims];     
	int num_receivers = 0;
     
 	memset(match, 0, dims*sizeof(int));
     
	// Counts the number of processors that receive from this processor
	// and finds the indices that are symmetric to the contracting index
 	int count = 0;
	//for(int i =0; i < dims; i++){
	//	if (SG_index_map[i] == SG_index_map[contr_dim] &&
	//        SG_index_map[i] <=1 &&
 
 	for(int i =0; i < dims; i++)
 	{
	    // index i is in the same symmetry group as the contracting dimension
 	    if (((SG_index_map[i] == SG_index_map[contr_dim] &&
		  // Make sure that the index we are checking is a part of some symmetry group
 		  SG_index_map[i] != NON_SYM)  ||  
		 // Or ensure this is the contracting dimension (then self sends)
 		 i == contr_dim)  &&  
		// Make sure that this processor holds the contraction index
 		proc_addr[index_dimension_map[i]] == contr_idx % pgrid[index_dimension_map[i]])
 	    {
		// If there are multiple contraction indices belonging to the same symmetry group,
		// this condition maintains the order between these contraction indices
		// works only in CCSD when the size of symmetry group is 2
 		if(contr_dim < i && cntr_map[i] > 0 )
 		{
 		    break;
 		}
 		match[i] = 1;
 		count++;
 	    }
 	}

	if(DEBUG_T && rank == RRANK) cout<<"Number of Matches is "<<count<<endl;

 	num_matches = count;
 
	//stores the permutation used for altering the block
	//address due to symmetry
 	int** sender_to_receiver_map = new int*[count];
     
 
	//for each matching index, the following variables keeps track of
	//the processor id along each dimension holds processor id along
	//each dimension of the processor grid
	/////////////////////////////////////////////////////////////////
 	int*** proc_id_per_dim = new int**[count];
 
	//holds processor id along each dimension
 	int** size_per_dim = new int*[count];
	/////////////////////////////////////////////////////////////////


	//initialize
 	matching_indices = new int[count];
	//int num_receivers = 0;
 
 
	// Identify the receiver addresses
 	count = 0;
 	for(int i =0; i < dims; i++)
 	{
 	
	    // If a receiver is identified for this dimension
 	    if(match[i])
 	    {
		int receivers_for_matching_index = 1;
		
		sender_to_receiver_map[count] = new int[grid_dims];
		//initialize the map
		for(int j = 0; j<grid_dims; j++)
		{
		    sender_to_receiver_map[count][j] = j;
		}
		
		
 		sender_to_receiver_map[count][index_dimension_map[i]] = index_dimension_map[contr_dim];			  
 			
		// Get the physical address permutation map. It tells that
		//block dimension distributed along index_dimension_map[j]
		//will be distribution along a different dimension to obtain
		//the correct block address. The permutation map is used to
		//identify multiple receiers due to permutation specially
		//when the size of the phhysical grid along the intial
		//dimensions and the new dimension where the block will be
		//sent to are different
 		if(i<contr_dim)
 
 		    for(int j = i+1; j< contr_dim+1; j++)
 			sender_to_receiver_map[count][index_dimension_map[j]] = index_dimension_map[j-1];
 
 		if(i>contr_dim)
 

 		    for(int j = i-1; j> contr_dim-1; j--)
 			sender_to_receiver_map[count][index_dimension_map[j]] = index_dimension_map[j+1];
 

 
 		proc_id_per_dim[count] = new int*[grid_dims];
 		size_per_dim[count]= new int[grid_dims];
 
 		for(int l = 0; l < grid_dims; l++)
		{
		    // only a single contraction index is being sent so
		    // there is only one receiving processor along this
		    // dimension
		    //multiple receivers can exist along the dimension
		    //to which the current dimenison is getting mapped
		    //to. The upper limit is given by Lowest Common
		    //Multiple divided by the number of processors along
		    //this dimension
 		    if((sender_to_receiver_map[count][l] == index_dimension_map[contr_dim])){
 
 			int cdim  = index_dimension_map[contr_dim];
 			proc_id_per_dim[count][cdim] = new int[1];
 			proc_id_per_dim[count][cdim][0] =  contr_idx % pgrid[index_dimension_map[contr_dim]];
 			size_per_dim[count][cdim] = 1;
 
 		    }else{
 			int receive_dim = sender_to_receiver_map[count][l];
 			int num_receiver_per_dim = lcm(pgrid[l], pgrid[receive_dim]) / pgrid[l];
 
 			size_per_dim[count][receive_dim] = num_receiver_per_dim;
 			proc_id_per_dim[count][receive_dim] = new int[num_receiver_per_dim];
			if(DEBUG_T && rank == RRANK) cout<<"Number of per dimension  is "<<num_receiver_per_dim<<endl;
			
 			for(int c =0; c< num_receiver_per_dim; c++){

			    int block_addr_along_l = c*pgrid[l] + proc_addr[l];
			    //cout<<"Block Addr is "<<block_addr_along_l<<" tensor_range " << tensor_range[reverse_index_map[l]]<<"Reverse Index Map is "<<reverse_index_map[l]<<" and l is "<<l<<endl;
			    
			    //if the block address does not exist then no need
			    //to receive from the processor that holds this
			    //block. We assume that the dimensionality of the
			    //tensor is always smaller than 99
			    if(reverse_index_map[l] < 99 
			       && block_addr_along_l >= vgrid[reverse_index_map[l]] 
			       && SG_index_map[reverse_index_map[l]] != CONTRACTED )
			    {
				
				if(DEBUG_T && rank == RRANK) cout<<"reverse_index_map "<<reverse_index_map[l]<<endl;
				if(DEBUG_T && rank == RRANK) cout<<"vgrid[reverse_index_map[l]] "<<vgrid[reverse_index_map[l]]<<endl;
				if(DEBUG_T && rank == RRANK) cout<<"In for c : "<<c<<" count "<<count<<" l "<<l<<endl;
				if(DEBUG_T && rank == RRANK) cout<<"Block addr along l is "<<block_addr_along_l<<endl;
				num_receiver_per_dim = c;
				size_per_dim[count][receive_dim] = num_receiver_per_dim;
				break;
			    }
			    
 			    int addr = block_addr_along_l % pgrid[receive_dim];
 			    proc_id_per_dim[count][receive_dim][c] = addr;
			    
 			}

			receivers_for_matching_index *= num_receiver_per_dim;		     
 		    }
		    
		}

		num_receivers += receivers_for_matching_index;
		
		matching_indices[count] = i;
		if(DEBUG_T && rank == RRANK) cout<<"Number of receivers is "<<i<<endl;
		count++;
		
	    }

	}

	receivers = new int*[num_receivers];
	int offset = 0;
	int cur_dim = 0;
	int* current_address = new int[grid_dims];
	memset(current_address,0.0,sizeof(int)*grid_dims);
	
//fill up the address for the receiver
	for(int i = 0; i< count; i++)
	{
	    create_cross_product(proc_id_per_dim[i], size_per_dim[i], 
				 dims, cur_dim, 
				 offset, current_address,
				 receivers);	
	}
	
	delete[] current_address;
	delete[] match;

	return num_receivers;
    }
}

