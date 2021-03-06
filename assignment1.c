/*********************************************************************
       Assignment 1 / Programming of Parallel Computers

       Authors: Georgios Ziogas, Charalampos Kominos, Stavros Mavrakis

*********************************************************************/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h> /* memset */
#include <unistd.h> /* close */

int main(int argc, char *argv[])
{

	// Variables
	int root = 0; //The root process
	int n,row,col,nproc,my_id,row_rank,column_rank,row_size,column_size,temp;
	int coords[2],pos[2],reorder=1,ndim=2,dims[2]={0,0},periods[2]={0,0};
	int x,y,k,count,blocklength,stride,limit,temp_rank;
	double *blockA,*blockB,*blockC;
	double *A,*B,*C,*tempB,*tempA;
	clock_t start_time,end_time;
	double total;
	MPI_Comm proc_grid,proc_row,proc_column;
	MPI_Datatype newtype;
	MPI_Request r1,r2;
	MPI_Status status;


	// Initialize MPI
  	MPI_Init(&argc,&argv);
  	MPI_Comm_size(MPI_COMM_WORLD,&nproc);

  	// Create a virtual 2D-grid topology
  	MPI_Dims_create(nproc,ndim,dims);
  	MPI_Cart_create(MPI_COMM_WORLD,ndim,dims,periods,reorder,&proc_grid);
  	MPI_Comm_rank(proc_grid,&my_id);
  	MPI_Cart_coords(proc_grid,my_id,ndim,coords);

  	// Create a communicator for each row
  	MPI_Comm_split(proc_grid,coords[0],coords[1],&proc_row);
  	MPI_Comm_rank(proc_row,&row_rank);
  	MPI_Comm_size(proc_row,&row_size);

  	// Create a communicator for each column
  	MPI_Comm_split(proc_grid,coords[1],coords[0],&proc_column);
  	MPI_Comm_rank(proc_column,&column_rank);
  	MPI_Comm_size(proc_column,&column_size);


	// Check for wrong number of arguments
	if (argc!=2){

		if (my_id==root)
		{
    		printf("Wrong number of arguments.\nNeeded matrices size only (1 argument).\n");
    	}
    	MPI_Finalize();
    	return 0;
  	}

	// Convert string argument to int
	n = atoi(argv[1]);

	if (my_id==root)
	{
		printf("I am the root process.\n");
  		printf("Size of matrices: %d\n",n);
  		printf("number of processes: %d\n",nproc);
	}


  	// Set and show the matrices by the root process
  	if (my_id==root)
  	{

  		// Create matrices according to given size and initialize them to 0
  		A=(double *)calloc(n*n,sizeof(double ));
  		B=(double *)calloc(n*n,sizeof(double ));
  		C=(double *)calloc(n*n,sizeof(double )); //Result-matrix

	  	srand(time(NULL)); //Used to change the seed according to time.

	  	// Set the random limit number
	  	double scaleLimit = 100.0;
	  	double divisor = (double)RAND_MAX/scaleLimit;

	  	// Fill the two matrices with random values
	  	for(row=0;row<n;row++){
			for(col=0;col<n;col++)
			{
				 A[row*n+col]=rand()/divisor;
				 B[row*n+col]=rand()/divisor;
				/* Just to test the correctness of our program we set one of the matrices to
				 * be the identity matrix */
				 /*A[row*n+col]=0;
				 if(row==col){
					 A[row*n+col]=1;
				 }*/
			}
		}

		/*// Test code, to show the 2 random generated matrices
		printf("\nMatrix A:\n");
		for(row=0;row<n;row++){
			for(col=0;col<n;col++)
			{
				printf("%f ",A[row*n+col]);
			}
			printf("\n");

		}

		printf("\nMatrix B:\n");
		for(row=0;row<n;row++){
			for(col=0;col<n;col++)
			{
				printf("%f ",B[row*n+col]);
			}
			printf("\n");
		}*/

		printf("\n");
	}

	// Start point to measure the processing time
	if(my_id==root){
		start_time = MPI_Wtime();
	}

	limit = sqrt(nproc);
	count = n/limit;
	blocklength = n/limit;
	stride = n;

	// Divide the blocks
	MPI_Type_vector(count,blocklength,stride,MPI_DOUBLE,&newtype);
	MPI_Type_commit(&newtype);

	// Create space for each process's block
	blockA=(double*)calloc(count*count,sizeof(double));
	blockB=(double*)calloc(count*count,sizeof(double));
	blockC=(double*)calloc(count*count,sizeof(double));

	// Root process sends each other process their assigned blocks of data
	if(my_id==root){
		for(row=0;row<row_size;row++)
			for(col=0;col<column_size;col++){
				pos[0]=row;
				pos[1]=col;
				MPI_Cart_rank(proc_grid,pos,&temp_rank);
				x=(row*n+col)*count;
				MPI_Isend(&A[x],1,newtype,temp_rank,111,proc_grid,&r1);
				MPI_Isend(&B[x],1,newtype,temp_rank,222,proc_grid,&r2);
			}
	}

	// The other processes receive their assigned blocks of data
	MPI_Recv(blockA,count*count,MPI_DOUBLE,0,111,proc_grid, &status);
	MPI_Recv(blockB,count*count,MPI_DOUBLE,0,222,proc_grid, &status);

	tempA=(double*)calloc(count*count,sizeof(double));
	tempB=(double*)calloc(count*count,sizeof(double));


	// Start Fox�s algorithm
	for(k=0;k<limit;k++){
		temp = (coords[0] + k)%limit;
		// Broadcast the diagonal elements of A across the rows
		if (row_rank == temp){
			MPI_Bcast(blockA,count*count,MPI_DOUBLE,temp,proc_row);
			for(row=0;row<count;row++){
				for(col=0;col<count;col++){
					for(y=0;y<count;y++){
						// Do the actual multiplication of the blocks
						blockC[col*count+y]+=blockA[col*count+row]*blockB[row*count+y];}
				}
			}
		}
		else{
			MPI_Bcast(tempA,count*count,MPI_DOUBLE,temp,proc_row);
			for(row=0;row<count;row++){
				for(col=0;col<count;col++){
					for(y=0;y<count;y++){
						// Do the actual multiplication of the blocks
						blockC[col*count+y]+=tempA[col*count+row]*blockB[row*count+y];}
				}
			}
		}

		// Shift the B blocks one step cyclicly upwards
		MPI_Isend(blockB,count*count, MPI_DOUBLE,(column_rank+column_size-1)%column_size,111,proc_column,&r1);
		MPI_Irecv(tempB,count*count,MPI_DOUBLE,MPI_ANY_SOURCE,111,proc_column,&r2);
		MPI_Wait(&r1,&status);
		MPI_Wait(&r2,&status);
		memcpy(blockB,tempB,count*count*sizeof(double));
	}
  
	// Send the resulting blocks back to the root process
	MPI_Isend(blockC,count*count,MPI_DOUBLE,0,111,proc_grid,&r1);
	if(my_id==root){
		for(row=0;row<nproc;row++){
			MPI_Probe(MPI_ANY_SOURCE,111,proc_grid,&status);
			for(col=0;col<n*n;col++)
				if (C[col]==0)
					break;
			MPI_Recv(&C[col],n,newtype,row,111,proc_grid,&status);
		}
	}

	MPI_Wait(&r1,&status);



	// Testing to show the resulting C matrix
	/*if(my_id==root) {
		printf("\nResult C after A*B:\n");
		for(row=0;row<n;row++){
			for(col=0;col<n;col++)
			{
				printf("%f ",C[row*n+col]);
			}
			printf("\n");
		}
		printf("\n");
	}
*/
	// Print total time taken
	if(my_id==root){
		total = MPI_Wtime() - start_time;
		// total = (double)end_time*1000 / CLOCKS_PER_SEC;
		printf("Wall time: %f seconds\n",total);
	}

  	// Free memory after execution
  	MPI_Comm_free(&proc_row);
  	MPI_Comm_free(&proc_column);
  	MPI_Comm_free(&proc_grid);

  	if(my_id==root) {
  		free(A),free(B),free(C);
  	}
  	free(blockA);
  	free(blockB);
  	free(blockC);
  	free(tempB);
  	free(tempA);

  	MPI_Finalize();
  	return 0;
}
