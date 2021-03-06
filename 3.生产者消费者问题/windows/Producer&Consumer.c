#include<stdio.h>
#include<stdlib.h>
#include<windows.h>
#include<time.h>

static HANDLE hMutexMapping = INVALID_HANDLE_VALUE;
int num = 0;
HANDLE lpHandle[10];

struct buf
{
	int num;
	int read;
	int write;
	int buffer[5];
};

BOOL StartClone()
{
	int i;
	BOOL bCreateOK;
	PROCESS_INFORMATION pi;
	TCHAR szFilename[MAX_PATH];
	GetModuleFileName(NULL, szFilename, MAX_PATH);
	TCHAR szCmdLine[MAX_PATH];
	for (i = 0; i < 2; i++)
	{
		sprintf_s(szCmdLine, "\"%s\" Producer %d", szFilename, i);
		STARTUPINFO si;
		ZeroMemory(reinterpret_cast<void *>(&si), sizeof(si));
		si.cb = sizeof(si);
		bCreateOK = CreateProcess(szFilename, szCmdLine, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi);
		if (!bCreateOK)
		{
			return false;
		}
		else
		{
			printf("producer %d is created\n", i + 1);
			lpHandle[num] = pi.hProcess;
			num++;
		}
	}
	for (i = 0; i < 3; i++)
	{
		sprintf_s(szCmdLine, "\"%s\" consumer %d", szFilename, i);
		STARTUPINFO si;
		ZeroMemory(reinterpret_cast<void *>(&si), sizeof(si));
		si.cb = sizeof(si);
		bCreateOK = CreateProcess(szFilename, szCmdLine, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi);
		if (!bCreateOK)
		{
			return false;
		}
		else
		{
			printf("consumer %d is created\n", i + 1);
			lpHandle[num] = pi.hProcess;
			num++;
		}
	}
	return true;
}

void Parent()
{
	hMutexMapping = CreateMutex(NULL, true, "mutex");

	//创建共享内存区的文件映射
	HANDLE hMapping = CreateFileMapping(NULL, NULL, PAGE_READWRITE, 0, sizeof(LONG), "map");
	if (hMapping != INVALID_HANDLE_VALUE)
	{
		LPVOID pData = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (pData != NULL)
		{
			ZeroMemory(pData, sizeof(LONG));
		}
		struct buf *pnData = reinterpret_cast<struct buf *>(pData);
		pnData->read = 0;
		pnData->write = 0;
		pnData->num = 0;
		memset(pnData->buffer, 0, sizeof(pnData->buffer));
		UnmapViewOfFile(pData);
	}
	CreateSemaphore(NULL, 3, 3, "EMPTY");
	CreateSemaphore(NULL, 0, 3, "FULL");
	//创建5个子进程
	StartClone();
	ReleaseMutex(hMutexMapping);
}

void Producer(int n)
{
	int j;
	hMutexMapping = OpenMutex(MUTEX_ALL_ACCESS, true, "mutex");
	HANDLE hMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "map");
	if (hMapping == INVALID_HANDLE_VALUE)
	{
		printf("error\n");
	}
	HANDLE semEmpty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "EMPTY");
	HANDLE semFull = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "FULL");
	for (int i = 0; i < 6; i++)
	{
		WaitForSingleObject(semEmpty, INFINITE);
		srand((unsigned)time(0));
		Sleep(rand() / 6);
		WaitForSingleObject(hMutexMapping, INFINITE);
		LPVOID pFile = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (pFile != NULL)
		{
			struct buf *pnData = reinterpret_cast<struct buf *>(pFile);
			pnData->buffer[pnData->write] = 1;
			pnData->write = (pnData->write + 1) % 3;
			pnData->num++;
			SYSTEMTIME st;
			GetSystemTime(&st);
			printf("%02d:%02d:%02d producer[%d] buffer:", (st.wHour + 8) % 24, st.wMinute, st.wSecond, n);
			for (j = 0; j < 3; j++)
			{
				printf("%d ", pnData->buffer[j]);
			}
			printf("\n");
		}
		UnmapViewOfFile(pFile);
		pFile = NULL;
		ReleaseSemaphore(semFull, 1, NULL);
		ReleaseMutex(hMutexMapping);
	}
	//printf("Producer[%d] is done\n", n);
}

void Consumer(int n)
{
	int j;
	hMutexMapping = OpenMutex(MUTEX_ALL_ACCESS, true, "mutex");
	HANDLE hMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "map");
	if (hMapping == INVALID_HANDLE_VALUE)
	{
		printf("error\n");
	}
	HANDLE semEmpty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "EMPTY");
	HANDLE semFull = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "FULL");
	for (int i = 0; i < 4; i++)
	{
		WaitForSingleObject(semFull, INFINITE);
		srand((unsigned)time(0));
		Sleep(rand() / 6);
		WaitForSingleObject(hMutexMapping, INFINITE);
		LPVOID pFile = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (pFile != NULL)
		{
			struct buf *pnData = reinterpret_cast<struct buf *>(pFile);
			pnData->buffer[pnData->read] = 0;
			pnData->read = (pnData->read + 1) % 3;
			pnData->num--;
			SYSTEMTIME st;
			GetSystemTime(&st);
			printf("%02d:%02d:%02d consumer[%d] buffer:", (st.wHour + 8) % 24, st.wMinute, st.wSecond, n);
			for (j = 0; j < 3; j++)
			{
				printf("%d ", pnData->buffer[j]);
			}
			printf("\n");
		}
		UnmapViewOfFile(pFile);
		pFile = NULL;
		ReleaseSemaphore(semEmpty, 1, NULL);
		ReleaseMutex(hMutexMapping);
	}
	//printf("consumer[%d] is done\n", n);
}

int main(int argc, char **argv)
{
	if (argc>1 && strcmp(argv[1], "Producer") == 0)
	{
		Producer(atoi(argv[2]));
	}
	else if (argc>1 && strcmp(argv[1], "consumer") == 0)
	{
		Consumer(atoi(argv[2]));
	}
	else
	{
		Parent();
		WaitForMultipleObjects(num, lpHandle, true, INFINITE);
	}
	return 0;
}