#include "tensor.h"
namespace RRR{
using namespace std;

    //initializes the tensor, then reads the tiles from DLTC tensor and stores 
    //them in RRR tensors
    void Tensor::initialize_from_ga_tensor(DLTC::Tensor &t)
    {
	initialize();

	//if(rank ==0) cout<<"Block Size is "<<block_size<<endl;
	bool tcache;
	for(int i=0; i<num_actual_tiles; i++)
	{
	    int* addr = tile_address + i*dims;
	    vector<int> idv;
	    //if(rank==0) print_tile_addr(4,addr);
	    //creates the vector address of the tile to get
	    for(int d =0; d <dims; d++)
		idv.push_back(addr[d]);
	    
	    //puts the tile from the ga tensor to local buffer data
	    double* data = tensor_tiles + i*block_size;
	    t.getTile(data, block_size, idv, tcache);
 

	}
    }

    //copies data from RRR tensor to DLTC tensor
    void Tensor::accumulate_to_ga_tensor(DLTC::Tensor &t)
    {
	bool tcache;
	for(int i=0; i<num_actual_tiles; i++)
	{
	    int* addr = tile_address + i*dims;
	    vector<int> idv;

	    //creates the vector address of the tile to get
	    for(int d =0; d <dims; d++)
		idv.push_back(addr[d]);
	    
	    //puts the tile from the ga tensor to local buffer data
	    double* data = tensor_tiles + i*block_size;
	    t.accTile(data, block_size, idv, 1.0);
 
	}
    } 

}
