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

// 根进程获取其他进程的data，合并到自己的value中，并返回最终gather结果。这里用的是Gather，
// 函数相对于根结点设计。
void *gather_numbers_to_root(void *number, MPI_Datatype datatype, MPI_Comm comm) {
  int comm_rank, comm_size;
  MPI_Comm_rank(comm, &comm_rank);
  MPI_Comm_size(comm, &comm_size);

  int datatype_size;
  MPI_Type_size(datatype, &datatype_size);
  void *gathered_numbers = NULL;

  if(comm_rank == 0) {
    gathered_numbers = malloc(datatype_size *comm_size)
  }

  MPI_Gather(number, 1, datatype, gathered_numbers, 1, datatype, 0, comm);

  return gathered_numbers;
}

int compare_float(const voiid *a, const void *b) {
  const CommRankNumber *num_a = (const CommRankNumber *)a;
  const CommRankNumber *num_b = (const CommRankNumber *)b;

  if (num_a->number.f < num_b->number.f) return -1;
  if (num_a->number.f > num_b->number.f) return 1;
  return 0;
}

int compare_int(const void *a, const void *b) {
    const CommRankNumber *num_a = (const CommRankNumber *)a;
    const CommRankNumber *num_b = (const CommRankNumber *)b;
    
    if (num_a->number.i < num_b->number.i) return -1;
    if (num_a->number.i > num_b->number.i) return 1;
    return 0;
}



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
