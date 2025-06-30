#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
  int comm_ranking;
  union {
    float f;
    int i;
  } number;
} CommRankNumber



int main(int argc, char** argv) {

  MPI_Init(&argc, &argv);
  int size, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  int local_value = rank * 10;

  int* all_value = (int*)malloc(size * sizeof(int))
  MPI_Allgather(&local_value, 1, MPI_INT, all_values, 1, MPI_INT, MPI_COMM_WORLD);

  int local_rank =0;
  for (int i = 0; i < size; i++){
    if (all_values[i] < local_value) {
      local_rank++'
    }
  }

  
}
