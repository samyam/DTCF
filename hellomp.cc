#include<iostream>
#include "omp.h"
#include "mpi.h"

using namespace std;

int main()
{
  int NN = 4096;

    int num_threads;
    //omp_set_num_threads(64);

#pragma omp parallel 
    {

	if(omp_get_thread_num() == 0) 
	{
	    num_threads = omp_get_num_threads();
	    cout<<endl<<"Total number of threads is"<<num_threads<<endl;
	}
    }

    int* test_vec = new int[num_threads];
    memset(test_vec,0,sizeof(int)*num_threads);


    double time =- MPI_Wtime();

#pragma omp for schedule(static, 16)
    for(int i = 0; i<NN*NN*NN*NN; i++)
    {
	test_vec[omp_get_thread_num()]+=i*24+7*i*i;
    }

    int sum = 0;
    for(int i = 0; i<num_threads; i++)
    {
	sum+=test_vec[i];
    }
    time += MPI_Wtime();
    cout<<"Time with omp is : "<<time<<endl;

    time =- MPI_Wtime();
    int sum2=0;
    for(int i = 0; i<NN*NN*NN*NN; i++)
    {
	sum2+=i*24+7*i*i;
    }
    time += MPI_Wtime();
    cout<<"Time without omp is : "<<time<<endl;
    return 0;

}
