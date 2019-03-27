#ifndef __PRODUCERANDCONSUMER_H
#define __PRODUCERANDCONSUMER_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
 
//2个生产者，每个生产者工作6次
#define Need_Producer 2
#define Works_Producer 6
 
//3个消费者，每个消费者工作4次
#define Need_Consumer 3
#define Works_Consumer 4
 
//缓冲区为3
#define buffer_len 3
 
#define MYBUF_LEN (sizeof(struct mybuffer))
 
#define SHM_MODE 0600//可读可写
#define SEM_ALL_KEY 1234
#define SEM_EMPTY 0
#define SEM_FULL 1
 
//sharable memory buffer
struct mybuffer
{
	char str[buffer_len];
	int head;
	int tail;
	int is_empty;
};
 
//get a random number between [0,10]
int get_random()
{
	int digit;
	srand((unsigned)(getpid() + time(NULL)));
	digit = rand() % 10;
	return digit;
}

//print current time
void show_time()
{
	time_t now;
    struct tm *timenow;
	time(&now);
    timenow=localtime(&now);
	printf("%02d:%02d:%02d ",timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
}
 
//P操作
void P(int sem_id, int sem_num)
{
	struct sembuf xx;
	xx.sem_num = sem_num;//信号量的索引
	xx.sem_op = -1;//信号量的操作值
	xx.sem_flg = 0;//访问标志
 	semop(sem_id,&xx,1);//一次需进行的操作的数组sembuf中的元素数为1
}
 
//V操作
void V(int sem_id, int sem_num)
{
	struct sembuf xx;
	xx.sem_num = sem_num;
	xx.sem_op = 1;
	xx.sem_flg = 0;
	semop(sem_id,&xx,1);
}
 
#endif