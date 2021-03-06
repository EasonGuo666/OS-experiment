#include "stdafx.h"
#include <cstdio>
#include <cstdlib>
#include <iostream> 
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#include <iomanip>
#include <conio.h>
#pragma comment(lib,"Shlwapi.lib")
using namespace std;

//judge the protection type of a process
inline bool TestSet(DWORD dwTarget, DWORD dwMask);

//display the page protection type
void ShowProtection(DWORD dwTarget);

//display the physical and virtual memory status
void ShowMemoryStatus(MEMORYSTATUSEX statex);

//display the usage status of system physical paged memmory
void ShowUsageStatus(PERFORMANCE_INFORMATION pi);

//Traverse entire virtual memory and displays the properties for each memory region of the working program  
void WalkVM(HANDLE hProcess);

//Get the position of cursor in console
void GetConsoleCursorPosition(int &x, int &y);

//move the cursor to (x,y)
void gotoxy(int x, int y);

int main(int argc, char* argv[])
{
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	//get the usage of memory
	GlobalMemoryStatusEx(&statex);
	ShowMemoryStatus(statex);
	
	PERFORMANCE_INFORMATION pi;
	pi.cb = sizeof(pi);
	//get the usage status of system physical paged memmory
	GetPerformanceInfo(&pi, sizeof(pi));
	ShowUsageStatus(pi);

	//Search the Memory Information Of A Process
	printf("----------------Search the Memory Information Of A Process-----------------\n");
	PROCESSENTRY32 pe;
	//pe.dwSize = sizeof(pe);
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	BOOL bMore = ::Process32First(hProcessSnap, &pe);

	printf("input process name:");
	char process_name[100];
	cin >> process_name;
	while (bMore)
	{
		HANDLE hP = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
		PROCESS_MEMORY_COUNTERS pmc;
		ZeroMemory(&pmc, sizeof(pmc));

		//search for the process we want to watch
		if (!strcmp(process_name, pe.szExeFile))
		{
			if (GetProcessMemoryInfo(hP, &pmc, sizeof(pmc)) == TRUE)
			{
				cout << "process ID:";
				wcout << pe.th32ProcessID << endl;
				cout << "process name:";
				wcout << pe.szExeFile << endl;
				cout << "virtual memory size:" << (float)pmc.WorkingSetSize / 1024 << "KB" << endl;
				cout << endl;
				WalkVM(hP);
				break;
			}
		}
		else//if the process bMore pointing at is not the process we search, move to the next one
		{
			bMore = ::Process32Next(hProcessSnap, &pe);
		}
	}
	printf("\n\n");

	
	//int ID;
	//cin >> ID;
	//HANDLE hP = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ID);
	//WalkVM(hP);
	getchar();
	getchar();
	return 0;
}

//display the protection type of each process
inline bool TestSet(DWORD dwTarget, DWORD dwMask)
{
	return ((dwTarget &dwMask) == dwMask);
}

//display the page protection type
void ShowProtection(DWORD dwTarget)
{
	if (TestSet(dwTarget, PAGE_READONLY))
		cout << "," << "READONLY";
	if (TestSet(dwTarget, PAGE_GUARD))
		cout << "," << "GUARD";
	if (TestSet(dwTarget, PAGE_NOCACHE))
		cout << "," << "NOCACHE";
	if (TestSet(dwTarget, PAGE_READWRITE))
		cout << "," << "READWRITE";
	if (TestSet(dwTarget, PAGE_WRITECOPY))
		cout << "," << "WRITECOPY";
	if (TestSet(dwTarget, PAGE_EXECUTE))
		cout << "," << "EXECUTE";
	if (TestSet(dwTarget, PAGE_EXECUTE_READ))
		cout << "," << "EXECUTE_READ";
	if (TestSet(dwTarget, PAGE_EXECUTE_READWRITE))
		cout << "," << "EXECUTE_READWRITE";
	if (TestSet(dwTarget, PAGE_EXECUTE_WRITECOPY))
		cout << "," << "EXECUTE_WRITECOPY";
	if (TestSet(dwTarget, PAGE_NOACCESS))
		cout << "," << "NOACCESS";
}

