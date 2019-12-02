
#include "regfltr.h"

HANDLE g_RootKey;


LPCWSTR
GetNotifyClassString(_In_ REG_NOTIFY_CLASS NotifyClass);

typedef NTSTATUS(*QUERY_INFO_PROCESS) (
	__in HANDLE ProcessHandle,
	__in PROCESSINFOCLASS ProcessInformationClass,
	__out_bcount(ProcessInformationLength) PVOID ProcessInformation,
	__in ULONG ProcessInformationLength,
	__out_opt PULONG ReturnLength
	);
QUERY_INFO_PROCESS ZwQueryInformationProcess;



NTSTATUS
GetProcessImageName(
	PEPROCESS eProcess,
	PUNICODE_STRING* ProcessImageName
)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG returnedLength;
	HANDLE hProcess = NULL;

	PAGED_CODE(); // this eliminates the possibility of the IDLE Thread/Process

	if (eProcess == NULL)
	{
		return STATUS_INVALID_PARAMETER_1;
	}

	status = ObOpenObjectByPointer(eProcess,
		0, NULL, 0, 0, KernelMode, &hProcess);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("ObOpenObjectByPointer Failed: %08x\n", status);
		return status;
	}

	if (ZwQueryInformationProcess == NULL)
	{
		UNICODE_STRING routineName = RTL_CONSTANT_STRING(L"ZwQueryInformationProcess");

		ZwQueryInformationProcess =
			(QUERY_INFO_PROCESS)MmGetSystemRoutineAddress(&routineName);

		if (ZwQueryInformationProcess == NULL)
		{
			DbgPrint("Cannot resolve ZwQueryInformationProcess\n");
			status = STATUS_UNSUCCESSFUL;
			goto cleanUp;
		}
	}

	/* Query the actual size of the process path */
	status = ZwQueryInformationProcess(hProcess,
		ProcessImageFileName,
		NULL, // buffer
		0,    // buffer size
		&returnedLength);

	if (STATUS_INFO_LENGTH_MISMATCH != status) {
		DbgPrint("ZwQueryInformationProcess status = %x\n", status);
		goto cleanUp;
	}

	*ProcessImageName = ExAllocatePoolWithTag(NonPagedPoolNx, returnedLength, '2gat');

	if (ProcessImageName == NULL)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto cleanUp;
	}

	/* Retrieve the process path from the handle to the process */
	status = ZwQueryInformationProcess(hProcess,
		ProcessImageFileName,
		*ProcessImageName,
		returnedLength,
		&returnedLength);

	if (!NT_SUCCESS(status)) ExFreePoolWithTag(*ProcessImageName, '2gat');

cleanUp:

	ZwClose(hProcess);

	return status;
}

