#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <utime.h>
//整体思路同windows非常相似
void Mycp(char *fsource, char *ftarget);       //将源目录信息复制到目标目录下
void CopyFile(char *fsource, char *ftarget);  //直接复制
											  //各种函数原型的应用，参数的设置，查找很多函数资料
int main(int argc, char *argv[])
{
	struct stat statbuf;     //文件状态结构
	struct utimbuf timeby;  //文件时间结构

	DIR * dir;             //DIR结构的指针，指向目录的第一个文件
	if (argc != 3)          //参数出错
	{
		printf("ARGC ERROR!\n");
	}
	else
	{
		if ((dir = opendir(argv[1])) == NULL)
		{
			//printf("源文件打开出错。\n"); 
		
		}

		stat(argv[1], &statbuf);
		timeby.actime = statbuf.st_atime;   //修改时间属性，存取时间
		timeby.modtime = statbuf.st_mtime;  //修改时间

		if ((dir = opendir(argv[2])) == NULL)
		{
			

			mkdir(argv[2], statbuf.st_mode);    //创建目录

			

		}

		Mycp(argv[1], argv[2]);    //开始复制
		utime(argv[2], &timeby);
	}
	printf("Copy Finished!\n");
	return 0;
}

void Mycp(char *fsource, char *ftarget)
{
	char source[512];
	char target[512];

	char buf[512];

	struct stat statbuf, statebuf1;
	struct utimbuf timeby;
	DIR *dir;
	struct dirent * entry;

	strcpy(source, fsource);
	strcpy(target, ftarget);



	dir = opendir(source);     //打开目录,返回指向DIR结构的指针

	while ((entry = readdir(dir)) != NULL)  //读目录

	{
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)   //判断目录
		{
			continue;
		}
		if (entry->d_type == 4)//是文件夹 递归复制 
		{
			strcat(source, "/");
			strcat(source, entry->d_name);
			strcat(target, "/");
			strcat(target, entry->d_name);

			stat(source, &statbuf);   //统计文件属性信息

			mkdir(target, statbuf.st_mode);  //创建目标目录


			Mycp(source, target);
			timeby.actime = statbuf.st_atime;
			timeby.modtime = statbuf.st_mtime;  //修改文件存取和修改时间
			utime(target, &timeby);
			strcpy(source, fsource);
			strcpy(target, ftarget);
		}
		else     //直接复制文件 
		{
			strcat(source, "/");
			strcat(source, entry->d_name);
			strcat(target, "/");
			strcat(target, entry->d_name);
			CopyFile(source, target);
			strcpy(source, fsource);
			strcpy(target, ftarget);
		}
	}
}

void CopyFile(char *fsource, char *ftarget)
{
	int fd = open(fsource, 0);   //打开文件，文件描述符
	int fdr;   //目标文件描述符
	char buf[512];
	struct stat statbuf, statebuf1;
	struct utimbuf timeby;
	struct timeval times[2];

	lstat(fsource, &statebuf1);
	if (S_ISLNK(statebuf1.st_mode))//是软链接 
	{
		int rslt = readlink(fsource, buf, 511);
		buf[rslt] = '\0';
		symlink(buf, ftarget);

		times[0].tv_sec = statebuf1.st_atime;
		times[0].tv_usec = 0;
		times[1].tv_sec = statebuf1.st_mtime;
		times[1].tv_usec = 0;
		lutimes(ftarget, times);
		return;
	}


	char BUFFER[1024];    //新开缓冲区，保存数据
	int wordbit;
	stat(fsource, &statbuf);

	fdr = creat(ftarget, statbuf.st_mode);    //创建新文件,返回文件描述符

	while ((wordbit = read(fd, BUFFER, 1024)) > 0)   //读取源文件字节数>0
	{
		write(fdr, BUFFER, wordbit);   //写入目标文件
	}
	timeby.actime = statbuf.st_atime;  //修改时间属性
	timeby.modtime = statbuf.st_mtime;
	utime(ftarget, &timeby);
	close(fd);  //关闭文件
	close(fdr);

}



