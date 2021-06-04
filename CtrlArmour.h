#ifndef CTRLARMOUR_H
#define CTRLARMOUR_H

#include <QObject>
#ifdef WIN32
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>

#define ATKACPI L"\\\\.\\ATKACPI"

enum ATK_ACPI_FUNCTION {
    ATK_ACPI_WMIFUNCTION     = 0x22240c,
    ATK_ACPI_FUNCTION        = 0x222404,
    ATK_ACPI_GET_NOTIFY_CODE = 0x222408,
    ATK_ACPI_ASSIGN_EVENT    = 0x222400
};

enum ATK_ACPI_METHOD {
        DSTS_Method = 0x53545344,
        DEVS_Method = 0x53564544,
        INIT_Method = 0x54494e49,
        BSTS_Method = 0x53545342, // returns 0 on G14
        SFUN_Method = 0x4e554653
};

enum ATK_ACPI_IIA0 {
DevsHardwareCtrl = 0x00100021,
DevsBatteryChargeLimit = 0x00120057,
DevsThrottleCtrl = 0x00120075,
DevsCPUFanCurve = 0x00110024,
DevsGPUFanCurve = 0x00110025,
DstsDefaultCPUFanCurve = 0x00110024,
DstsDefaultGPUFanCurve = 0x00110025,
DstsCurrentCPUFanSpeed = 0x00110013,
DstsCurrentGPUFanSpeed = 0x00110014,
DstsCheckCharger = 0x0012006c
};

enum HID{
    HID_SET_FEATURE = 0xb0191,
    HID_GET_FEATURE = 0xb0192
};

class CtrlArmour : public QObject
{
    Q_OBJECT
public:
    CtrlArmour(){}
    ~CtrlArmour(){}

    bool sendArmourThrottlePlan(int idx) {
        BOOL bResult   = FALSE;
        DWORD out_buffer_written_ref = 0;
        atkacpiHandle = INVALID_HANDLE_VALUE;
        atkacpiHandle = CreateFileW(LPWSTR(ATKACPI),0xc0000000,3,NULL,3,0,NULL);

        if (atkacpiHandle == INVALID_HANDLE_VALUE)    // cannot open the drive
        {
            wprintf (L"INVALID_HANDLE_VALUE\n");
            return (FALSE);
        }

        UINT8 buffer[16] = {
            0x44, 0x45, 0x56, 0x53,
            0x08, 0x00, 0x00, 0x00,
            0x75, 0x00, 0x12, 0x00,
            0x00, 0x00, 0x00, 0x00
        };//^^^^ - Power Plan
        //PowerPlan::PerformanceWindows => 0x00,
        //PowerPlan::TurboManual => 0x01,
        //PowerPlan::Silent => 0x02,
        size_t buffer_size = 16;
        buffer[12] = idx;

        UINT8 *out_buffer = (UINT8*)malloc(buffer_size);
        void *out_buffer_void = out_buffer;

        bResult = DeviceIoControl(atkacpiHandle, DWORD(ATK_ACPI_WMIFUNCTION),
                                  buffer, buffer_size,
                                  out_buffer_void, buffer_size,
                                  &out_buffer_written_ref, NULL);

        CloseHandle(atkacpiHandle);
        return (bResult);
    }

private:
    HANDLE atkacpiHandle = INVALID_HANDLE_VALUE;
};
#else
class CtrlArmour : public QObject
{
    //Add armory preset switch for unix
    Q_OBJECT
public:
    CtrlArmour(){}
    ~CtrlArmour(){}

    bool sendArmourThrottlePlan(int idx) {
        return 0;
    }
};
#endif
#endif // CTRLARMOUR_H
