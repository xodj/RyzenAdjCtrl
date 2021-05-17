#ifndef CTRLARMOUR_H
#define CTRLARMOUR_H

#include <QObject>
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
    CtrlArmour();
    ~CtrlArmour();

    bool sendArmourThrottlePlan(int idx);

private:
    HANDLE atkacpiHandle = INVALID_HANDLE_VALUE;
};

#endif // CTRLARMOUR_H
