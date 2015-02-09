/*********************************************************************
       Assignment 1 / Programming of Parallel Computers

       Authors: Georgios Ziogas, Charalampos Kominos, Stavros Mavrakis

*********************************************************************/
// #include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) 
{

	// Variables test test changes
	int n,p,row,col;

	// Check for wrong number of arguments
	printf("\n");
	if (argc!=3){
    	printf("Wrong number of arguments.\nneeded 2 (Matrices Size, Processors)\n");    
    	return 0;
  	}

	// Converts string arguments to int
	n = atoi(argv[1]);
  	p = atoi(argv[2]);
  	printf("Size of matrices: %d\n",n);
  	printf("number of processors: %d\n",p);

  	// Set matrices according to given size
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
	printf("\nMatrix A:\n");
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

	// Free memory after execution
	free(A),free(B),free(C);
  	return 0;
}