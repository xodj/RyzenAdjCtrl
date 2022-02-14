#ifndef CTRLARMOUR_H
#define CTRLARMOUR_H

#include <QObject>
#include <QDebug>
#include "CtrlConfig.h"

#ifdef WIN32
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>

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
        atkacpiHandle = CreateFileW(LPWSTR(THERMAL_PATH),0xc0000000,3,NULL,3,0,NULL);

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
#include <QFile>

class CtrlArmour : public QObject
{
    //Add armory preset switch for unix
    Q_OBJECT
public:
    CtrlArmour(){
        atkpci.setFileName(THERMAL_PATH);
    }
    bool sendArmourThrottlePlan(int idx) {
        QString newThrottleThermalPolicy = QString::number(idx) + "\n";
        atkpci.open(QIODevice::ReadWrite);
        atkpci.write(newThrottleThermalPolicy.toUtf8());
        atkpci.close();

        atkpci.open(QIODevice::ReadOnly);
        int rertieveNumber = QString::fromUtf8(atkpci.readAll()).remove(1,2).toInt();
        atkpci.close();

        bool retrieve = rertieveNumber == idx;
        qDebug() << "Retrieve Throttle Thermal Policy:" << retrieve;
        return retrieve;
    }

private:
    QFile atkpci;
};
#endif
#endif // CTRLARMOUR_H
