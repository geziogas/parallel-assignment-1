/*********************************************************************
       Assignment 1 / Programming of Parallel Computers

       Authors: Georgios Ziogas, Charalampos Kominos, Stavros Mavrakis

*********************************************************************/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int main(int argc, char *argv[])
{

	// Variables
	int root = 0; //The root process
	int n,row,col,nproc,my_id,row_rank,column_rank,row_size,column_size;
	int coords[2],pos[2],reorder=1,ndim=2,dims[2]={0,0},periods[2]={0,0};
	int x,count,blocklength,stride,limit,temp_rank;
	double *blockA,*blockB,*blockC;
	double *A,*B,*C;
	clock_t start_time,end_time;
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
	// printf("\n");
	if (argc!=2){

		if (my_id==root)
		{
    		printf("Wrong number of arguments.\nNeeded matrices size only (1 argument).\n");
    	}
    	MPI_Finalize();
    	return 0;
  	}

	// Converts string arguments to int
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

  		// Create matrices according to given size
  		A=(double *)calloc(n*n,sizeof(double ));
  		B=(double *)calloc(n*n,sizeof(double ));
  		C=(double *)calloc(n*n,sizeof(double )); //Result-matrix

	  	srand(time(NULL)); //Used to change the seed according to time.

	  	// Set the random limit number
	  	double scaleLimit = 100.0;
	  	double divisor = (double)RAND_MAX/scaleLimit;

	  	for(row=0;row<n;row++){
			for(col=0;col<n;col++)
			{/* 
			Gia to testining A= monadiaios pinakas
			
				A[row*n+col]=0;
            if(row=col){A[row*n+col]=1;}
   			 
  			 */					
				
				
				A[row*n+col]=rand()/divisor;				
				B[row*n+col]=rand()/divisor;
			}

		}

		// Test code, to show the 2 random generated matrices
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
		}

		printf("\nMatrix C:\n");
		for(row=0;row<n;row++){
			for(col=0;col<n;col++)
			{
				printf("%f ",C[row*n+col]);
			}
			printf("\n");
		}

		printf("\n");
	}


	printf("Welcome. I am process No %d / coords: %d.%d\n",my_id,coords[0],coords[1]);


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

	// Root process sends each other process their assigned block of data
	if(my_id==root) {
		for(row=0;row<row_size;row++){
			for(col=0;col<column_size;col++){
				pos[0]=row;
				pos[1]=col;
				MPI_Cart_rank(proc_grid,pos,&temp_rank);
				x=(row*n+col)*count;
				MPI_Isend(&A[x],1,newtype,temp_rank,111,proc_grid,&r1);
				MPI_Isend(&B[x],1,newtype,temp_rank,222,proc_grid,&r2);
			}
		}
	}

	// The other processes receive their assigned block of data
	if(my_id!=root) {
		MPI_Recv(blockA,count*count,MPI_DOUBLE,0,111,proc_grid, &status);
		MPI_Recv(blockB,count*count,MPI_DOUBLE,0,222,proc_grid, &status);
	}

	// Measure the processing time
	if(my_id==root){
		start_time = clock();
	}









	// Part to show the result C matrix
	if(my_id==root) {
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

  	// Free memory after execution
  	MPI_Comm_free(&proc_row);
  	MPI_Comm_free(&proc_column);
  	MPI_Comm_free(&proc_grid);
  	
if(my_id==root) {
	free(A),free(B),free(C);
	free(blockA);
	free(blockB);
	free(blockC);
}else{
	free(blockA);
	free(blockB);
	free(blockC);

}
MPI_Finalize();
  	return 0;
}
