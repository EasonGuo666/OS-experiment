#include "stdafx.h"
#include <iostream>  
#include <windows.h>  
using namespace std;

//打印时间函数
void ShowTime(SYSTEMTIME start, SYSTEMTIME end);

int main(int argc, char *argv[])
{
	SYSTEMTIME start, end;

	//启动设置
	STARTUPINFO si;
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;
	
	//拼接字符串给子进程传参数
	char subprogram_parameter[100] = { "\0" };
	strcat_s(subprogram_parameter, sizeof(subprogram_parameter), argv[1]);

	if (argv[2] != NULL)
	{
		strcat_s(subprogram_parameter, sizeof(subprogram_parameter), " ");
		strcat_s(subprogram_parameter, sizeof(subprogram_parameter), argv[2]);
	}
	
	//创建进程pi
	PROCESS_INFORMATION pi;

	//if (!CreateProcess(NULL, argv[1], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
	if (!CreateProcess(NULL, subprogram_parameter, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
	{
		cout << "Create Fail!" << endl;
		exit(1);
	}
	else
	{
		printf("mainprogram argv[2]: %s\n", argv[2]);
		printf("subprogram_parameter: %s\n", subprogram_parameter);
		cout << "Success!" << endl;
		GetSystemTime(&start);
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	//计时终点
	GetSystemTime(&end);

	//打印时间
	ShowTime(start, end);

	return 0;
}

void ShowTime(SYSTEMTIME start, SYSTEMTIME end)
{
	int hour, minutes, seconds, milliseconds;
	milliseconds = end.wMilliseconds - start.wMilliseconds;
	seconds = end.wSecond - start.wSecond;
	minutes = end.wMinute - start.wMinute;
	hour = end.wHour - start.wHour;

	if (milliseconds < 0)
	{
		seconds--;
		milliseconds += 1000;
	}
	if (seconds < 0)
	{
		minutes--;
		seconds += 60;
	}
	if (minutes < 0)
	{
		hour--;
		minutes += 60;
	}
	printf("%d小时 %d分 %d秒 %d毫秒\n", hour, minutes, seconds, milliseconds);
}