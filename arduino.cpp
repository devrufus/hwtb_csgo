#pragma comment(lib, "Setupapi.lib")
#include "arduino.h"
#include <devguid.h>
#include <SetupAPI.h>
#include <iostream>


bool arduino::scan_devices(LPCSTR friendly_name, LPSTR com_port)
{
    bool result = false;
    const char com[] = "COM";

    HDEVINFO device_info = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, nullptr, nullptr, DIGCF_PRESENT);
    if (device_info == INVALID_HANDLE_VALUE)
        return result;

    SP_DEVINFO_DATA dev_info_data = { };
    dev_info_data.cbSize = sizeof(dev_info_data);

    DWORD device_count = 0;
    while (SetupDiEnumDeviceInfo(device_info, device_count++, &dev_info_data))
    {
        BYTE buffer[256] = { 0 };
        if (SetupDiGetDeviceRegistryProperty(
            device_info,
            &dev_info_data,
            SPDRP_FRIENDLYNAME,
            nullptr,
            buffer,
            sizeof(buffer),
            nullptr
        ))
        {
            DWORD out_buff_len = strlen(com_port);
            LPCSTR port_name_pos = strstr(reinterpret_cast<LPCSTR>(buffer), com);
            if (port_name_pos == nullptr)
                continue;

            DWORD len = out_buff_len + strlen(port_name_pos);
            if (strstr(reinterpret_cast<LPCSTR>(buffer), friendly_name))
            {
                for (DWORD i = 0; i < len; i++, out_buff_len++)
                    com_port[out_buff_len] = port_name_pos[i];

                com_port[strlen(com_port) - 1] = 0;
                result = true;
                break;
            }
        }
    }

    return result;
}
bool arduino::send_data(char* buffer, DWORD buffer_size)
{
    DWORD bytes_written;
    return WriteFile(this->arduino_handle, buffer, buffer_size, &bytes_written, NULL);
}

arduino::arduino(LPCSTR device_name)
{
    char port[] = "\\.\\";

    while (!scan_devices(device_name, port))
    {
        Sleep(1000);
    }

    this->arduino_handle = CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (this->arduino_handle)
    {
        DCB dcb = { 0 };
        dcb.DCBlength = sizeof(dcb);
        if (!GetCommState(this->arduino_handle, &dcb))
        {
            printf("[!] GetCommState() failed\n");
            CloseHandle(this->arduino_handle);
        }

        dcb.BaudRate = CBR_115200;
        dcb.ByteSize = 8;
        dcb.StopBits = ONESTOPBIT;
        dcb.Parity = NOPARITY;
        if (!SetCommState(this->arduino_handle, &dcb))
        {
            printf("[!] SetCommState() failed\n");
            CloseHandle(this->arduino_handle);
        }

        COMMTIMEOUTS cto = { 0 };
        cto.ReadIntervalTimeout = 50;
        cto.ReadTotalTimeoutConstant = 50;
        cto.ReadTotalTimeoutMultiplier = 10;
        cto.WriteTotalTimeoutConstant = 50;
        cto.WriteTotalTimeoutMultiplier = 10;
        if (!SetCommTimeouts(this->arduino_handle, &cto))
        {
            printf("[!] SetCommTimeouts() failed\n");
            CloseHandle(this->arduino_handle);
        }
        printf("[!] Connected to [%s]\n", device_name);
    }
}

arduino::~arduino()
{
    CloseHandle(this->arduino_handle);
}