char str[300];
void
PrintStringInFile()
{
	NTSTATUS status;
	UNICODE_STRING UnicodeFileName;
	OBJECT_ATTRIBUTES FileAttributes;
	HANDLE Handle;
	IO_STATUS_BLOCK IoStatusBlock;
	//переделаем строку с путем файла в юникод
	RtlInitUnicodeString(&UnicodeFileName, L"\\Device\\HarddiskVolume2\\Users\\nadia\\Desktop\\Lab\\text.txt");
	//получаем свойства дескриптора файла
	InitializeObjectAttributes(&FileAttributes, &UnicodeFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	//проверяем, можем ли открыть файл (текущий Irql драйвера)
	if (KeGetCurrentIrql() != PASSIVE_LEVEL)
		return;

	//открываем файл
	status = ZwCreateFile(&Handle,
		FILE_APPEND_DATA,
		&FileAttributes, &IoStatusBlock,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		0,
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL, 0);
	//если файл открыть удалось
	if (status == STATUS_SUCCESS)
	{
		ULONG buffer_size = (ULONG)strlen(str);
		status = ZwWriteFile(Handle, NULL, NULL, NULL, &IoStatusBlock,
			str, buffer_size, &GlobalByteOffset, NULL);
		GlobalByteOffset.LowPart += buffer_size;
		if (status == STATUS_SUCCESS)
			DbgPrint("[mydriver]: PrintStringInFile successfully!\n");
		else
			DbgPrint("[mydriver]: PrintStringInFile (ZwWriteFile) unsuccessfully!\n");
	}
	else
		DbgPrint("[mydriver]: PrintStringInFile (ZwCreateFile) unsuccessfully!\n");
	
	ZwClose(Handle);
}


VOID ProcessLoadImageCallback(
		_In_opt_ PUNICODE_STRING FullImageName,
		_In_ HANDLE ProcessId,                // pid into which image is being mapped
		_In_ PIMAGE_INFO ImageInfo
	)
{
	
		UNREFERENCED_PARAMETER(ImageInfo);
		LARGE_INTEGER SystemTime;
		TIME_FIELDS TimeFields;

		KeQuerySystemTime(&SystemTime);
		RtlTimeToTimeFields(&SystemTime, &TimeFields);
		PUNICODE_STRING processName = NULL;
		NTSTATUS status;
		status = GetProcessImageName(PsGetCurrentProcess(), &processName);

		sprintf(str, "[%d]Process Name: %ws\nProcess Id: %u\nFullImageName: %ws\nTime: %i : %i : %i\n", count_PsSet, processName->Buffer, PtrToInt(ProcessId),
			FullImageName->Buffer, TimeFields.Hour + 3, TimeFields.Minute, TimeFields.Second);
		count_PsSet++;

		PrintStringInFile();
		
	

}



NTSTATUS 
Callback (
    _In_     PVOID CallbackContext,
    _In_opt_ PVOID Argument1,
    _In_opt_ PVOID Argument2
)
{
	UNREFERENCED_PARAMETER(CallbackContext);
    NTSTATUS Status = STATUS_SUCCESS;
    REG_NOTIFY_CLASS NotifyClass;
	NTSTATUS status;
    NotifyClass = (REG_NOTIFY_CLASS)(ULONG_PTR)Argument1;


	PUNICODE_STRING processName = NULL;
	
	status = GetProcessImageName(PsGetCurrentProcess(), &processName);
	if (!NT_SUCCESS(status))
	{
		return STATUS_INVALID_PARAMETER;
	}
	__try 
	{
		int j = 0;
		int key = 0;
		int exe = 0;
		wchar_t my_exe[100] = { 0 };
		wchar_t no_open[100] = { 0 };
		while (exe < count_exe) 
		{
			j = 0;
			while (rules_exe[exe][j++] != '\0') my_exe[j - 1] = rules_exe[exe][j];
			if (wcsstr(processName->Buffer, my_exe) != NULL)
				break;
			exe++;
		}
		if (exe==count_exe)
			return STATUS_SUCCESS;
				
		REG_PRE_OPEN_KEY_INFORMATION* pRegPreCreateKey = (REG_PRE_OPEN_KEY_INFORMATION*)Argument2;
		if (pRegPreCreateKey != NULL) //хочет создать ключ (эта штука ловит и открытие ключей - одна и та же структура)
		{
				while (key < count_key)
				{
							j = 0;
							while (rules_key[key][j++] != '\0') no_open[j - 1] = rules_key[key][j];
							if (wcscmp(pRegPreCreateKey->CompleteName->Buffer, no_open) == 0)
							{
								if (rules_exe[exe][0] >= rules_key[key][0])
								{
									DbgPrint("[mydriver]Process %ws can open %ws \n", (processName->Buffer), no_open);
									return STATUS_SUCCESS;
								}
								else {
									DbgPrint("[mydriver]Process %ws can't open %ws \n", (processName->Buffer), no_open);
									return STATUS_ACCESS_DENIED;
								}
							}
							key++;
				}
				if (key == count_key)
				{
							
							return STATUS_SUCCESS;
				}
		}
					/*REG_PRE_CREATE_KEY_INFORMATION* pRegPreCreateKey2 = (REG_PRE_CREATE_KEY_INFORMATION*)Argument2;
					if (pRegPreCreateKey2 != NULL)
						DbgPrint("[mydriver]CREATE %ws\n", pRegPreCreateKey2->CompleteName->Buffer);*/
					//return STATUS_ACCESS_DENIED;
	}
	__except (status == STATUS_SUCCESS) { return status; }

    if (Argument2 == NULL) {
        ErrorPrint("\tCallback: Argument 2 unexpectedly 0. Filter will "
                    "abort and return success.");
        return STATUS_SUCCESS;
    }
    return Status;
    
}

LPCWSTR 
GetNotifyClassString (
    _In_ REG_NOTIFY_CLASS NotifyClass
    )

{
    switch (NotifyClass) {
        case RegNtPreDeleteKey:                 return L"RegNtPreDeleteKey";
        case RegNtPreSetValueKey:               return L"RegNtPreSetValueKey";
        case RegNtPreDeleteValueKey:            return L"RegNtPreDeleteValueKey";
        case RegNtPreSetInformationKey:         return L"RegNtPreSetInformationKey";
        case RegNtPreRenameKey:                 return L"RegNtPreRenameKey";
        case RegNtPreEnumerateKey:              return L"RegNtPreEnumerateKey";
        case RegNtPreEnumerateValueKey:         return L"RegNtPreEnumerateValueKey";
        case RegNtPreQueryKey:                  return L"RegNtPreQueryKey";
        case RegNtPreQueryValueKey:             return L"RegNtPreQueryValueKey";
        case RegNtPreQueryMultipleValueKey:     return L"RegNtPreQueryMultipleValueKey";
        case RegNtPreKeyHandleClose:            return L"RegNtPreKeyHandleClose";
        case RegNtPreCreateKeyEx:               return L"RegNtPreCreateKeyEx";
        case RegNtPreOpenKeyEx:                 return L"RegNtPreOpenKeyEx";
        case RegNtPreFlushKey:                  return L"RegNtPreFlushKey";
        case RegNtPreLoadKey:                   return L"RegNtPreLoadKey";
        case RegNtPreUnLoadKey:                 return L"RegNtPreUnLoadKey";
        case RegNtPreQueryKeySecurity:          return L"RegNtPreQueryKeySecurity";
        case RegNtPreSetKeySecurity:            return L"RegNtPreSetKeySecurity";
        case RegNtPreRestoreKey:                return L"RegNtPreRestoreKey";
        case RegNtPreSaveKey:                   return L"RegNtPreSaveKey";
        case RegNtPreReplaceKey:                return L"RegNtPreReplaceKey";

        case RegNtPostDeleteKey:                return L"RegNtPostDeleteKey";
        case RegNtPostSetValueKey:              return L"RegNtPostSetValueKey";
        case RegNtPostDeleteValueKey:           return L"RegNtPostDeleteValueKey";
        case RegNtPostSetInformationKey:        return L"RegNtPostSetInformationKey";
        case RegNtPostRenameKey:                return L"RegNtPostRenameKey";
        case RegNtPostEnumerateKey:             return L"RegNtPostEnumerateKey";
        case RegNtPostEnumerateValueKey:        return L"RegNtPostEnumerateValueKey";
        case RegNtPostQueryKey:                 return L"RegNtPostQueryKey";
        case RegNtPostQueryValueKey:            return L"RegNtPostQueryValueKey";
        case RegNtPostQueryMultipleValueKey:    return L"RegNtPostQueryMultipleValueKey";
        case RegNtPostKeyHandleClose:           return L"RegNtPostKeyHandleClose";
        case RegNtPostCreateKeyEx:              return L"RegNtPostCreateKeyEx";
        case RegNtPostOpenKeyEx:                return L"RegNtPostOpenKeyEx";
        case RegNtPostFlushKey:                 return L"RegNtPostFlushKey";
        case RegNtPostLoadKey:                  return L"RegNtPostLoadKey";
        case RegNtPostUnLoadKey:                return L"RegNtPostUnLoadKey";
        case RegNtPostQueryKeySecurity:         return L"RegNtPostQueryKeySecurity";
        case RegNtPostSetKeySecurity:           return L"RegNtPostSetKeySecurity";
        case RegNtPostRestoreKey:               return L"RegNtPostRestoreKey";
        case RegNtPostSaveKey:                  return L"RegNtPostSaveKey";
        case RegNtPostReplaceKey:               return L"RegNtPostReplaceKey";

        case RegNtCallbackObjectContextCleanup: return L"RegNtCallbackObjectContextCleanup";

        default:
            return L"Unsupported REG_NOTIFY_CLASS";
    }
}

