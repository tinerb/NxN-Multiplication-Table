#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <mpi.h>

//function to determine if a number is unique
int unique(unsigned long long i, unsigned long long j, unsigned long long n){
	unsigned long long value = i * j; //actual value
	unsigned long long max_row = (int)floor(sqrt(value)); //maximum row the value can be on
	unsigned long long row = max_row; //loop variable
	//counting down through rows
	while (row > 0){
		//if to see if value is on the row
		if (value%row == 0){
			//column will always be less then n, column and row will always be unique
			//if its the closest value to the diagonal
			if (i == row){
				return 1;
			}
			//if else it is not the unique value
			else{
				return 0;
			}
		}
		row--;
	}
	return 0;
}
//standard work divsion function
void work_division(unsigned long long n, int p, int rank, unsigned long long *out_first, unsigned long long *out_last){
	unsigned long long delta = n/p;
	unsigned long long range_start = delta * rank;
	unsigned long long range_end = delta * (rank + 1) - 1;
	if (rank < n%p){
		range_start += rank;
		range_end += rank + 1;
	}
	else{
		range_start += n%p;
		range_end += n%p;
	}	
	*out_first = range_start + 1;
	*out_last = range_end + 1;
}

int main(int argc, char **argv){
	unsigned long long n = atoi(argv[1]); //problem size as argument
	int p, rank; //processors and rank
	unsigned long long result = 0, temp_result;
	unsigned long long start_range, end_range; //start and end range for work division
	MPI_Status status; //MPI_status
	//MPI Commands
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	//work division algorithm
	work_division(((n * (n + 1))/2), p, rank, &start_range, &end_range);
	
	unsigned long long count = 0; //to count through range
	//setting variables to index work division
	unsigned long long pre_i = start_range, running = start_range, x = 1;
	//counting through matrix to get to correct spot
	while(running > n - (x - 1)){
		running -= n - (x - 1);
		pre_i += x;
		x++;
	}
	//setting loop variables at correct spot in upper triangle
	unsigned long long _i = (pre_i - 1) / n + 1;
	unsigned long long _j = (pre_i - 1) % n + 1;
	//looping for first row for processor
	for (unsigned long long j = _j; j <= n; j++){
		//breaking loop if range is done
		if (count > end_range - start_range){
			break;
		}	
		count++;
		//checking of value is unique
		if (unique(_i, j, n) == 1){
				result++;
		}
		
	}
	//adding 1 to i if the row was finished
	_i++;
	//looping for the rest of the range
	for (unsigned long long i = _i; i <= n; i++){
		for (unsigned long long j = i; j <= n; j++){
			//breaking if range is done
			if (count > end_range - start_range){
				break;
			}	
			count++;
			//checking if value is unique
			if (unique(i, j, n) == 1){
				result++;
			}
		}
		//breaking loop if range is done
		if (count > end_range - start_range){
			break;
		}
	}
	//reciving from other processors
	if (rank == 0){
		for (int i = 1; i < p; i++){
			MPI_Recv(&temp_result, 1, MPI_UNSIGNED_LONG_LONG, i, 0, MPI_COMM_WORLD, &status);
			result += temp_result;
		}
		printf("The number of unique values in a multiplication table of size %lld by %lld is %lld\n", n, n, result);
	}
	//sending to 0 processor
	else{
		MPI_Send(&result, 1, MPI_UNSIGNED_LONG_LONG, 0, 0, MPI_COMM_WORLD);
	}
	
	MPI_Finalize();
	return 0;
}