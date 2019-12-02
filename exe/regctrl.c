/*++
Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    regctrl.c

Abstract: 

    Invokes the usermode and kernel mode callback samples.

Environment:

    User mode Win32 console application

Revision History:

--*/

#include "regctrl.h"
#include <stdio.h>
HANDLE g_Driver;
HKEY g_RootKey;

int ParseF(char buf[])
{
	HKEY key;
	char Reget[256] = { 0 };
	DWORD dw = 256;
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\SuperProgram", 0, KEY_READ, &key) != ERROR_SUCCESS)
	{
		printf("RegCreateKey error"); return 0;
	}
	if (RegQueryValueExA(key, "conf", NULL, NULL, (LPBYTE)&Reget, &dw) != ERROR_SUCCESS)
	{
		printf("RegSetValueEx error"); return 0;
	}
	if (RegCloseKey(key) != ERROR_SUCCESS)

	printf("RegCloseKey error");
	printf("\n%s\n", Reget);
	int count_exe = 0;
	int i2 = 1;
	if (Reget[0] != '<')
	{
		printf("input govno\n");
		return 0;
	}
	int i = 0;
	if (Reget[1] == 'e')
		i = 6;
	else
		i = 5;
	for (i; i < strlen(Reget) - 6; i++)
	{
		while (Reget[i] != '>') {
			buf[i2] = Reget[i];
			i++; i2++;
		}
		buf[i2] = '|'; i2++;
		i += 6;
		buf[count_exe] = Reget[i];
		i += 7;
		count_exe = i2; i2++;

		if (Reget[i + 2] == 'e' && Reget[i + 1] == '/') {
			i += 12; i2--;
			buf[i2] = '!';
			i2++; count_exe = i2; i2++;
			for (i; i < strlen(Reget) - 6; i++)
			{

				while (Reget[i] != '>') {
					buf[i2] = Reget[i];
					i++; i2++;
				}
				buf[i2] = '|'; i2++;
				i += 6;
				buf[count_exe] = Reget[i];
				i += 7;
				count_exe = i2; i2++;
			}
		}
	}
	return i2;
}


VOID __cdecl
wmain(
    _In_ ULONG argc,
    _In_reads_(argc) LPCWSTR argv[]
    )
{

    BOOL Result;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    Result = UtilLoadDriver(DRIVER_NAME,
                            DRIVER_NAME_WITH_EXT,
                            WIN32_DEVICE_NAME,
                            &g_Driver);

    if (Result != TRUE) {
        ErrorPrint("UtilLoadDriver failed, exiting...");
        exit(1);
    }

	char file[200] = { 0 };
	int len = ParseF(file);
	DeviceIoControl(g_Driver,
		IOCTL_DO_KERNELMODE_SAMPLES,
		file,
		len,
		0,
		0,
		0,
		NULL);
	char p[2] = { 0 };
	scanf("%s", &p);
	while (p[0] == '+' || p[0] == '-')
	{
		DeviceIoControl(g_Driver,
			IOCTL_DO_KERNELMODE_SAMPLES,
			p,
			2,
			0,
			0,
			0,
			NULL);
		scanf("%s", &p);
	}
	UtilUnloadDriver(g_Driver, NULL, DRIVER_NAME);
	
	system("pause");
}


