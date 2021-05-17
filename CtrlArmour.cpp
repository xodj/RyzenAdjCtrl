#include "CtrlArmour.h"

CtrlArmour::CtrlArmour() {}

CtrlArmour::~CtrlArmour() {}

bool CtrlArmour::sendArmourThrottlePlan(int idx) {
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
