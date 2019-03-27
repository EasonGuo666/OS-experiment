#include "producerAndConsumer.h"
 
int main(int argc, char *argv[])
{
	int i, j;
	int shm_id, sem_id;
	int num_Producer = 0, num_Consumer = 0;
	struct mybuffer *shmptr;
 
	pid_t pid_p, pid_c;
 
	printf("parrent process starts! \n");
	//create a semaphore set including 2 semaphore
	sem_id = semget(SEM_ALL_KEY,2,IPC_CREAT|0660);
	if (sem_id < 0)
	{
		printf("create semaphores failed!\n");
	}
	
	//create 2 semaphores with SEM_EMPTY and SEM_FULL as their indexs.
	semctl(sem_id, SEM_EMPTY, SETVAL, buffer_len);
	semctl(sem_id, SEM_FULL, SETVAL, 0);
 
	//apply for sharable memory return -1 when failed.
	shm_id = shmget(IPC_PRIVATE, MYBUF_LEN, SHM_MODE);
	if (shm_id < 0)
	{
		printf("fail to allocate sharable memory!\n");
		exit(1);
	}
 
	//map the sharable memory with process memory space and return the virtual address
	shmptr = shmat(shm_id, 0, 0);
	if (shmptr == (void *)-1)
	{
		printf("mapping process failed！\n");
		exit(1);
	}
 
	shmptr->head = 0;
	shmptr->tail = 0;
	shmptr->is_empty = 1;
	
	//create producer subprocess in total
	while ((num_Producer++) < Need_Producer)
	{
		pid_p = fork();
		if (pid_p < 0)
		{
			printf("create producer process failed\n");
			exit(1);
		}

		//create producer subprocess
		if (pid_p == 0)
		{
			//map the sharable memory with process memory space and return the virtual address
			shmptr = shmat(shm_id, 0, 0);
			if (shmptr == (void *)-1)
			{
				printf("mapping process failed！\n");
				exit(1);
			}
		 	for (i=0; i<Works_Producer; i++)
			{
				P(sem_id, SEM_EMPTY);
				sleep(get_random());

				shmptr->str[shmptr->tail] = '1';
				shmptr->tail = (shmptr->tail + 1) % buffer_len;
				shmptr->is_empty = 0;

				show_time();

				printf("producer%d ",num_Producer);
				printf("buffer:");
				j = (shmptr->tail-1 >= shmptr->head) ? (shmptr->tail-1) : (shmptr->tail-1+buffer_len);
				for (; !(shmptr->is_empty) && j >= shmptr->head; j--)
				{
					printf("%c ", shmptr->str[j%buffer_len]);
				}
				printf("\n");
				
				fflush(stdout);
				V(sem_id,SEM_FULL);
			}
			//detach the mapping between process and the sharable memory
			shmdt(shmptr);
			exit(0);
		}
	}

	//create 3 consumer subprocess in total
	while ((num_Consumer++) < Need_Consumer)
	{
		pid_c = fork();
		if (pid_c < 0)
		{
			printf("create consumer process failed！\n");
			exit(1);
		}
		
		//create consumer subprocess
		if (pid_c == 0)
		{
			//map the sharable memory with process memory space and return the virtual address
			shmptr = shmat(shm_id, 0, 0);
			if (shmptr == (void *)-1)
			{
				printf("mapping process failed！\n");
				exit(1);
			}
		 	for (i=0; i<Works_Consumer; i++)
			{
				P(sem_id, SEM_FULL);
				sleep(get_random());

				shmptr->head = (shmptr->head + 1) % buffer_len;
				shmptr->is_empty = (shmptr->head == shmptr->tail);
 
				//print the current time
				show_time();

				printf("consumer%d ",num_Consumer);
				printf("buffer:");
				j = (shmptr->tail-1 >= shmptr->head) ? (shmptr->tail-1) : (shmptr->tail-1+buffer_len);
				for (; !(shmptr->is_empty) && j >= shmptr->head; j--)
				{
					printf("%c ", shmptr->str[j%buffer_len]);
				}
				printf("\n");
				fflush(stdout);
				V(sem_id,SEM_EMPTY);
			}
			//detach the mapping between process and the sharable memory
			shmdt(shmptr);
			exit(0);
		}
	}
 
	//parrent process exit here
	while (wait(0) != -1);
	
	//detach the sharable memory with parrent process
	shmdt(shmptr);
	
	//delete the sharable memory paragraph
	shmctl(shm_id,IPC_RMID,0);

	//delete the semaphores
	shmctl(sem_id,IPC_RMID,0);

	printf("parent process is done!\n");
	fflush(stdout);
	exit(0);
}