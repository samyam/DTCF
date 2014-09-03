#include "cost.h"


namespace RRR{
using namespace std;


cost::cost(Tensor* &Ainit, Tensor* &Binit, Tensor* &Cinit, Grid* &Ginit)
    {
        A=Ainit;
        B=Binit;
        C=Cinit;
	G=Ginit;
        dimA=A->dims;
        dimB=B->dims;
        dimC=C->dims;
        dimG=G->grid_dims;
	nprocs=G->nprocs;
	found=false;
	
    }


bool cost::bestCost(int* &idmapA, int* &idmapB, int* &idmapC, int* &ndimG, int* &npgrid)
{	
	
	ndimG = new int[1];
	bool changed = true;

	if(dimA==2 && dimB==2 && dimC==2)
		contract_2D_2D_2D(idmapA, idmapB, idmapC, ndimG, npgrid);
	else if(dimA==4 && dimB==4 && dimC==4)
		contract_4D_4D_4D(idmapA, idmapB, idmapC, ndimG, npgrid);
	else if(dimA==4 && dimB==2 && dimC==4)
		contract_4D_2D_4D(idmapA, idmapB, idmapC, ndimG, npgrid);
	else if(dimA==4 && dimB==4 && dimC==2)
		contract_4D_4D_2D(idmapA, idmapB, idmapC, ndimG, npgrid);		
	else if(dimA==2 && dimB==4 && dimC==4)
		contract_2D_4D_4D(idmapA, idmapB, idmapC, ndimG, npgrid);
	else if(dimA==4 && dimB==2 && dimC==2)
		contract_4D_2D_2D(idmapA, idmapB, idmapC, ndimG, npgrid);
	else if(dimA==2 && dimB==4 && dimC==2)
		contract_2D_4D_2D(idmapA, idmapB, idmapC, ndimG, npgrid);
	else if(dimA==2 && dimB==2 && dimC==4)
		contract_2D_2D_4D(idmapA, idmapB, idmapC, ndimG, npgrid);
	else changed=false;
 
	return changed;
}


void cost::contract_2D_2D_4D(int* &idmapA, int* &idmapB, int* &idmapC, int* &ndimG, int* &npgrid)
{
	idmapA = new int[2];
	idmapB=new int[2];
	idmapC=new int[4];
	ndimG[0]=4; 
	npgrid=new int[ndimG[0]];

	int size_A=1, size_B=1;
	for(int i=0;i<2;i++){ size_A*=A->tensor_size[i];	size_B*=B->tensor_size[i];}
	
	int fsize=2*ceil(sqrt(nprocs));
	int* temp = new int[fsize];
	int nfacts=0;
	bool done=false;

	for(int i=1; i<ceil(sqrt(nprocs));i++) if(nprocs%i==0) {temp[nfacts]=i; nfacts++; temp[nfacts]=nprocs/i; nfacts++;}

	for(int i=1;i<nfacts;i++) for(int j=1;j<nfacts;j++) 
	if(!done){ if(temp[i]*temp[i]*temp[j]*temp[j] == nprocs) { npgrid[0]=temp[i]; npgrid[1]=temp[i]; npgrid[2]=temp[j]; npgrid[3]=temp[j]; done=true;}}
	
	if(size_A>=size_B) 
	{
		idmapA[0]=2; idmapA[1]=3;
		idmapB[0]=0; idmapB[1]=1;
		idmapC[0]=2; idmapC[1]=3; idmapC[2]=0; idmapC[3]=1;
	}
	else
	{
		idmapB[0]=2; idmapB[1]=3;
		idmapA[0]=0; idmapA[1]=1;
		idmapC[0]=0; idmapC[1]=1; idmapC[2]=2; idmapC[3]=3;
	}	
}

void cost::contract_4D_2D_2D(int* &idmapA, int* &idmapB, int* &idmapC, int* &ndimG, int* &npgrid)
{	
	
	idmapA = new int[4];
	idmapB=new int[2];
	idmapC=new int[2];
	ndimG[0]=2; //Always 2D grid for these contractions
	npgrid=new int[ndimG[0]];
	//Assuming nprocs is a perfect square
	for(int i=0; i<ndimG[0]; i++)	npgrid[i]=sqrt(nprocs);

	int contA[2]={};
	int extA[2]={};
	int contB[2]={};
	
int ccnt=0, ecnt=0;
	for(int i=0; i<dimA; i++)
	    	{
        		for(int j=0; j<dimB; j++)
       		 	{
        	    		if(A->contr_dim_str[i].compare(B->contr_dim_str[j]) == 0)
          			{
               		 		// i in A and j in B are contracting indices
                			contA[ccnt]=i; contB[ccnt]=j;
					ccnt++;
						
            			}
        		}
    		}

for(int i=0;i<4;i++) {if(i!=contA[0] && i!=contA[1]) extA[ecnt]=i; ecnt++;}
ecnt=0;
	if((A->tensor_str.at(extA[0]) == 'c' || A->tensor_str.at(extA[0]) == 'd') && (A->tensor_str.at(extA[1]) == 'c' || A->tensor_str.at(extA[1]) == 'd'))
	{
		idmapA[extA[0]]=0; idmapA[extA[1]]=1; idmapA[contA[0]]=2; idmapA[contA[1]]=2;
		idmapB[contB[0]]=0; idmapB[contB[1]]=1;
		idmapC[0]=0; idmapC[1]=1;
	}

	else if((A->tensor_str.at(extA[0]) == 'a' && A->tensor_str.at(contA[0]) == 'A') || (A->tensor_str.at(extA[0]) == 'b' && A->tensor_str.at(contA[0]) == 'B'))
	{
 		idmapA[extA[0]]=0; idmapA[contA[0]]=1; idmapA[contA[1]]=2; idmapA[extA[1]]=2;
		idmapB[contB[0]]=0; idmapB[contB[1]]=1;
		idmapC[0]=0; idmapC[1]=1;
	}

	else if((A->tensor_str.at(extA[0]) == 'a' && A->tensor_str.at(contA[1]) == 'A') || (A->tensor_str.at(extA[0]) == 'b' && A->tensor_str.at(contA[1]) == 'B'))
	{
		idmapA[extA[0]]=0; idmapA[contA[1]]=1; idmapA[contA[0]]=2; idmapA[extA[1]]=2;
		idmapB[contB[0]]=1; idmapB[contB[1]]=0;
		idmapC[0]=0; idmapC[1]=1;
	}

	else if((A->tensor_str.at(extA[1]) == 'a' && A->tensor_str.at(contA[0]) == 'A') || (A->tensor_str.at(extA[1]) == 'b' && A->tensor_str.at(contA[0]) == 'B'))
	{
 		idmapA[extA[1]]=0; idmapA[contA[0]]=1; idmapA[contA[1]]=2; idmapA[extA[0]]=2;
		idmapB[contB[0]]=0; idmapB[contB[1]]=1;
		idmapC[0]=0; idmapC[1]=1;
	}
	else if((A->tensor_str.at(extA[1]) == 'a' && A->tensor_str.at(contA[1]) == 'A') || (A->tensor_str.at(extA[1]) == 'b' && A->tensor_str.at(contA[1]) == 'B'))
	{
		idmapA[extA[1]]=0; idmapA[contA[1]]=1; idmapA[contA[0]]=2; idmapA[extA[0]]=2;
		idmapB[contB[0]]=1; idmapB[contB[1]]=0;
		idmapC[0]=0; idmapC[1]=1;
	}
}

void cost::contract_2D_4D_2D(int* &idmapA, int* &idmapB, int* &idmapC, int* &ndimG, int* &npgrid)
{	
	
	idmapA = new int[2];
	idmapB=new int[4];
	idmapC=new int[2];
	ndimG[0]=2; //Always 2D grid for these contractions
	npgrid=new int[ndimG[0]];
	//Assuming nprocs is a perfect square
	for(int i=0; i<ndimG[0]; i++)	npgrid[i]=sqrt(nprocs);

	int contB[2]={};
	int extB[2]={};
	int contA[2]={};
	
int ccnt=0, ecnt=0;
	for(int i=0; i<dimA; i++)
	    	{
        		for(int j=0; j<dimB; j++)
       		 	{
        	    		if(A->contr_dim_str[i].compare(B->contr_dim_str[j]) == 0)
          			{
               		 		// i in A and j in B are contracting indices
                			contA[ccnt]=i; contB[ccnt]=j;
					ccnt++;
						
            			}
        		}
    		}

for(int i=0;i<4;i++) {if(i!=contB[0] && i!=contB[1]) extB[ecnt]=i; ecnt++;}

if((B->tensor_str.at(extB[0]) == 'c' || B->tensor_str.at(extB[0]) == 'd') && (B->tensor_str.at(extB[1]) == 'c' || B->tensor_str.at(extB[1]) == 'd'))
	{
		idmapB[extB[0]]=0; idmapB[extB[1]]=1; idmapB[contB[0]]=2; idmapB[contB[1]]=2;
		idmapA[contA[0]]=0; idmapA[contA[1]]=1;
		idmapC[0]=0; idmapC[1]=1;
	}

	else if((B->tensor_str.at(extB[0]) == 'a' && B->tensor_str.at(contB[0]) == 'A') || (B->tensor_str.at(extB[0]) == 'b' && B->tensor_str.at(contB[0]) == 'B'))
	{
 		idmapB[extB[0]]=0; idmapB[contB[0]]=1; idmapB[contB[1]]=2; idmapB[extB[1]]=2;
		idmapA[contA[0]]=0; idmapA[contA[1]]=1;
		idmapC[0]=0; idmapC[1]=1;
	}

	else if((B->tensor_str.at(extB[0]) == 'a' && B->tensor_str.at(contB[1]) == 'A') || (B->tensor_str.at(extB[0]) == 'b' && B->tensor_str.at(contB[1]) == 'B'))
	{
		idmapB[extB[0]]=0; idmapB[contB[1]]=1; idmapB[contB[0]]=2; idmapB[extB[1]]=2;
		idmapA[contA[0]]=1; idmapA[contA[1]]=0;
		idmapC[0]=0; idmapC[1]=1;
	}

	else if((B->tensor_str.at(extB[1]) == 'a' && B->tensor_str.at(contB[0]) == 'A') || (B->tensor_str.at(extB[1]) == 'b' && B->tensor_str.at(contB[0]) == 'B'))
	{
 		idmapB[extB[1]]=0; idmapB[contB[0]]=1; idmapB[contB[1]]=2; idmapB[extB[0]]=2;
		idmapA[contA[0]]=0; idmapA[contA[1]]=1;
		idmapC[0]=0; idmapC[1]=1;
	}
	else if((B->tensor_str.at(extB[1]) == 'a' && B->tensor_str.at(contB[1]) == 'A') || (B->tensor_str.at(extB[1]) == 'b' && B->tensor_str.at(contB[1]) == 'B'))
	{
		idmapB[extB[1]]=0; idmapB[contB[1]]=1; idmapB[contB[0]]=2; idmapB[extB[0]]=2;
		idmapA[contA[0]]=1; idmapA[contA[1]]=0;
		idmapC[0]=0; idmapC[1]=1;
	}

}

void cost::contract_2D_2D_2D(int* &idmapA, int* &idmapB, int* &idmapC, int* &ndimG, int* &npgrid)
{	
	
	idmapA = new int[2];
	idmapB=new int[2];
	idmapC=new int[2];
	ndimG[0]=2; //Always 2D grid for these contractions
	npgrid=new int[ndimG[0]];
	//Assuming nprocs is a perfect square
	for(int i=0; i<ndimG[0]; i++)	npgrid[i]=sqrt(nprocs);

	//Find where to distribute the contraction indices
	for(int i=0; i<dimA; i++)
	    	{
        		for(int j=0; j<dimB; j++)
       		 	{
        	    		if(A->contr_dim_str[i].compare(B->contr_dim_str[j]) == 0)
          			{
               		 		// i in A and j in B are contracting indices
                			idmapA[i]=0;	idmapA[1-i]=1;	
					idmapB[j]=1;	idmapB[1-j]=0;
					idmapC[0]=1;	idmapC[1]=0;	
            			}
        		}
    		}
}

void cost::contract_4D_4D_4D(int* &idmapA, int* &idmapB, int* &idmapC, int* &ndimG, int* &npgrid)
{	
	
	ndimG[0]=4; //Always 4D grid for these contractions
	npgrid=new int[ndimG[0]];
	
	int fsize=2*ceil(sqrt(nprocs));
	int* temp = new int[fsize];
	int nfacts=0;
	

	for(int i=1; i<ceil(sqrt(nprocs));i++) if(nprocs%i==0) {temp[nfacts]=i; nfacts++; temp[nfacts]=nprocs/i; nfacts++;}
	
	int *facts = new int[4*nfacts];
	for(int i=0;i<nfacts;i++){for(int j=0; j<4; j++)facts[i*4+j]=temp[i];}
	
	int* curgrid = new int[4];
	found=false;
	
	
	Grid_Comb_4D(facts,curgrid,0,4*nfacts-1,0,4, idmapA, idmapB, idmapC, npgrid);
	
}


void cost::Grid_Comb_4D(int arr[], int data[], int start, int end, int index, int r,int* &idmapA, int* &idmapB, int* &idmapC, int* &npgrid)
{	
	if(found)return;
	if(index==r)
	{	
		int temp=1;
		for (int i=0;i<4;i++) temp=temp*data[i];
		if(temp==nprocs && data[0]==data[1] && data[2]==data[3]) //Additional conditions to ensure symmetric dimensions are in same sized dimensions
		{	
			best_Cost_GGrid_4D(data, idmapA, idmapB, idmapC);
				     	
			for(int i=0;i<4;i++) npgrid[i]=data[i];
			found=true;
		}
	return;
	}

	for (int i=start; i<=end && end-i+1 >= r-index; i++)
    	{
        	data[index] = arr[i];
        	Grid_Comb_4D(arr, data, i+1, end, index+1, r, idmapA,idmapB,idmapC,npgrid);
  
        	// Remove duplicates
        	while (arr[i] == arr[i+1])
             	i++;
    	}

}


void cost::best_Cost_GGrid_4D(int curGrid[], int* &curidmapA, int* &curidmapB, int* &curidmapC)
{	
	curidmapA = new int[4];
	curidmapB=new int[4];
	curidmapC=new int[4];
	int contA[2]={};
	int extA[2]={};
	int contB[2]={};
	int extB[2]={};

int ccnt=0, ecnt=0;
	for(int i=0; i<dimA; i++)
	    	{
        		for(int j=0; j<dimB; j++)
       		 	{
        	    		if(A->contr_dim_str[i].compare(B->contr_dim_str[j]) == 0)
          			{
               		 		// i in A and j in B are contracting indices
                			contA[ccnt]=i; contB[ccnt]=j;
					ccnt++;
						
            			}
        		}
    		}

for(int i=0;i<4;i++) {if(i!=contA[0] && i!=contA[1]) extA[ecnt]=i; ecnt++;}
ecnt=0;
for(int i=0;i<4;i++) {if(i!=contB[0] && i!=contB[1]) extB[ecnt]=i; ecnt++;}

int lar=1;
if(curGrid[3] > curGrid[1]) lar=3;

int size_A=1, size_B=1;
for(int i=0;i<4;i++){ size_A*=A->tensor_size[i];	size_B*=B->tensor_size[i];}

if(size_A > size_B)
{	cout<<"1"<<endl;
	
	if(A->tensor_size[extA[0]] >= A->tensor_size[extA[1]])
	{cout<<"2"<<endl;
		curidmapA[extA[0]]=lar;
		if(A->tensor_str.at(extA[0]) == 'c' || A->tensor_str.at(extA[0]) == 'd' || A->tensor_str.at(extA[0]) == A->tensor_str.at(extA[1]))
			{ cout<<"3"<<endl;
			if(A->tensor_str.at(extA[1]) == 'c' || A->tensor_str.at(extA[1]) == 'd')
				{cout<<"4"<<endl;
				curidmapA[extA[1]]=lar-1; curidmapA[contA[0]]=4-lar; curidmapA[contA[1]] = 4-lar-1;
			  	curidmapB[extB[0]]=4-lar; curidmapB[extB[1]]=4-lar-1; curidmapB[contB[0]]=lar; curidmapB[contB[1]]=lar-1;
				curidmapC[0]=lar; curidmapC[1]=lar-1; curidmapC[2]=4-lar; curidmapC[3]=4-lar-1; 
				}
			else if((A->tensor_str.at(extA[1]) == 'a' && A->tensor_str.at(contA[0]) == 'A') || (A->tensor_str.at(extA[1]) == 'b' && A->tensor_str.at(contA[0]) == 'B'))
				{cout<<"1"<<endl;
				curidmapA[extA[1]]=4-lar; curidmapA[contA[0]]=4-lar-1; curidmapA[contA[1]] =lar-1;
				  curidmapB[extB[0]]=4-lar-1; curidmapB[extB[1]]=lar-1; curidmapB[contB[0]]=4-lar; curidmapB[contB[1]]=lar;
				curidmapC[0]=lar; curidmapC[1]=4-lar; curidmapC[2]=4-lar-1; curidmapC[3]=lar-1; 
				}
			else if((A->tensor_str.at(extA[1]) == 'a' && A->tensor_str.at(contA[1]) == 'A') || (A->tensor_str.at(extA[1]) == 'b' && A->tensor_str.at(contA[1]) == 'B'))
				{cout<<"1"<<endl;
				curidmapA[extA[1]]=4-lar; curidmapA[contA[1]]=4-lar-1; curidmapA[contA[0]] =lar-1;
				  curidmapB[extB[0]]=4-lar-1; curidmapB[extB[1]]=lar-1; curidmapB[contB[0]]=4-lar; curidmapB[contB[1]]=lar;
				curidmapC[0]=lar; curidmapC[1]=4-lar; curidmapC[2]=4-lar-1; curidmapC[3]=lar-1; 
				}
			}
		else if((A->tensor_str.at(extA[0]) == 'a' && A->tensor_str.at(contA[0]) == 'A') || (A->tensor_str.at(extA[0]) == 'b' && A->tensor_str.at(contA[0]) == 'B'))
			{ cout<<"1"<<endl;
				curidmapA[contA[0]]=lar-1; curidmapA[contA[1]]=4-lar; curidmapA[extA[1]]=4-lar-1;
			  curidmapB[extB[0]]=4-lar; curidmapB[extB[1]]=lar-1; curidmapB[contB[0]]=4-lar-1; curidmapB[contB[1]]=lar;
				curidmapC[0]=lar; curidmapC[1]=4-lar-1; curidmapC[2]=4-lar; curidmapC[3]=lar-1; 
			}
		else if((A->tensor_str.at(extA[0]) == 'a' && A->tensor_str.at(contA[1]) == 'A') || (A->tensor_str.at(extA[0]) == 'b' && A->tensor_str.at(contA[1]) == 'B')) 
			{ cout<<"1"<<endl;
			curidmapA[contA[1]]=lar-1; curidmapA[contA[0]]=4-lar; curidmapA[extA[1]]=4-lar-1;
			  curidmapB[extB[0]]=4-lar; curidmapB[extB[1]]=lar-1; curidmapB[contB[0]]=4-lar-1; curidmapB[contB[1]]=lar;
				curidmapC[0]=lar; curidmapC[1]=4-lar-1; curidmapC[2]=4-lar; curidmapC[3]=lar-1; 
			}
	}
	else
	{cout<<"11"<<endl;
		curidmapA[extA[1]]=lar;
		if(A->tensor_str.at(extA[1]) == 'c' || A->tensor_str.at(extA[1]) == 'd' || A->tensor_str.at(extA[0]) == A->tensor_str.at(extA[1]))
			{ 
			if(A->tensor_str.at(extA[0]) == 'c' || A->tensor_str.at(extA[0]) == 'd')
				{curidmapA[extA[0]]=lar-1; curidmapA[contA[0]]=4-lar; curidmapA[contA[1]] = 4-lar-1;
			  	curidmapB[extB[0]]=4-lar; curidmapB[extB[1]]=4-lar-1; curidmapB[contB[0]]=lar; curidmapB[contB[1]]=lar-1;
				curidmapC[0]=lar-1; curidmapC[1]=lar; curidmapC[2]=4-lar; curidmapC[3]=4-lar-1; 
				}
			else if((A->tensor_str.at(extA[0]) == 'a' && A->tensor_str.at(contA[0]) == 'A') || (A->tensor_str.at(extA[0]) == 'b' && A->tensor_str.at(contA[0]) == 'B'))
				{curidmapA[extA[0]]=4-lar; curidmapA[contA[0]]=4-lar-1; curidmapA[contA[1]] =lar-1;
				  curidmapB[extB[0]]=4-lar-1; curidmapB[extB[1]]=lar-1; curidmapB[contB[0]]=4-lar; curidmapB[contB[1]]=lar;
				curidmapC[0]=4-lar; curidmapC[1]=lar; curidmapC[2]=4-lar-1; curidmapC[3]=lar-1; 
				}
			else if((A->tensor_str.at(extA[0]) == 'a' && A->tensor_str.at(contA[1]) == 'A') || (A->tensor_str.at(extA[0]) == 'b' && A->tensor_str.at(contA[1]) == 'B'))
				{curidmapA[extA[0]]=4-lar; curidmapA[contA[1]]=4-lar-1; curidmapA[contA[0]] =lar-1;
				  curidmapB[extB[0]]=4-lar-1; curidmapB[extB[1]]=lar-1; curidmapB[contB[0]]=4-lar; curidmapB[contB[1]]=lar;
				curidmapC[0]=4-lar; curidmapC[1]=lar; curidmapC[2]=4-lar-1; curidmapC[3]=lar-1; 
				}
			}
		else if((A->tensor_str.at(extA[1]) == 'a' && A->tensor_str.at(contA[0]) == 'A') || (A->tensor_str.at(extA[1]) == 'b' && A->tensor_str.at(contA[0]) == 'B'))
			{ curidmapA[contA[0]]=lar-1; curidmapA[contA[1]]=4-lar; curidmapA[extA[0]]=4-lar-1;
			  curidmapB[extB[0]]=4-lar; curidmapB[extB[1]]=lar-1; curidmapB[contB[0]]=4-lar-1; curidmapB[contB[1]]=lar;
				curidmapC[0]=4-lar-1; curidmapC[1]=lar; curidmapC[2]=4-lar; curidmapC[3]=lar-1; 
			}
		else if((A->tensor_str.at(extA[1]) == 'a' && A->tensor_str.at(contA[1]) == 'A') || (A->tensor_str.at(extA[1]) == 'b' && A->tensor_str.at(contA[1]) == 'B')) 
			{ curidmapA[contA[1]]=lar-1; curidmapA[contA[0]]=4-lar; curidmapA[extA[0]]=4-lar-1;
			  curidmapB[extB[0]]=4-lar; curidmapB[extB[1]]=lar-1; curidmapB[contB[0]]=4-lar-1; curidmapB[contB[1]]=lar;
				curidmapC[0]=4-lar-1; curidmapC[1]=lar; curidmapC[2]=4-lar; curidmapC[3]=lar-1; 
			}
	}	
}	
else
{
	if(B->tensor_size[extB[0]] >= B->tensor_size[extB[1]])
	{cout<<"133"<<endl;
		curidmapB[extB[0]]=lar;
		if(B->tensor_str.at(extB[0]) == 'c' || B->tensor_str.at(extB[0]) == 'd' || B->tensor_str.at(extB[0]) == B->tensor_str.at(extB[1]))
			{ cout<<"1313"<<endl;
			if(B->tensor_str.at(extB[1]) == 'c' || B->tensor_str.at(extB[1]) == 'd')
				{curidmapB[extB[1]]=lar-1; curidmapB[contB[0]]=4-lar; curidmapB[contB[1]] = 4-lar-1;
			  	curidmapA[extA[0]]=4-lar; curidmapA[extA[1]]=4-lar-1; curidmapA[contA[0]]=lar; curidmapA[contA[1]]=lar-1;
				curidmapC[0]=4-lar; curidmapC[1]=4-lar-1; curidmapC[2]=lar; curidmapC[3]=lar-1; 
			}
			else if((B->tensor_str.at(extB[1]) == 'a' && B->tensor_str.at(contB[0]) == 'A') || (B->tensor_str.at(extB[1]) == 'b' && B->tensor_str.at(contB[0]) == 'B'))
				{cout<<"1233"<<endl;
				curidmapB[extB[1]]=4-lar; curidmapB[contB[0]]=4-lar-1; curidmapB[contB[1]] =lar-1;
				  curidmapA[extA[0]]=4-lar-1; curidmapA[extA[1]]=lar-1; curidmapA[contA[0]]=4-lar; curidmapA[contA[1]]=lar;
				curidmapC[0]=4-lar-1; curidmapC[1]=lar-1; curidmapC[2]=lar; curidmapC[3]=4-lar; 
				}
			else if((B->tensor_str.at(extB[1]) == 'a' && B->tensor_str.at(contB[1]) == 'A') || (B->tensor_str.at(extB[1]) == 'b' && B->tensor_str.at(contB[1]) == 'B'))
				{cout<<"1333"<<endl;
				curidmapB[extB[1]]=4-lar; curidmapB[contB[1]]=4-lar-1; curidmapB[contB[0]] =lar-1;
				  curidmapA[extA[0]]=4-lar-1; curidmapA[extA[1]]=lar-1; curidmapA[contA[0]]=4-lar; curidmapA[contA[1]]=lar;
				curidmapC[0]=4-lar-1; curidmapC[1]=lar-1; curidmapC[2]=lar; curidmapC[3]=4-lar; 
				}
			}
		else if((B->tensor_str.at(extB[0]) == 'a' && B->tensor_str.at(contB[0]) == 'A') || (B->tensor_str.at(extB[0]) == 'b' && B->tensor_str.at(contB[0]) == 'B'))
			{ cout<<"1343"<<endl;
			curidmapB[contB[0]]=lar-1; curidmapB[contB[1]]=4-lar; curidmapB[extB[1]]=4-lar-1;
			  curidmapA[extA[0]]=4-lar; curidmapA[extA[1]]=lar-1; curidmapA[contA[0]]=4-lar-1; curidmapA[contA[1]]=lar;
				curidmapC[0]=4-lar; curidmapC[1]=lar-1; curidmapC[2]=lar; curidmapC[3]=4-lar-1; 
			}
		else if((B->tensor_str.at(extB[0]) == 'a' && B->tensor_str.at(contB[1]) == 'A') || (B->tensor_str.at(extB[0]) == 'b' && B->tensor_str.at(contB[1]) == 'B'))
			{ cout<<"1353"<<endl;
			curidmapB[contB[1]]=lar-1; curidmapB[contB[0]]=4-lar; curidmapB[extB[1]]=4-lar-1;
			  curidmapA[extA[0]]=4-lar; curidmapA[extA[1]]=lar-1; curidmapA[contA[0]]=4-lar-1; curidmapA[contA[1]]=lar;
				curidmapC[0]=4-lar; curidmapC[1]=lar-1; curidmapC[2]=lar; curidmapC[3]=4-lar-1; 
			}
	}
	else
	{cout<<"155"<<endl;
		curidmapB[extB[1]]=lar;
		if(B->tensor_str.at(extB[1]) == 'c' || B->tensor_str.at(extB[1]) == 'd' || B->tensor_str.at(extB[0]) == B->tensor_str.at(extB[1]))
			{ 
			if(B->tensor_str.at(extB[0]) == 'c' || B->tensor_str.at(extB[0]) == 'd')
				{curidmapB[extB[0]]=lar-1; curidmapB[contB[0]]=4-lar; curidmapB[contB[1]] = 4-lar-1;
			  	curidmapA[extA[0]]=4-lar; curidmapA[extA[1]]=4-lar-1; curidmapA[contA[0]]=lar; curidmapA[contA[1]]=lar-1;
				curidmapC[0]=4-lar; curidmapC[1]=4-lar-1; curidmapC[2]=lar-1; curidmapC[3]=lar; 
				}
			else if((B->tensor_str.at(extB[0]) == 'a' && B->tensor_str.at(contB[0]) == 'A') || (B->tensor_str.at(extB[0]) == 'b' && B->tensor_str.at(contB[0]) == 'B'))
				{curidmapB[extB[0]]=4-lar; curidmapB[contB[0]]=4-lar-1; curidmapB[contB[1]] =lar-1;
				  curidmapA[extA[0]]=4-lar-1; curidmapA[extA[1]]=lar-1; curidmapA[contA[0]]=4-lar; curidmapA[contA[1]]=lar;
				curidmapC[0]=4-lar-1; curidmapC[1]=lar-1; curidmapC[2]=4-lar; curidmapC[3]=lar; 
				}
			else if((B->tensor_str.at(extB[0]) == 'a' && B->tensor_str.at(contB[1]) == 'A') || (B->tensor_str.at(extB[0]) == 'b' && B->tensor_str.at(contB[1]) == 'B'))
				{curidmapB[extB[0]]=4-lar; curidmapB[contB[1]]=4-lar-1; curidmapB[contB[0]] =lar-1;
				  curidmapA[extA[0]]=4-lar-1; curidmapA[extA[1]]=lar-1; curidmapA[contA[0]]=4-lar; curidmapA[contA[1]]=lar;
				curidmapC[0]=4-lar-1; curidmapC[1]=lar-1; curidmapC[2]=4-lar; curidmapC[3]=lar; 
				}
			}
		else if((B->tensor_str.at(extB[1]) == 'a' && B->tensor_str.at(contB[0]) == 'A') || (B->tensor_str.at(extB[1]) == 'b' && B->tensor_str.at(contB[0]) == 'B'))
			{ curidmapB[contB[0]]=lar-1; curidmapB[contB[1]]=4-lar; curidmapB[extB[0]]=4-lar-1;
			  curidmapA[extA[0]]=4-lar; curidmapA[extA[1]]=lar-1; curidmapA[contA[0]]=4-lar-1; curidmapA[contA[1]]=lar;
				curidmapC[0]=4-lar; curidmapC[1]=lar-1; curidmapC[2]=4-lar-1; curidmapC[3]=lar; 
			}
		else if((B->tensor_str.at(extB[1]) == 'a' && B->tensor_str.at(contB[1]) == 'A') || (B->tensor_str.at(extB[1]) == 'b' && B->tensor_str.at(contB[1]) == 'B')) 
			{ curidmapB[contB[1]]=lar-1; curidmapB[contB[0]]=4-lar; curidmapB[extB[0]]=4-lar-1;
			  curidmapA[extA[0]]=4-lar; curidmapA[extA[1]]=lar-1; curidmapA[contA[0]]=4-lar-1; curidmapA[contA[1]]=lar;
				curidmapC[0]=4-lar; curidmapC[1]=lar-1; curidmapC[2]=4-lar-1; curidmapC[3]=lar; 
			}
	}

}	
}




void cost::contract_4D_2D_4D(int* &idmapA, int* &idmapB, int* &idmapC, int* &ndimG, int* &npgrid)
{	
	
	ndimG[0]=4; //Always 4D grid for these contractions
	npgrid=new int[ndimG[0]];
	
	int fsize=2*ceil(sqrt(nprocs));
	int* temp = new int[fsize];
	int nfacts=0;

	for(int i=1; i<ceil(sqrt(nprocs));i++) if(nprocs%i==0) {temp[nfacts]=i; nfacts++; temp[nfacts]=nprocs/i; nfacts++;}
	int *facts = new int[4*nfacts];
	for(int i=0;i<nfacts;i++){for(int j=0; j<4; j++)facts[i*4+j]=temp[i];}
	
	int* curgrid = new int[4];
	found=false;
	Grid_Comb_4Db(facts,curgrid,0,4*nfacts-1,0,4, idmapA, idmapB, idmapC, npgrid);
}



void cost::Grid_Comb_4Db(int arr[], int data[], int start, int end, int index, int r,int* &idmapA, int* &idmapB, int* &idmapC, int* &npgrid)
{	
	if(found)return;
	if(index==r)
	{
		int temp=1;
		for (int i=0;i<4;i++) temp=temp*data[i];
		if(temp==nprocs && data[0]==data[1] && data[2]==data[3]) //Additional conditions to ensure symmetric dimensions are in same sized dimensions
		{	
			best_Cost_GGrid_4Db(data, idmapA, idmapB, idmapC);	     	
			for(int i=0;i<4;i++) npgrid[i]=data[i];
			found=true;
		}
	return;
	}

	for (int i=start; i<=end && end-i+1 >= r-index; i++)
    	{
        	data[index] = arr[i];
        	Grid_Comb_4Db(arr, data, i+1, end, index+1, r, idmapA, idmapB, idmapC, npgrid);
  
        	// Remove duplicates
        	while (arr[i] == arr[i+1])
             	i++;
    	}

}


void cost::best_Cost_GGrid_4Db(int* &curGrid, int* &curidmapA, int* &curidmapB, int* &curidmapC)
{	
	
	curidmapA = new int[4];
	curidmapB=new int[2];
	curidmapC=new int[4];
	int contA[1]={};
	int extA[3]={};
	int contB[1]={};
	int extB[1]={};

	int ccnt=0, ecnt=0;
	for(int i=0; i<dimA; i++)
	    	{
        		for(int j=0; j<dimB; j++)
       		 	{
        	    		if(A->contr_dim_str[i].compare(B->contr_dim_str[j]) == 0)
          			{
               		 		// i in A and j in B are contracting indices
                			contA[ccnt]=i; contB[ccnt]=j;
					ccnt++;
						
            			}
        		}
    		}

	for(int i=0;i<4;i++) {if(i!=contA[0]) extA[ecnt]=i; ecnt++;}
	ecnt=0;
	for(int i=0;i<4;i++) {if(i!=contB[0]) extB[ecnt]=i; ecnt++;}

	int lar=1;
	if(curGrid[3] > curGrid[1]) lar=3;

	
	
	int temp;
	for(int i=1; i<3; i++){if(A->tensor_size[extA[i]] > A->tensor_size[extA[0]] ) {temp = extA[0]; extA[0]=extA[i]; extA[i]=temp;}}
	
	
	if(A->tensor_size[extA[0]] >= A->tensor_size[extA[1]] && A->tensor_size[extA[0]] >= A->tensor_size[extA[2]])
	{
		curidmapA[extA[0]]=lar;
		if(A->tensor_str.at(extA[0]) == 'c' || A->tensor_str.at(extA[0]) == 'd')
		{ 
			if(A->tensor_size[extA[1]] >= A->tensor_size[extA[2]])
			{
				if(A->tensor_str.at(extA[1]) == 'c' || A->tensor_str.at(extA[1]) == 'd')
				{
					curidmapA[extA[1]]=lar-1; curidmapA[extA[2]]=4-lar; curidmapA[contA[0]]=4-lar-1;
					curidmapB[contB[0]]=4-lar; curidmapB[extB[0]]=4-lar-1;
					curidmapC[0]=lar; curidmapC[1]=lar-1; curidmapC[2]=4-lar; curidmapC[3]=4-lar-1;
				}
				else if(A->tensor_str.at(extA[1]) == A->tensor_str.at(extA[2]))
				{
					curidmapA[extA[1]]=4-lar; curidmapA[extA[2]]=4-lar-1; curidmapA[contA[0]]=lar-1;
					curidmapB[contB[0]]=lar; curidmapB[extB[0]]=lar-1;
					curidmapC[0]=lar; curidmapC[1]=4-lar; curidmapC[2]=4-lar-1; curidmapC[3]=lar-1;	 
				}
				else
				{
					curidmapA[extA[1]]=4-lar; curidmapA[extA[2]]=lar-1; curidmapA[contA[0]]=4-lar-1;
					curidmapB[contB[0]]=4-lar; curidmapB[extB[0]]=4-lar-1;
					curidmapC[0]=lar; curidmapC[1]=4-lar; curidmapC[2]=lar-1; curidmapC[3]=4-lar-1;
				}
			}
			else if(A->tensor_size[extA[2]] >= A->tensor_size[extA[1]])
			{
				if(A->tensor_str.at(extA[2]) == 'c' || A->tensor_str.at(extA[2]) == 'd')
				{
					curidmapA[extA[2]]=lar-1; curidmapA[extA[1]]=4-lar; curidmapA[contA[0]]=4-lar-1;
					curidmapB[contB[0]]=4-lar; curidmapB[extB[0]]=4-lar-1;
					curidmapC[0]=lar; curidmapC[2]=lar-1; curidmapC[1]=4-lar; curidmapC[3]=4-lar-1;
				}
				else if(A->tensor_str.at(extA[2]) == A->tensor_str.at(extA[1]))
				{
					curidmapA[extA[2]]=4-lar; curidmapA[extA[1]]=4-lar-1; curidmapA[contA[0]]=lar-1;
					curidmapB[contB[0]]=lar; curidmapB[extB[0]]=lar-1;
					curidmapC[0]=lar; curidmapC[2]=4-lar; curidmapC[1]=4-lar-1; curidmapC[3]=lar-1;	 
				}
				else
				{
					curidmapA[extA[2]]=4-lar; curidmapA[extA[1]]=lar-1; curidmapA[contA[0]]=4-lar-1;
					curidmapB[contB[0]]=4-lar; curidmapB[extB[0]]=4-lar-1;
					curidmapC[0]=lar; curidmapC[2]=4-lar; curidmapC[1]=lar-1; curidmapC[3]=4-lar-1;
				}
			}
		}
		else if(A->tensor_str.at(extA[0])  ==  A->tensor_str.at(extA[1]))
		{
			curidmapA[extA[1]]=lar-1; curidmapA[extA[2]]=4-lar; curidmapA[contA[0]]=4-lar-1;
			curidmapB[contB[0]]=4-lar; curidmapB[extB[0]]=4-lar-1;
			curidmapC[0]=lar; curidmapC[1]=lar-1; curidmapC[2]=4-lar; curidmapC[3]=4-lar-1;
		}
		else if(A->tensor_str.at(extA[0])  ==  A->tensor_str.at(extA[2]))
		{
			curidmapA[extA[2]]=lar-1; curidmapA[extA[1]]=4-lar; curidmapA[contA[0]]=4-lar-1;
			curidmapB[contB[0]]=4-lar; curidmapB[extB[0]]=4-lar-1;
			curidmapC[0]=lar; curidmapC[2]=lar-1; curidmapC[1]=4-lar; curidmapC[3]=4-lar-1;
		}
		else
		{
			curidmapA[contA[0]]=lar-1; curidmapA[extA[1]]=4-lar; curidmapA[extA[2]]=4-lar-1;
			curidmapB[contB[0]]=lar; curidmapB[extB[0]]=lar-1;
			curidmapC[0]=lar; curidmapC[2]=4-lar; curidmapC[1]=4-lar-1; curidmapC[3]=lar-1;
		}	
	}

}	





void cost::contract_4D_4D_2D(int* &idmapA, int* &idmapB, int* &idmapC, int* &ndimG, int* &npgrid)
{	
	
	ndimG[0]=4; //Always 4D grid for these contractions
	npgrid=new int[ndimG[0]];
	
	int fsize=2*ceil(sqrt(nprocs));
	int* temp = new int[fsize];
	int nfacts=0;

	for(int i=1; i<ceil(sqrt(nprocs));i++) if(nprocs%i==0) {temp[nfacts]=i; nfacts++; temp[nfacts]=nprocs/i; nfacts++;}
	int *facts = new int[4*nfacts];
	for(int i=0;i<nfacts;i++){for(int j=0; j<4; j++)facts[i*4+j]=temp[i];}
	
	int* curgrid = new int[4];
	found=false;
	Grid_Comb_4Dc(facts,curgrid,0,4*nfacts-1,0,4, idmapA, idmapB, idmapC, npgrid);
}


void cost::Grid_Comb_4Dc(int arr[], int data[], int start, int end, int index, int r,int* &idmapA, int* &idmapB, int* &idmapC, int* &npgrid)
{	
	if(found)return;
	if(index==r)
	{
		int temp=1;
		for (int i=0;i<4;i++) temp=temp*data[i];
		if(temp==nprocs && data[0]==data[1] && data[2]==data[3]) //Additional conditions to ensure symmetric dimensions are in same sized dimensions
		{	
			best_Cost_GGrid_4Dc(data, idmapA, idmapB, idmapC);	     	
			for(int i=0;i<4;i++) npgrid[i]=data[i];
			found=true;
		}
	return;
	}

	for (int i=start; i<=end && end-i+1 >= r-index; i++)
    	{
        	data[index] = arr[i];
        	Grid_Comb_4Dc(arr, data, i+1, end, index+1, r, idmapA, idmapB, idmapC, npgrid);
  
        	// Remove duplicates
        	while (arr[i] == arr[i+1])
             	i++;
    	}

}


void cost::best_Cost_GGrid_4Dc(int* &curGrid, int* &curidmapA, int* &curidmapB, int* &curidmapC)
{	
	
	curidmapA = new int[4];
	curidmapB=new int[4];
	curidmapC=new int[2];
	int contA[3]={};
	int extA[1]={};
	int contB[3]={};
	int extB[1]={};

	int ccnt=0, ecnt=0;
	for(int i=0; i<dimA; i++)
	    	{
        		for(int j=0; j<dimB; j++)
       		 	{
        	    		if(A->contr_dim_str[i].compare(B->contr_dim_str[j]) == 0)
          			{
               		 		// i in A and j in B are contracting indices
                			contA[ccnt]=i; contB[ccnt]=j;
					ccnt++;
						
            			}
        		}
    		}

	for(int i=0;i<4;i++) {if(i!=contA[0]) extA[ecnt]=i; ecnt++;}
	ecnt=0;
	for(int i=0;i<4;i++) {if(i!=contB[0]) extB[ecnt]=i; ecnt++;}

	int lar=1;
	if(curGrid[3] > curGrid[1]) lar=3;

	int size_A=1, size_B=1;
	for(int i=0;i<4;i++){ size_A*=A->tensor_size[i];	size_B*=B->tensor_size[i];}
	
	bool symA1f=false,symA2f=false,symB1f=false,symB2f=false;
	
	
	if(A->tensor_str.at(extA[0]) == 'a' || A->tensor_str.at(contA[0]) == 'A' || A->tensor_str.at(contA[1]) == 'A' || A->tensor_str.at(contA[2]) == 'A') symA1f=true;
	if(A->tensor_str.at(extA[0]) == 'b' || A->tensor_str.at(contA[0]) == 'B' || A->tensor_str.at(contA[1]) == 'B' || A->tensor_str.at(contA[2]) == 'B') symA2f=true;
	if(B->tensor_str.at(extA[0]) == 'a' || B->tensor_str.at(contA[0]) == 'A' || B->tensor_str.at(contA[1]) == 'A' || B->tensor_str.at(contA[2]) == 'A') symB1f=true;
	if(B->tensor_str.at(extA[0]) == 'b' || B->tensor_str.at(contA[0]) == 'B' || B->tensor_str.at(contA[1]) == 'B' || B->tensor_str.at(contA[2]) == 'B') symB2f=true;
	
	
	if(size_A >= size_B)
	{	
		curidmapA[extA[0]]=lar;
		if(!symA1f && !symA2f)
		{
			curidmapA[contA[0]]=lar-1; curidmapA[contA[1]]=4-lar; curidmapA[contA[2]]=4-lar-1;
		}
		else if(symA1f && A->tensor_str.at(extA[0]) == 'a')
		{
			if(A->tensor_str.at(contA[0])=='A'){ curidmapA[contA[0]]=lar-1; curidmapA[contA[1]]=4-lar; curidmapA[contA[2]]=4-lar-1; }
			else if(A->tensor_str.at(contA[1])=='A'){ curidmapA[contA[1]]=lar-1; curidmapA[contA[0]]=4-lar; curidmapA[contA[2]]=4-lar-1; }
			else if(A->tensor_str.at(contA[2])=='A'){ curidmapA[contA[2]]=lar-1; curidmapA[contA[0]]=4-lar; curidmapA[contA[0]]=4-lar-1; }
		}
		else if(symA2f && A->tensor_str.at(extA[0]) == 'b')
		{
			if(A->tensor_str.at(contA[0])=='B'){ curidmapA[contA[0]]=lar-1; curidmapA[contA[1]]=4-lar; curidmapA[contA[2]]=4-lar-1; }
			else if(A->tensor_str.at(contA[1])=='B'){ curidmapA[contA[1]]=lar-1; curidmapA[contA[0]]=4-lar; curidmapA[contA[2]]=4-lar-1; }
			else if(A->tensor_str.at(contA[2])=='B'){ curidmapA[contA[2]]=lar-1; curidmapA[contA[0]]=4-lar; curidmapA[contA[0]]=4-lar-1; }
		}
		else if(symA1f && A->tensor_str.at(contA[0]) == 'A')
		{
			if(A->tensor_str.at(contA[1]) == 'A'){curidmapA[contA[0]]=4-lar; curidmapA[contA[1]]=4-lar-1; curidmapA[contA[2]]=lar-1;}
			else if(A->tensor_str.at(contA[2]) == 'A'){curidmapA[contA[0]]=4-lar; curidmapA[contA[2]]=4-lar-1; curidmapA[contA[1]]=lar-1;}
		}
		else if(symA2f && A->tensor_str.at(contA[0]) == 'B')
		{
			if(A->tensor_str.at(contA[1]) == 'B'){curidmapA[contA[0]]=4-lar; curidmapA[contA[1]]=4-lar-1; curidmapA[contA[2]]=lar-1;}
			else if(A->tensor_str.at(contA[2]) == 'B'){curidmapA[contA[0]]=4-lar; curidmapA[contA[2]]=4-lar-1; curidmapA[contA[1]]=lar-1;}
		}
		else if(symA1f && A->tensor_str.at(contA[1]) == 'A')
		{
			curidmapA[contA[1]]=4-lar; curidmapA[contA[2]]=4-lar-1; curidmapA[contA[0]]=lar-1;
		}
		else if(symA2f && 	A->tensor_str.at(contA[1]) == 'B')
		{
			curidmapA[contA[1]]=4-lar; curidmapA[contA[2]]=4-lar-1; curidmapA[contA[0]]=lar-1;
		}
		
		if(!symB1f && !symB2f)
		{
			curidmapB[extB[0]]=lar-1; curidmapB[contB[0]]=lar; curidmapB[contB[1]]=4-lar; curidmapB[contB[2]]=4-lar-1;
			curidmapC[0]=lar; curidmapC[1]=lar-1;
		}
		else if(symB1f && B->tensor_str.at(extB[0]) == 'a')
		{
			curidmapB[extB[0]]=4-lar; 
			if(B->tensor_str.at(contB[0]) == 'A'){ curidmapB[contB[0]]=4-lar-1; curidmapB[contB[1]]=lar; curidmapB[contB[2]]=lar-1; curidmapC[0]=lar; curidmapC[1]=4-lar;}
			else if(B->tensor_str.at(contB[1]) == 'A'){ curidmapB[contB[1]]=4-lar-1; curidmapB[contB[0]]=lar; curidmapB[contB[2]]=lar-1; curidmapC[0]=lar; curidmapC[1]=4-lar;}
			else if(B->tensor_str.at(contB[2]) == 'A'){ curidmapB[contB[2]]=4-lar-1; curidmapB[contB[1]]=lar; curidmapB[contB[0]]=lar-1; curidmapC[0]=lar; curidmapC[1]=4-lar;}
		}	
		else if	(symB2f && B->tensor_str.at(extB[0]) == 'b')
		{
			curidmapB[extB[0]]=4-lar; 
			if(B->tensor_str.at(contB[0]) == 'B'){ curidmapB[contB[0]]=4-lar-1; curidmapB[contB[1]]=lar; curidmapB[contB[2]]=lar-1; curidmapC[0]=lar; curidmapC[1]=4-lar;}
			else if(B->tensor_str.at(contB[1]) == 'B'){ curidmapB[contB[1]]=4-lar-1; curidmapB[contB[0]]=lar; curidmapB[contB[2]]=lar-1; curidmapC[0]=lar; curidmapC[1]=4-lar;}
			else if(B->tensor_str.at(contB[2]) == 'B'){ curidmapB[contB[2]]=4-lar-1; curidmapB[contB[1]]=lar; curidmapB[contB[0]]=lar-1; curidmapC[0]=lar; curidmapC[1]=4-lar;}
		}
		else if	(symB1f && B->tensor_str.at(contB[0]) == 'A')
		{
			curidmapB[extB[0]]=lar-1; 
			if(B->tensor_str.at(contB[1]) == 'A'){ curidmapB[contB[0]]=4-lar; curidmapB[contB[1]]=4-lar-1; curidmapB[contB[2]]=lar-1; curidmapC[0]=lar; curidmapC[1]=lar-1;}
			else if(B->tensor_str.at(contB[2]) == 'A'){ curidmapB[contB[1]]=lar-1; curidmapB[contB[0]]=4-lar; curidmapB[contB[2]]=4-lar-1; curidmapC[0]=lar; curidmapC[1]=lar-1;}
		}	
		else if	(symB1f && B->tensor_str.at(contB[1]) == 'A')
		{
			curidmapB[extB[0]]=lar-1; 
			 curidmapB[contB[0]]=lar; curidmapB[contB[1]]=4-lar; curidmapB[contB[2]]=4-lar-1; curidmapC[0]=lar; curidmapC[1]=lar-1;
		}
		else if	(symB2f && B->tensor_str.at(contB[0]) == 'B')
		{
			curidmapB[extB[0]]=lar-1; 
			if(B->tensor_str.at(contB[1]) == 'B'){ curidmapB[contB[0]]=4-lar; curidmapB[contB[1]]=4-lar-1; curidmapB[contB[2]]=lar-1; curidmapC[0]=lar; curidmapC[1]=lar-1;}
			else if(B->tensor_str.at(contB[2]) == 'B'){ curidmapB[contB[1]]=lar-1; curidmapB[contB[0]]=4-lar; curidmapB[contB[2]]=4-lar-1; curidmapC[0]=lar; curidmapC[1]=lar-1;}
		}	
		else if	(symB2f && B->tensor_str.at(contB[1]) == 'B')
		{
			curidmapB[extB[0]]=lar-1; 
			 curidmapB[contB[0]]=lar; curidmapB[contB[1]]=4-lar; curidmapB[contB[2]]=4-lar-1; curidmapC[0]=lar; curidmapC[1]=lar-1;
		}
	}
	else
	{
		curidmapB[extB[0]]=lar;
		if(!symB1f && !symB2f)
		{
			curidmapB[contB[0]]=lar-1; curidmapB[contB[1]]=4-lar; curidmapB[contB[2]]=4-lar-1;
		}
		else if(symB1f && B->tensor_str.at(extB[0]) == 'a')
		{
			if(B->tensor_str.at(contB[0])=='A'){ curidmapB[contB[0]]=lar-1; curidmapB[contB[1]]=4-lar; curidmapB[contB[2]]=4-lar-1; }
			else if(B->tensor_str.at(contB[1])=='A'){ curidmapB[contB[1]]=lar-1; curidmapB[contB[0]]=4-lar; curidmapB[contB[2]]=4-lar-1; }
			else if(B->tensor_str.at(contB[2])=='A'){ curidmapB[contB[2]]=lar-1; curidmapB[contB[0]]=4-lar; curidmapB[contB[0]]=4-lar-1; }
		}
		else if(symB2f && B->tensor_str.at(extB[0]) == 'b')
		{
			if(B->tensor_str.at(contB[0])=='B'){ curidmapB[contB[0]]=lar-1; curidmapB[contB[1]]=4-lar; curidmapB[contB[2]]=4-lar-1; }
			else if(B->tensor_str.at(contB[1])=='B'){ curidmapB[contB[1]]=lar-1; curidmapB[contB[0]]=4-lar; curidmapB[contB[2]]=4-lar-1; }
			else if(B->tensor_str.at(contB[2])=='B'){ curidmapB[contB[2]]=lar-1; curidmapB[contB[0]]=4-lar; curidmapB[contB[0]]=4-lar-1; }
		}
		else if(symB1f && B->tensor_str.at(contB[0]) == 'A')
		{
			if(B->tensor_str.at(contB[1]) == 'A'){curidmapB[contB[0]]=4-lar; curidmapB[contB[1]]=4-lar-1; curidmapB[contB[2]]=lar-1;}
			else if(B->tensor_str.at(contB[2]) == 'A'){curidmapB[contB[0]]=4-lar; curidmapB[contB[2]]=4-lar-1; curidmapB[contB[1]]=lar-1;}
		}
		else if(symB2f && B->tensor_str.at(contB[0]) == 'B')
		{
			if(B->tensor_str.at(contB[1]) == 'B'){curidmapB[contB[0]]=4-lar; curidmapB[contB[1]]=4-lar-1; curidmapB[contB[2]]=lar-1;}
			else if(B->tensor_str.at(contB[2]) == 'B'){curidmapB[contB[0]]=4-lar; curidmapB[contB[2]]=4-lar-1; curidmapB[contB[1]]=lar-1;}
		}
		else if(symB1f && B->tensor_str.at(contB[1]) == 'A')
		{
			curidmapB[contB[1]]=4-lar; curidmapB[contB[2]]=4-lar-1; curidmapB[contB[0]]=lar-1;
		}
		else if(symB2f && 	B->tensor_str.at(contB[1]) == 'B')
		{
			curidmapB[contB[1]]=4-lar; curidmapB[contB[2]]=4-lar-1; curidmapB[contB[0]]=lar-1;
		}
		
		if(!symA1f && !symA2f)
		{
			curidmapA[extA[0]]=lar-1; curidmapA[contA[0]]=lar; curidmapA[contA[1]]=4-lar; curidmapA[contA[2]]=4-lar-1;
			curidmapC[0]=lar; curidmapC[1]=lar-1;
		}
		else if(symA1f && A->tensor_str.at(extA[0]) == 'a')
		{
			curidmapA[extA[0]]=4-lar; 
			if(A->tensor_str.at(contA[0]) == 'A'){ curidmapA[contA[0]]=4-lar-1; curidmapA[contA[1]]=lar; curidmapA[contA[2]]=lar-1; curidmapC[0]=lar; curidmapC[1]=4-lar;}
			else if(A->tensor_str.at(contA[1]) == 'A'){ curidmapA[contA[1]]=4-lar-1; curidmapA[contA[0]]=lar; curidmapA[contA[2]]=lar-1; curidmapC[0]=lar; curidmapC[1]=4-lar;}
			else if(A->tensor_str.at(contA[2]) == 'A'){ curidmapA[contA[2]]=4-lar-1; curidmapA[contA[1]]=lar; curidmapA[contA[0]]=lar-1; curidmapC[0]=lar; curidmapC[1]=4-lar;}
		}	
		else if	(symA2f && A->tensor_str.at(extA[0]) == 'b')
		{
			curidmapA[extA[0]]=4-lar; 
			if(A->tensor_str.at(contA[0]) == 'B'){ curidmapA[contA[0]]=4-lar-1; curidmapA[contA[1]]=lar; curidmapA[contA[2]]=lar-1; curidmapC[0]=lar; curidmapC[1]=4-lar;}
			else if(A->tensor_str.at(contA[1]) == 'B'){ curidmapA[contA[1]]=4-lar-1; curidmapA[contA[0]]=lar; curidmapA[contA[2]]=lar-1; curidmapC[0]=lar; curidmapC[1]=4-lar;}
			else if(A->tensor_str.at(contA[2]) == 'B'){ curidmapA[contA[2]]=4-lar-1; curidmapA[contA[1]]=lar; curidmapA[contA[0]]=lar-1; curidmapC[0]=lar; curidmapC[1]=4-lar;}
		}
		else if	(symA1f && A->tensor_str.at(contA[0]) == 'A')
		{
			curidmapA[extA[0]]=lar-1; 
			if(A->tensor_str.at(contA[1]) == 'A'){ curidmapA[contA[0]]=4-lar; curidmapA[contA[1]]=4-lar-1; curidmapA[contA[2]]=lar-1; curidmapC[0]=lar; curidmapC[1]=lar-1;}
			else if(A->tensor_str.at(contA[2]) == 'A'){ curidmapA[contA[1]]=lar-1; curidmapA[contA[0]]=4-lar; curidmapA[contA[2]]=4-lar-1; curidmapC[0]=lar; curidmapC[1]=lar-1;}
		}	
		else if	(symA1f && A->tensor_str.at(contA[1]) == 'A')
		{
			curidmapA[extA[0]]=lar-1; 
			 curidmapA[contA[0]]=lar; curidmapA[contA[1]]=4-lar; curidmapA[contA[2]]=4-lar-1; curidmapC[0]=lar; curidmapC[1]=lar-1;
		}
		else if	(symA2f && A->tensor_str.at(contA[0]) == 'B')
		{
			curidmapA[extA[0]]=lar-1; 
			if(A->tensor_str.at(contA[1]) == 'B'){ curidmapA[contA[0]]=4-lar; curidmapA[contA[1]]=4-lar-1; curidmapA[contA[2]]=lar-1; curidmapC[0]=lar; curidmapC[1]=lar-1;}
			else if(A->tensor_str.at(contA[2]) == 'B'){ curidmapA[contA[1]]=lar-1; curidmapA[contA[0]]=4-lar; curidmapA[contA[2]]=4-lar-1; curidmapC[0]=lar; curidmapC[1]=lar-1;}
		}	
		else if	(symA2f && A->tensor_str.at(contA[1]) == 'B')
		{
			curidmapA[extA[0]]=lar-1; 
			 curidmapA[contA[0]]=lar; curidmapA[contA[1]]=4-lar; curidmapA[contA[2]]=4-lar-1; curidmapC[0]=lar; curidmapC[1]=lar-1;
		}
	}	
}	



void cost::contract_2D_4D_4D(int* &idmapA, int* &idmapB, int* &idmapC, int* &ndimG, int* &npgrid)
{	
	
	ndimG[0]=4; //Always 4D grid for these contractions
	npgrid=new int[ndimG[0]];
	
	int fsize=2*ceil(sqrt(nprocs));
	int* temp = new int[fsize];
	int nfacts=0;

	for(int i=1; i<ceil(sqrt(nprocs));i++) if(nprocs%i==0) {temp[nfacts]=i; nfacts++; temp[nfacts]=nprocs/i; nfacts++;}
	int *facts = new int[4*nfacts];
	for(int i=0;i<nfacts;i++){for(int j=0; j<4; j++)facts[i*4+j]=temp[i];}
	
	int* curgrid = new int[4];
	found=false;
	Grid_Comb_4Dd(facts,curgrid,0,4*nfacts-1,0,4, idmapA, idmapB, idmapC, npgrid);
}



void cost::Grid_Comb_4Dd(int arr[], int data[], int start, int end, int index, int r,int* &idmapA, int* &idmapB, int* &idmapC, int* &npgrid)
{	
	if(found)return;
	if(index==r)
	{
		int temp=1;
		for (int i=0;i<4;i++) temp=temp*data[i];
		if(temp==nprocs && data[0]==data[1] && data[2]==data[3]) //Additional conditions to ensure symmetric dimensions are in same sized dimensions
		{	
			best_Cost_GGrid_4Dd(data, idmapA, idmapB, idmapC);	     	
			for(int i=0;i<4;i++) npgrid[i]=data[i];
			found=true;
		}
	return;
	}

	for (int i=start; i<=end && end-i+1 >= r-index; i++)
    	{
        	data[index] = arr[i];
        	Grid_Comb_4Dd(arr, data, i+1, end, index+1, r, idmapA, idmapB, idmapC, npgrid);
  
        	// Remove duplicates
        	while (arr[i] == arr[i+1])
             	i++;
    	}

}


void cost::best_Cost_GGrid_4Dd(int* &curGrid, int* &curidmapA, int* &curidmapB, int* &curidmapC)
{	
	
	curidmapA = new int[2];
	curidmapB=new int[4];
	curidmapC=new int[4];
	int contB[1]={};
	int extB[3]={};
	int contA[1]={};
	int extA[1]={};

	int ccnt=0, ecnt=0;
	for(int i=0; i<dimA; i++)
	    	{
        		for(int j=0; j<dimB; j++)
       		 	{
        	    		if(A->contr_dim_str[i].compare(B->contr_dim_str[j]) == 0)
          			{
               		 		// i in A and j in B are contracting indices
                			contA[ccnt]=i; contB[ccnt]=j;
					ccnt++;
						
            			}
        		}
    		}

	for(int i=0;i<4;i++) {if(i!=contA[0]) extA[ecnt]=i; ecnt++;}
	ecnt=0;
	for(int i=0;i<4;i++) {if(i!=contB[0]) extB[ecnt]=i; ecnt++;}

	int lar=1;
	if(curGrid[3] > curGrid[1]) lar=3;

	
	int temp;
	for(int i=1; i<3; i++){if(B->tensor_size[extB[i]] > B->tensor_size[extB[0]] ) {temp = extB[0]; extA[0]=extB[i]; extB[i]=temp;}}
	
	
	if(B->tensor_size[extB[0]] >= B->tensor_size[extB[1]] && B->tensor_size[extB[0]] >= B->tensor_size[extB[2]])
	{
		curidmapB[extB[0]]=lar;
		if(B->tensor_str.at(extB[0]) == 'c' || B->tensor_str.at(extB[0]) == 'd')
		{ 
			if(B->tensor_size[extB[1]] >= B->tensor_size[extB[2]])
			{
				if(B->tensor_str.at(extB[1]) == 'c' || B->tensor_str.at(extB[1]) == 'd')
				{
					curidmapB[extB[1]]=lar-1; curidmapB[extB[2]]=4-lar; curidmapB[contB[0]]=4-lar-1;
					curidmapA[contA[0]]=4-lar; curidmapA[extA[0]]=4-lar-1;
					curidmapC[0]=lar; curidmapC[1]=lar-1; curidmapC[2]=4-lar; curidmapC[3]=4-lar-1;
				}
				else if(B->tensor_str.at(extB[1]) == B->tensor_str.at(extB[2]))
				{
					curidmapB[extB[1]]=4-lar; curidmapB[extB[2]]=4-lar-1; curidmapB[contB[0]]=lar-1;
					curidmapA[contA[0]]=lar; curidmapA[extA[0]]=lar-1;
					curidmapC[0]=lar; curidmapC[1]=4-lar; curidmapC[2]=4-lar-1; curidmapC[3]=lar-1;	 
				}
				else
				{
					curidmapB[extB[1]]=4-lar; curidmapB[extB[2]]=lar-1; curidmapB[contB[0]]=4-lar-1;
					curidmapA[contA[0]]=4-lar; curidmapA[extA[0]]=4-lar-1;
					curidmapC[0]=lar; curidmapC[1]=4-lar; curidmapC[2]=lar-1; curidmapC[3]=4-lar-1;
				}
			}
			else if(B->tensor_size[extB[2]] >= B->tensor_size[extB[1]])
			{
				if(B->tensor_str.at(extB[2]) == 'c' || B->tensor_str.at(extB[2]) == 'd')
				{
					curidmapB[extB[2]]=lar-1; curidmapB[extB[1]]=4-lar; curidmapB[contB[0]]=4-lar-1;
					curidmapA[contA[0]]=4-lar; curidmapA[extA[0]]=4-lar-1;
					curidmapC[0]=lar; curidmapC[2]=lar-1; curidmapC[1]=4-lar; curidmapC[3]=4-lar-1;
				}
				else if(A->tensor_str.at(extA[2]) == A->tensor_str.at(extA[1]))
				{
					curidmapB[extB[2]]=4-lar; curidmapB[extB[1]]=4-lar-1; curidmapB[contB[0]]=lar-1;
					curidmapA[contA[0]]=lar; curidmapA[extA[0]]=lar-1;
					curidmapC[0]=lar; curidmapC[2]=4-lar; curidmapC[1]=4-lar-1; curidmapC[3]=lar-1;	 
				}
				else
				{
					curidmapB[extB[2]]=4-lar; curidmapB[extB[1]]=lar-1; curidmapB[contB[0]]=4-lar-1;
					curidmapA[contA[0]]=4-lar; curidmapA[extA[0]]=4-lar-1;
					curidmapC[0]=lar; curidmapC[2]=4-lar; curidmapC[1]=lar-1; curidmapC[3]=4-lar-1;
				}
			}
		}
		else if(B->tensor_str.at(extB[0])  ==  B->tensor_str.at(extB[1]))
		{
			curidmapB[extB[1]]=lar-1; curidmapB[extB[2]]=4-lar; curidmapB[contB[0]]=4-lar-1;
			curidmapA[contA[0]]=4-lar; curidmapA[extA[0]]=4-lar-1;
			curidmapC[0]=lar; curidmapC[1]=lar-1; curidmapC[2]=4-lar; curidmapC[3]=4-lar-1;
		}
		else if(B->tensor_str.at(extB[0])  ==  B->tensor_str.at(extB[2]))
		{
			curidmapB[extB[2]]=lar-1; curidmapB[extB[1]]=4-lar; curidmapB[contB[0]]=4-lar-1;
			curidmapA[contA[0]]=4-lar; curidmapA[extA[0]]=4-lar-1;
			curidmapC[0]=lar; curidmapC[2]=lar-1; curidmapC[1]=4-lar; curidmapC[3]=4-lar-1;
		}
		else
		{
			curidmapB[contB[0]]=lar-1; curidmapB[extB[1]]=4-lar; curidmapB[extB[2]]=4-lar-1;
			curidmapA[contA[0]]=lar; curidmapA[extA[0]]=lar-1;
			curidmapC[0]=lar; curidmapC[2]=4-lar; curidmapC[1]=4-lar-1; curidmapC[3]=lar-1;
		}	
	}

}	

}


