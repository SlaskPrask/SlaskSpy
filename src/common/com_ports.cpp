#include "com_ports.h"

#include <cstdint>
#include <iostream>
#include <vector>
#include <windows.h>
#include <string>

#include <initguid.h>
#include <devguid.h>
#include <setupapi.h>

namespace com_ports {

COMDevice::COMDevice(int32_t com_index,
                     int32_t baud_rate,
                     size_t read_buffer_size,
                     std::function<void(char*)> const& set_data_callback,
                     std::function<void()> const& graphics_update_callback) :
    handle_{nullptr},
    com_index_{com_index},
    baud_rate_{baud_rate},
    kBufferSize{read_buffer_size},
    read_buffer_{new char[kBufferSize]},
    set_data_callback_{set_data_callback},
    graphics_update_callback_{graphics_update_callback}
{
    TryReconnecting();
}

bool COMDevice::Valid() {
    return handle_ != INVALID_HANDLE_VALUE;
}

COMDevice::~COMDevice() {
    if (handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(handle_);
    }

    if (read_buffer_ != nullptr) {
        delete[] read_buffer_;
    }
}

void COMDevice::Tick() {

    try {
        DWORD bytes_read = 0;

        if (!ReadFile(handle_,
                 read_buffer_,
                 kBufferSize,
                 &bytes_read,
                      nullptr)) {
            std::cout  << "Error Reading" << std::endl;
            TryReconnecting();
        }

        if (bytes_read < 1) {
            std::cout << "Nothing Read" << std::endl;
            return;
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return;
    }

    if (read_buffer_[kBufferSize - 1] != 0x0A) {
        return;
    }

    set_data_callback_(read_buffer_);
    graphics_update_callback_();
}

bool COMDevice::TryReconnecting() {
    if (handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(handle_);
    }

    handle_ =
        CreateFileA(("COM" + std::to_string(com_index_)).c_str(),
                    GENERIC_READ,
                    0,
                    0,
                    OPEN_EXISTING,
                    0, nullptr);

    if (handle_ == INVALID_HANDLE_VALUE) {
        std::cerr << "Invalid Handle" << std::endl;
        return false;
    }

    DCB serial_params{};
    serial_params.DCBlength = sizeof(serial_params);
    if (!GetCommState(handle_, &serial_params)){
        handle_ = INVALID_HANDLE_VALUE;
        std::cerr << "Failed getting params" << std::endl;
        return false;
    }
    serial_params.BaudRate = baud_rate_;
    serial_params.ByteSize = 8;
    serial_params.StopBits = TWOSTOPBITS;
    serial_params.Parity = NOPARITY;
    if (!SetCommState(handle_, &serial_params)) {
        handle_ = INVALID_HANDLE_VALUE;
        std::cerr << "Failed setting params" << std::endl;
        return false;
    }

    return true;
}

std::vector<ComPortData> FetchCOMPorts() {
    char lp_target_path[5000];
    std::vector<ComPortData> port_list{};
    int32_t index{0};

    for (int32_t i{0}; i < kMaxPort; ++i) {
        std::string str{"COM" + std::to_string(i)};
        DWORD result{QueryDosDeviceA(str.c_str(), lp_target_path, 5000)};

        if (result != 0) {
            ComPortData const new_port{
                i,
                GetFriendlyName(index)
            };
            port_list.push_back(new_port);
            ++index;
        }
    }
    return port_list;
}

std::string GetFriendlyName(int32_t device) {
    HDEVINFO h_device_info{SetupDiGetClassDevs(
        &GUID_DEVCLASS_PORTS,
        0,
        nullptr,
        DIGCF_PRESENT
    )};

    std::string res{""};
    if (h_device_info == INVALID_HANDLE_VALUE) {
        return res;
    }

    SP_DEVINFO_DATA dev_info_data{};
    dev_info_data.cbSize = sizeof(dev_info_data);
    SetupDiEnumDeviceInfo(h_device_info, device, &dev_info_data);

    DWORD req_size{0};
    SetupDiGetDeviceRegistryProperty(h_device_info, &dev_info_data, SPDRP_HARDWAREID, nullptr, nullptr, 0, &req_size);

    BYTE* hardware_id{new BYTE[(req_size > 1) ? req_size : 1]};
    DWORD reg_data_type{0};
    if (SetupDiGetDeviceRegistryProperty(h_device_info, &dev_info_data, SPDRP_HARDWAREID, &reg_data_type, hardware_id, sizeof(hardware_id) * req_size, nullptr)) {
        req_size = 0;
        SetupDiGetDeviceRegistryProperty(h_device_info, &dev_info_data, SPDRP_FRIENDLYNAME, nullptr, nullptr, 0, &req_size);

        BYTE* friendly_name = new BYTE[(req_size > 1) ? req_size : 1];
        if (!SetupDiGetDeviceRegistryProperty(h_device_info, &dev_info_data, SPDRP_FRIENDLYNAME, nullptr, friendly_name, sizeof(friendly_name) * req_size, nullptr))
        {
            // device does not have this property set

            memset(friendly_name, 0, req_size > 1 ? req_size : 1);
        }
        wchar_t* str = new wchar_t[req_size / 2 + 1];
        memcpy(str, friendly_name, req_size);
        std::wstring ws{str};
        res = std::string(ws.begin(), ws.end());

        delete[] str;
        delete[] friendly_name;
    }
    delete[] hardware_id;
    return res;
}
}
