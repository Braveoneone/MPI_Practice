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
    gathered_numbers = malloc(datatype_size * comm_size)
  }

  MPI_Gather(number, 1, datatype, gathered_numbers, 1, datatype, 0, comm);

  return gathered_numbers;
}

int compare_float(const void *a, const void *b) {
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

int *get_ranks(void *gathered_nums, int count, MPI_Datatype, datatype) {
    int datatype_size;
    MPI_Type_size(datatype, &datatype_size);
    
    CommRankNumber *rank_nums = malloc(count * sizeof(CommRankNumber));

    for (int i = 0; i < count; i++) {
        rank_nums[i].comm_ranking = i;
        memcpy(&rank_nums[i].number, ((char*)gathered_nums) + i * datatype_size, datatype_size);

    }

    if (datatype == MPI_FLOAT) {
        qsort(rank_nums, count, sizeof(CommRankNumber), compare_float);
    } else {
        qsort(rank_nums, count, sizeof(CommRankNumber), compare_int);
    }
    
    int *ranks = malloc(count * sizeof(int));
    for (int i = 0; i < count; i++) {
        ranks[rank_nums[i].comm_ranking] = i;

    }

    free(rank_nums);
    return ranks;
}

// 主排名函数: 计算并返回每个进程的排名
int TMPI_Rank(void *send_data, void *recv_data, MPI_Datatype datatype, MPI_Comm comm) {
    // 检查支持的数据类型
    if (datatype != MPI_INT && datatype != MPI_FLOAT) {
        return MPI_ERR_TYPE;
    }

    int comm_rank;
    MPI_Comm_rank(comm, &comm_rank);  // 获取当前进程排名

    // 步骤1: 收集所有数据到根进程
    void *gathered = gather_numbers_to_root(send_data, datatype, comm);
    
    // 步骤2: 根进程计算排名
    int *ranks = NULL;
    if (comm_rank == 0) {
        int comm_size;
        MPI_Comm_size(comm, &comm_size);
        ranks = get_ranks(gathered, comm_size, datatype);
    }

    // 步骤3: 分发排名结果给所有进程
    MPI_Scatter(ranks, 1, MPI_INT,   // 发送缓冲区(仅根进程有效)
                recv_data, 1, MPI_INT, // 接收缓冲区
                0, comm);            // 从根进程分发

    // 步骤4: 资源清理
    if (comm_rank == 0) {
        free(gathered);
        free(ranks);
    }
    
    return MPI_SUCCESS;  // 返回成功状态
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);  // 初始化MPI环境

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // 获取当前进程排名
    MPI_Comm_size(MPI_COMM_WORLD, &size);   // 获取总进程数

    srand(time(NULL) + rank);  // 设置随机种子(每个进程不同)
    float number = (float)(rand() % 100); // 生成0-99的随机数，以浮点数为例子。

    int my_rank;  // 存储计算得到的排名
    
    // 计算当前数值在全局中的排名
    TMPI_Rank(&number, &my_rank, MPI_FLOAT, MPI_COMM_WORLD);

    // 收集所有进程的数值用于显示
    float *all_numbers = NULL;
    if (rank == 0) {
        all_numbers = malloc(size * sizeof(float));
    }
    MPI_Gather(&number, 1, MPI_FLOAT, all_numbers, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // 打印结果
    if (rank == 0) {
        printf("\n并行排名结果:\n");
        printf("进程\t数值\t排名\n");
        printf("-------------------\n");
        for (int i = 0; i < size; i++) {
            printf("%d\t%.2f\t%d\n", i, all_numbers[i], my_rank);
        }
        free(all_numbers);
    }

    MPI_Finalize();  // 清理MPI环境
    return 0;
}
