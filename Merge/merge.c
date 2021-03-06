#include <stdio.h>
#include <mpi.h>
#include <omp.h>	
#include <math.h>
#include<stdlib.h>
#include <string.h>
#include<stdbool.h>
#include<sys/time.h>
#define winner 0
#define loser 1
#define bye 2
#define champion 3
#define dropout 4

struct round_t
{
	int role;
	int vpid;
	int tb_round;
	bool *opponent;
	bool flag;
};
typedef struct round_t round_t;
round_t array[100][100];

void tournament_barrier( int vpid, bool *sense,int num_rounds)
{
	int round = 0;
	while(1)
	{

		if(array[vpid][round].role == loser)
		{
			*( array[vpid][round] ).opponent = *sense;
			while( array[vpid][round].flag != *sense );
			break;
		}

		if( array[vpid][round].role == winner )
		{
			while( array[vpid][round].flag != *sense );
		}
		if( array[vpid][round].role == champion )
		{
			while( array[vpid][round].flag != *sense );
			*( array[vpid][round] ).opponent = *sense;
			break;
		}

		if(round < num_rounds)
		{
			round = round + 1;
		}
	}
	while(1) 
	{
		if( round > 0 )
		{
			round = round - 1;
		}

		if( array[vpid][round].role == winner )
		{
			*( array[vpid][round] ).opponent = *sense;
		}
		if( array[vpid][round].role == dropout )
		{
			break;
		}
	}

	*sense = !*sense;
}

void central_barrier(int proc,int num_processes)
{
  int tag = 1;
  int my_msg[2];
  MPI_Status mpi_result;
  my_msg[0] = 0;
  my_msg[1] = 1;

  if(proc==0)
  {
   int i;
    for(i=0;i<num_processes-1;i++)
    {
    MPI_Recv(&my_msg, 2, MPI_INT,i+1, tag, MPI_COMM_WORLD, &mpi_result);
    }
    for(i=0;i<num_processes-1;i++)
    {
     MPI_Send(&my_msg, 2, MPI_INT, i+1, tag, MPI_COMM_WORLD);
    }
  }
 else
 {
  MPI_Send(&my_msg, 2, MPI_INT,0, tag, MPI_COMM_WORLD);
  MPI_Recv(&my_msg, 2, MPI_INT, 0, tag, MPI_COMM_WORLD, &mpi_result);

 }

}

int main(int argc, char **argv)
{
	bool x = false;
	if(argc!=3)
  	{
  		printf("Enter the number of threads and barriers.\n"); 
        	exit(0);
  	}
	struct timeval start, end;
	int thread_count, barrier_count,my_id;
  	int num_processes;
    	MPI_Init(&argc, &argv);
      	MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
      	MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
	printf("[START] Process Id: %d\n", my_id);		
      	//MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
	thread_count = atoi(argv[1]);
	barrier_count = atoi(argv[2]);
	omp_set_num_threads(thread_count);	
	int num_rounds = ceil( log(thread_count)/log(2) );
	//round_t array[thread_count][num_rounds];
	int i,k;
	for(i=0;i<thread_count;i++)
	{
		for(k=0;k<=num_rounds;k++)
		{
			array[i][k].flag = false;
			array[i][k].role = -1;
			array[i][k].opponent = &x;
		}
	}
	int second_check, first_check=0;
	for(i=0 ; i<thread_count; i++)
	{
		for(k=0;k<=num_rounds;k++)
		{
			second_check = ceil( pow(2,k) );
			first_check = ceil( pow(2,k-1) );

			if((k > 0) && (i%second_check==0) && ((i + (first_check))< thread_count) && (second_check < thread_count))
			{
				array[i][k].role = winner;
			}

			if((k > 0) && (i%second_check == 0) && ((i + first_check)) >= thread_count)
			{
				array[i][k].role = bye;
			}

			if((k > 0) && ((i%second_check == first_check)))
			{
				array[i][k].role = loser;
			}

			if((k > 0) && (i==0) && (second_check >= thread_count))
			{
				array[i][k].role = champion;
			}

			if(k==0)
			{
				array[i][k].role = dropout;
			}
			if(array[i][k].role == loser)
			{
				array[i][k].opponent = &array[i-first_check][k].flag;
			}
			if(array[i][k].role == winner || array[i][k].role == champion)
			{
				array[i][k].opponent = &array[i+first_check][k].flag;
			}
		}
	}
	gettimeofday(&start, NULL);
	#pragma omp parallel num_threads(thread_count) shared(array)
	{
		int vpid=0;
		bool *sense;
		bool temp = true;
		#pragma omp critical
		{
			vpid = omp_get_thread_num();
			sense = &temp;
		}
		int num_threads = omp_get_num_threads();
	 	int i,j;
		int thread_num = omp_get_thread_num();
		printf( "Thread Number: %d Process: %d Ready.\n", vpid,my_id);
		for( i = 0; i < barrier_count; i++ )
		{
			printf("Thread: %d Process: %d is waiting at Barrier %d.\n",vpid,my_id,i+1);
			tournament_barrier(vpid,sense,num_rounds);
			printf("Thread: %d Process: %d left Barrier %d.\n",vpid,my_id,i+1);
	    	}
	}
	central_barrier(my_id,num_processes);
	printf("Process %d left Central Barrier MPI.\n",my_id);
	gettimeofday(&end, NULL);
	printf("Total Time:    %ld\n", ((end.tv_sec * 1000000 + end.tv_usec)- (start.tv_sec * 1000000 + start.tv_usec)));
	MPI_Finalize();
	return 0;
}
