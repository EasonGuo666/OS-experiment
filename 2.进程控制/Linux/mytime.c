#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

void show_time(struct timeval start, struct timeval end);

int main(int argc, char *argv[])
{
	int i;
	struct timeval start;
	struct timeval end;

	if (argc==1)
	{
		printf("Error!\n");  
        exit(0);  
    }  
    if (fork()==0)  
    {  
		char *exec_argv[4];
        for (i=0; i<argc; i++)  
        {  
            exec_argv[i]=argv[i+1];  
            printf("exec_argv[%d]:%s\n", i,exec_argv[i]);  
        }
		if (argv[2]!=NULL)
		{
			char p[50] = {"\0"};
			strcat(p, argv[1]);
			strcat(p, " ");
			strcat(p, argv[2]);
			exec_argv[0]=p;
		}

        printf("Child Create\n");  
        execvp(argv[1],exec_argv);
		 
    }  
    else 
    {
        gettimeofday( &start, NULL );
		wait(NULL);
        gettimeofday( &end, NULL );
		
		show_time(start, end);
    }  
    return 0;  
}

void show_time(struct timeval start, struct timeval end)
{
	int hour, minute, second, millisecond, microsecond;
	microsecond = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;

	millisecond = microsecond / 1000;
	microsecond %= 1000;

	second = millisecond / 1000;
	millisecond %= 1000;

	minute = second / 60;
	second %= 60;

	hour = minute / 60;
	minute %= 60;
	
	printf("time cost: %d hour %d minute %d second %d millisecond %d microsecond\n", hour, minute, second, millisecond, microsecond);
}