//Traverse entire virtual memory and displays the properties for each memory region of the working program
void WalkVM(HANDLE hProcess)
{
	SYSTEM_INFO si;	//系统信息结构
	ZeroMemory(&si, sizeof(si));	//初始化
	GetSystemInfo(&si);	//获得系统信息

	MEMORY_BASIC_INFORMATION mbi;	//进程虚拟内存空间的基本信息结构
	ZeroMemory(&mbi, sizeof(mbi));	//分配缓冲区，用于保存信息

	//timeslot for refreshing, after 5s break from while
	int timeslot = 10000;

	clock_t start, end;
	start = clock();
	
	//int x = 0, y = 0;
	//get the original start position of cursor
	//GetConsoleCursorPosition(x, y);
/*
	while (1)
	{
		end = clock();
		if (end - start > timeslot)
		{
			break;
		}*/

		LPCVOID pBlock = (LPVOID)si.lpMinimumApplicationAddress;
		//循环整个应用程序地址空间
		while (pBlock < si.lpMaximumApplicationAddress)
		{
			//获得下一个虚拟内存块的信息
			if (VirtualQueryEx(hProcess, pBlock, &mbi, sizeof(mbi)) == sizeof(mbi))
			{
				//计算块的结尾及其长度
				LPCVOID pEnd = (PBYTE)pBlock + mbi.RegionSize;
				TCHAR szSize[MAX_PATH];
				//将数字转换成字符串
				StrFormatByteSize(mbi.RegionSize, szSize, MAX_PATH);

				//显示块地址和长度
				cout.fill('0');
				cout << hex << setw(8) << (DWORD)pBlock << "-" << hex << setw(8) << (DWORD)pEnd << (strlen(szSize) == 7 ? "(" : "(") << szSize << ")";

				//display the status of each memory block
				switch (mbi.State)
				{
				case MEM_COMMIT:
					printf("Committed");
					break;
				case MEM_FREE:
					printf("Free");
					break;
				case MEM_RESERVE:
					printf("Reserved");
					break;
				}

				//显示保护
				if (mbi.Protect == 0 && mbi.State != MEM_FREE)
				{
					mbi.Protect = PAGE_READONLY;
				}
				ShowProtection(mbi.Protect);

				//显示类型
				switch (mbi.Type)
				{
				case MEM_IMAGE:
					printf(", Image");
					break;
				case MEM_MAPPED:
					printf(", Mapped");
					break;
				case MEM_PRIVATE:
					printf(", Private");
					break;
				}

				//检验可执行的映像
				//TCHAR szFilename[MAX_PATH];
				//if (GetModuleFileName((HMODULE)pBlock, szFilename, MAX_PATH) > 0)				//实际使用的缓冲区长度
				//{
				//	//除去路径并显示
				//	PathStripPath(szFilename);
				//	printf(", Module:%s", szFilename);
				//}
				printf("\n");

				//移动块指针以获得下一个块
				pBlock = pEnd;

			}//if (VirtualQueryEx(hProcess, pBlock, &mbi, sizeof(mbi)) == sizeof(mbi))
		
		}//while (pBlock < si.lpMaximumApplicationAddress)
		
		//move the cursor back to where it used to be.
		//gotoxy(x, y);
		//system("cls");
	//}
}

//display the physical and virtual memory status
void ShowMemoryStatus(MEMORYSTATUSEX statex)
{
	printf("----------------Virtual Memory && Physical Memory Information--------------\n");
	printf("Physical Memory:\n");
	printf("usage rate:%ld%%\n", statex.dwMemoryLoad);
	printf("total capacity: %.2fGB.\n", (float)statex.ullTotalPhys / 1024 / 1024 / 1024);
	printf("available size: %.2fGB.\n\n", (float)statex.ullAvailPhys / 1024 / 1024 / 1024);
	printf("Virtual Memory:\n");
	printf("total capacity of virtual memory: %.2fGB.\n", (float)statex.ullTotalVirtual / 1024 / 1024 / 1024);
	printf("available size: %.2fGB.\n\n", (float)statex.ullAvailVirtual / 1024 / 1024 / 1024);
	printf("Page:\n");
	printf("total page file:%.2fGB.\n", (float)statex.ullTotalPageFile / 1024 / 1024 / 1024);
	printf("available page file：%.2fGB.\n\n", (float)statex.ullAvailPageFile / 1024 / 1024 / 1024);
	//printf("保留字段的容量为：%.2fByte.\n", statex.ullAvailExtendedVirtual);
	printf("\n\n");
}

//display the usage status of system physical paged memmory
void ShowUsageStatus(PERFORMANCE_INFORMATION pi)
{
	printf("----------------------------Main Memory Usage------------------------------\n");
	cout << "physical paged memory: " << pi.PhysicalTotal << endl;
	cout << "physical available: " << pi.PhysicalAvailable << endl;
	cout << "Cache: " << pi.SystemCache << endl;
	//cout << "commit total: " << pi.CommitTotal << endl;
	//cout << "max number of commit page total: " << pi.CommitLimit << endl;
	//cout << "system committed page peak: " << pi.CommitPeak << endl;
	cout << "kernel total: " << pi.KernelTotal << endl;
	cout << "kernel paged: " << pi.KernelPaged << endl;
	cout << "kernel nonpaged: " << pi.KernelNonpaged << endl;
	cout << "page size: " << pi.PageSize << endl;
	cout << "handle count: " << pi.HandleCount << endl;
	cout << "process count: " << pi.ProcessCount << endl;
	cout << "thread count: " << pi.ThreadCount << endl;
	printf("\n\n");
}

//get the position of cursor in console
void GetConsoleCursorPosition(int &x, int &y)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); 
	//COORD coordScreen = { 0, 0 }; 
	//光标位置 
	CONSOLE_SCREEN_BUFFER_INFO csbi; 
	if (GetConsoleScreenBufferInfo(hConsole, &csbi))
	{ 
		printf("光标坐标:(%d,%d)\n", csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y); 
		x = csbi.dwCursorPosition.X;
		y = csbi.dwCursorPosition.Y;
	}
}

//move the cursor to (x,y)
void gotoxy(int x, int y)
{
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	HANDLE hConsoleOut;
	hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hConsoleOut, &csbiInfo);
	csbiInfo.dwCursorPosition.X = x;
	csbiInfo.dwCursorPosition.Y = y;
	SetConsoleCursorPosition(hConsoleOut, csbiInfo.dwCursorPosition);
}