
#include "regfltr.h"
#define IOCTL_FROM_APP CTL_CODE(0x00008000,0x801,METHOD_BUFFERED,FILE_ANY_ACCESS)

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD     DeviceUnload;

_Dispatch_type_(IRP_MJ_CREATE)         DRIVER_DISPATCH DeviceCreate;
_Dispatch_type_(IRP_MJ_CLOSE)          DRIVER_DISPATCH DeviceClose;
_Dispatch_type_(IRP_MJ_CLEANUP)        DRIVER_DISPATCH DeviceCleanup;
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL) DRIVER_DISPATCH DeviceControl;

PDEVICE_OBJECT g_DeviceObj;
LARGE_INTEGER g_CmCookie = { 0 };

NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS Status;
    UNICODE_STRING NtDeviceName;
    UNICODE_STRING DosDevicesLinkName;
    UNICODE_STRING DeviceSDDLString;

    UNREFERENCED_PARAMETER(RegistryPath);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, 
               DPFLTR_ERROR_LEVEL,
               "RegFltr: DriverEntry()\n");

    RtlInitUnicodeString(&NtDeviceName, NT_DEVICE_NAME);
    RtlInitUnicodeString(&DeviceSDDLString, DEVICE_SDDL);

    Status = IoCreateDeviceSecure(
		DriverObject,                  // указатель на объект драйвера
		0,                             // размер расширени€ устройства
		&NtDeviceName,                 // им€ устройства
		FILE_DEVICE_UNKNOWN,           // тип устройства
		0,                             // характеристики устройства
                            TRUE,                         // not exclusive
		&DeviceSDDLString,             // строка SDDL, определ€юща€ доступ
                            NULL,                         // указатель класса устройства
		&g_DeviceObj);                // возвращает указатель на объект устройства

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE]         = DeviceCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = DeviceClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]        = DeviceCleanup;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControl;
    DriverObject->DriverUnload                         = DeviceUnload;

    RtlInitUnicodeString(&DosDevicesLinkName, DOS_DEVICES_LINK_NAME);
    Status = IoCreateSymbolicLink(&DosDevicesLinkName, &NtDeviceName);

    if (!NT_SUCCESS(Status)) {
        IoDeleteDevice(DriverObject->DeviceObject);
        return Status;
    }

	UNICODE_STRING AltitudeString = RTL_CONSTANT_STRING(L"380010");
	Status = CmRegisterCallbackEx(Callback,
		&AltitudeString,
		g_DeviceObj->DriverObject,
		NULL,
		&g_CmCookie,
		NULL);

	if (!NT_SUCCESS(Status)) {
		ErrorPrint("CmRegisterCallback failed. Status 0x%x", Status);
		return STATUS_NO_CALLBACK_ACTIVE;
	}
    return STATUS_SUCCESS;
}


NTSTATUS
DeviceCreate (
    _In_ PDEVICE_OBJECT DeviceObject,
    _Inout_ PIRP Irp
    )
{
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}


NTSTATUS
DeviceClose (
    _In_ PDEVICE_OBJECT DeviceObject,
    _Inout_ PIRP Irp
    )
{
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}



NTSTATUS
DeviceCleanup (
    _In_ PDEVICE_OBJECT DeviceObject,
    _Inout_ PIRP Irp
    )
{
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}



NTSTATUS
DeviceControl (
    _In_ PDEVICE_OBJECT DeviceObject,
    _Inout_ PIRP Irp
    )
{
    PIO_STACK_LOCATION IrpStack;
   // ULONG Ioctl;
    NTSTATUS Status;
	
	
    UNREFERENCED_PARAMETER(DeviceObject);

    Status = STATUS_SUCCESS;
	count_PsSet = 0;
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
	if (IrpStack->Parameters.DeviceIoControl.InputBufferLength < 3)
	{
		char show[100] = { 0 };
		in_buf = NULL;
		in_buf = Irp->AssociatedIrp.SystemBuffer;
		strcpy(show, in_buf);
		if (show[0] == '+')
		{
			GlobalByteOffset.LowPart = 0;
			DbgPrint("[mydriver]++++");
			Status = PsSetLoadImageNotifyRoutine(ProcessLoadImageCallback);
			if (!NT_SUCCESS(Status)) {
				ErrorPrint("PsSetLoadImageNotifyRoutine failed. Status 0x%x", Status);
			}
		}
		else if (show[0] == '-')
		{
			DbgPrint("[mydriver]--");
			Status = PsRemoveLoadImageNotifyRoutine(ProcessLoadImageCallback);
			if (!NT_SUCCESS(Status)) {
				ErrorPrint("PsRemoveLoadImageNotifyRoutine failed. Status 0x%x", Status);
			}
		}
	}
	
  else if (IrpStack->Parameters.DeviceIoControl.InputBufferLength >3)
	{
		wchar_t only_open[60] = L"regctrl.exe";
		wchar_t only_for_me[60] = L"SOFTWARE\\SuperProgram";
		char copy[100] = { 0 };
		int j = 0;
		rules_exe[0][0] = '5';
		for (j = 1; j<12; j++)
			rules_exe[0][j] = only_open[j-1];

		rules_key[0][0] = '4';
		for (j = 1; j < 22; j++)
			rules_key[0][j] = only_for_me[j-1];
		
		j = 1;
		count_exe = 1;
		count_key = 1;
		in_buf = NULL;
		in_buf = Irp->AssociatedIrp.SystemBuffer;
		DbgPrint("[mydriver]Rules: %s", in_buf);
		strcpy(copy, in_buf);
		int i2 = 0;
		for (unsigned int i = 0; i < IrpStack->Parameters.DeviceIoControl.InputBufferLength; i2++,i++)
		{
			if (copy[i] == '|')
			{
				j++; i2 = -1;
				count_exe = j;

			}
			else if (copy[i] == '!')
			{
				i++;
				i2 = 0; j = 1;
				for (i; i < IrpStack->Parameters.DeviceIoControl.InputBufferLength; i2++, i++)
				{
					if (copy[i] == '|')
					{
						j++; i2 = -1;
						count_key = j;
					}
					rules_key[j][i2] = (wchar_t)copy[i];
				}
				break;
			}
			rules_exe[j][i2] = (wchar_t)copy[i];
		}

	}
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}


VOID
DeviceUnload (
    _In_ PDRIVER_OBJECT DriverObject
    )
{
    UNICODE_STRING  DosDevicesLinkName;
    RtlInitUnicodeString(&DosDevicesLinkName, DOS_DEVICES_LINK_NAME);
    IoDeleteSymbolicLink(&DosDevicesLinkName);
    IoDeleteDevice(DriverObject->DeviceObject);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, 
               DPFLTR_ERROR_LEVEL,
               "RegFltr: DeviceUnload\n");
	NTSTATUS Status;
	Status = CmUnRegisterCallback(g_CmCookie);

	if (!NT_SUCCESS(Status)) {
		ErrorPrint("CmUnRegisterCallback failed. Status 0x%x", Status);
	}
}

