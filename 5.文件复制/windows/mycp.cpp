#include "stdafx.h"
#include <iostream>
#include <string.h>
#include<Windows.h>
#include<Windowsx.h>

using namespace std;

/*
查找指定文件路径的文件FindFirstFile()函数
HANDLE FindFirstFile(LPCTSTR lpFileName, //file name
LPWIN32_FIND_DATA lpFindFileData	//data buffer
);
返回值：如果调用成功，返回值为非0，否则返回值为0.
*/

//直接文件复制
void CopyFile(char *fsource, char *ftarget)
{
	WIN32_FIND_DATA lpfindfiledata;
	//查找指定文件路径             //源文件名   //数据缓冲区
	HANDLE hfind = FindFirstFile(fsource, &lpfindfiledata);
						//指向源文件名//指定对象的访问类型(实现读写)//与其他进程共享文件的方式//如果文件存在则打开文件，否则创建新的文件//指定文件的属性和标志(FILE_ATTRIBUTE_NORMAL设置为默认属性)
	HANDLE hsource = CreateFile(fsource, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,NULL, OPEN_ALWAYS,                           FILE_ATTRIBUTE_NORMAL,                                  NULL);
						//指向目标文件名						//指向安全属性的指针(NULL表示默认安全对象)//CREATE_ALWAYS为创建一个新文件，如果存在会将现有的文件清空且重写文件)
	HANDLE htarget = CreateFile(ftarget,  GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,NULL, CREATE_ALWAYS,  FILE_ATTRIBUTE_NORMAL, NULL);
																							//指定文件的属性和标志(FILE_ATTRIBUTE_NORMAL设置为默认属性)
	//源文件的大小
	LONG size = lpfindfiledata.nFileSizeLow - lpfindfiledata.nFileSizeHigh;

	DWORD wordbit;

	//新开缓冲区，保存数据
	int *BUFFER = new int[size];

	//源文件读数据//指定要读的文件句柄//指向存放从文件读的数据的缓冲区的地址指针//要从文件读的字节数//存放实际从文件中读的字节数的变量地址
	ReadFile(    hsource,          BUFFER,                                size,             &wordbit,         NULL);//同步方式打开设置为NULL

	//目标文件写数据//指定要写的文件对象的句柄//指向要写入文件的数据缓冲区指针//要写入文件的字节数//实际写入的字节数
	WriteFile(     htarget,                BUFFER,                      size,             &wordbit,  NULL);//同步方式打开设置为NULL

	//修改目标的创建时间
	SetFileTime(htarget,
				&lpfindfiledata.ftCreationTime,
				&lpfindfiledata.ftLastAccessTime,
				&lpfindfiledata.ftLastWriteTime);


	CloseHandle(hfind);
	CloseHandle(hsource);
	CloseHandle(htarget);
}

//将源目录信息复制到目标目录下
void mycp(char *fsource, char *ftarget)
{
	WIN32_FIND_DATA lpfindfiledata;
	char source[100];
	char target[100];
	lstrcpy(source, fsource);
	lstrcpy(target, ftarget);
	lstrcat(source, "\\*.*");
	lstrcat(target, "\\");
	HANDLE hfind = FindFirstFile(source, &lpfindfiledata);//数据缓冲区
	if (hfind != INVALID_HANDLE_VALUE)
	{
		while (FindNextFile(hfind, &lpfindfiledata) != 0)//循环查找FindFirstFile()函数搜索后的下一个文件
		{
			//查找下一个文件成功
			if ((lpfindfiledata.dwFileAttributes) == FILE_ATTRIBUTE_DIRECTORY)//判断是否是目录(若为目录FILE_ATTRIBUTE_DIRECTORY是16)
			{
				if ((strcmp(lpfindfiledata.cFileName, ".") != 0) && (strcmp(lpfindfiledata.cFileName, "..") != 0))
				{
					memset(source, '0', sizeof(source));
					lstrcpy(source, fsource);
					lstrcat(source, "\\");
					lstrcat(source, lpfindfiledata.cFileName);//追加文件
					lstrcat(target, lpfindfiledata.cFileName);
					CreateDirectory(target, NULL);//为目标文件创建目录
					
					HANDLE hsource = CreateFile(source, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
					HANDLE htarget = CreateFile(target, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

					GetFileTime(hsource,
						&lpfindfiledata.ftCreationTime,
						&lpfindfiledata.ftLastAccessTime,
						&lpfindfiledata.ftLastWriteTime);
					//修改目标目录的创建时间
					SetFileTime(htarget,
						&lpfindfiledata.ftCreationTime,
						&lpfindfiledata.ftLastAccessTime,
						&lpfindfiledata.ftLastWriteTime);

					mycp(source, target);//进入子目录复制
					lstrcpy(source, fsource);
					lstrcat(source, "\\");
					lstrcpy(target, ftarget);
					lstrcat(target, "\\");
				}
			}
			else//当前文件不是目录
			{
				memset(source, '0', sizeof(source));
				lstrcpy(source, fsource);
				lstrcat(source, "\\");
				lstrcat(source, lpfindfiledata.cFileName);
				lstrcat(target, lpfindfiledata.cFileName);
				CopyFile(source, target);//直接调用文件复制函数
				lstrcpy(source, fsource);
				lstrcat(source, "\\");
				lstrcpy(target, ftarget);
				lstrcat(target, "\\");
			}
		}
	}
	else
	{
		printf("查找指定文件路径的文件失败!\n");
	}
}

int main(int argc, char *argv[])
{
	WIN32_FIND_DATA lpfindfiledata;

	//check the parameter number
	if (argc != 3)
	{
		printf("parameter number is not 3!\n");
	}
	else
	{
		//check the existence of source file folder
		if (FindFirstFile(argv[1], &lpfindfiledata) == INVALID_HANDLE_VALUE)
		{
			printf("source file does not exist!\n");
		}
		if (FindFirstFile(argv[2], &lpfindfiledata) == INVALID_HANDLE_VALUE)
		{
			//创建目标文件夹
			CreateDirectory(argv[2], NULL);
			printf("create target folder!\n");
			HANDLE targetfolder = FindFirstFile(argv[2], &lpfindfiledata);
			HANDLE sourcefolder = FindFirstFile(argv[1], &lpfindfiledata);
			//修改目标目录的创建时间
			BOOL x = SetFileTime(targetfolder,
				&lpfindfiledata.ftCreationTime,
				&lpfindfiledata.ftLastAccessTime,
				&lpfindfiledata.ftLastWriteTime);
		}
		mycp(argv[1], argv[2]);
	}
	printf("复制完成!\n");
	getchar();
	return 0;
}

