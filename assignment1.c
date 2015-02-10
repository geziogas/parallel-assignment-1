/*********************************************************************
       Assignment 1 / Programming of Parallel Computers

       Authors: Georgios Ziogas, Charalampos Kominos, Stavros Mavrakis

*********************************************************************/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{

	// Variables
	int n,row,col,nproc,my_id,row_rank,column_rank,row_size,column_size;
	MPI_Comm proc_grid,proc_row,proc_column;
	int coords[2],pos[2],reorder=1,ndim=2,dims[2]={0,0},periods[2]={0,0};


	// Check for wrong number of arguments
	printf("\n");
	if (argc!=3){
    	printf("Wrong number of arguments.\nneeded 2 (Matrices Size, Processors)\n");
    	return 0;
  	}

	// Converts string arguments to int
	n = atoi(argv[1]);
	nproc = atoi(argv[2]);
  	printf("Size of matrices: %d\n",n);
  	printf("number of processors: %d\n",nproc);

  	// Create matrices according to given size
  	double *A,*B,*C;
  	A=(double *)calloc(n*n,sizeof(double ));
  	B=(double *)calloc(n*n,sizeof(double ));
  	C=(double *)calloc(n*n,sizeof(double )); //Result-matrix

  	srand(time(NULL)); //Used to change the seed according to time.

  	// Set the random limit number
  	double scaleLimit = 100.0;
  	double divisor = (double)RAND_MAX/scaleLimit;

  	for(row=0;row<n;row++){
		for(col=0;col<n;col++)
		{
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
  	MPI_Comm_size(proc_row,&row_size); // De kserw akoma pou to xrisimopoioun auto, den koitaksa parakatw ton kwdika

  	// Create a communicator for each column
  	MPI_Comm_split(proc_grid,coords[1],coords[0],&proc_column);
  	MPI_Comm_rank(proc_column,&column_rank);
  	MPI_Comm_size(proc_column,&column_size); // De kserw akoma pou to xrisimopoioun auto, den koitaksa parakatw ton kwdika

	if(myid==0) {
		printf("I am the root. Number of processes: %d\n",nproc);
	}

	printf("Welcome. I am process No %d / coords: %d.%d\n",myid,row_rank,column_rank);
	printf("\n");



  	// Free memory after execution
  	MPI_Comm_free(&proc_row);
  	MPI_Comm_free(&proc_column);
  	MPI_Comm_free(&proc_grid);
  	MPI_Finalize();

	free(A),free(B),free(C);
  	return 0;
}