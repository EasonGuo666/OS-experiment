#include "stdafx.h"
#include <iostream>
#include <string.h>
#include <windows.h>
using namespace std;

int tran_string_to_int(char string[])
{
	int len = strlen(string);
	int temp_num = 0;
	for (int i = 0; i < len; i++)
	{
		if (string[i] >= '0'&&string[i] <= '9')
			temp_num = temp_num * 10 + (string[i] - '0');
		else
		{
			printf("error found in parameter, you should input only integar\n");
			return -1;
		}
	}
	return temp_num;
}

int main(int argc, char **argv)
{
	int waitsecond = tran_string_to_int(argv[1]);
	Sleep(waitsecond);
	return 0;
}