#include "com_ports.h"

#include <cstdint>
#include <string>
#include <thread>
#include <vector>
#include <windows.h>

#include <devguid.h>
#include <initguid.h>
#include <setupapi.h>

#include "logger.h"

namespace com_ports {

COMDevice::COMDevice(int32_t com_index, int32_t baud_rate,
		     size_t read_buffer_size,
		     std::function<void(char *)> const &set_data_callback,
		     std::function<void()> const &graphics_update_callback)
	: handle_{nullptr},
	  com_index_{com_index},
	  baud_rate_{baud_rate},
	  kBufferSize{read_buffer_size},
	  read_buffer_{new char[kBufferSize]},
	  set_data_callback_{set_data_callback},
	  graphics_update_callback_{graphics_update_callback}
{
	TryReconnecting();
}

bool COMDevice::Valid()
{
	return handle_ != INVALID_HANDLE_VALUE;
}

COMDevice::~COMDevice()
{
	if (handle_ != INVALID_HANDLE_VALUE) {
		CloseHandle(handle_);
	}

	if (read_buffer_ != nullptr) {
		delete[] read_buffer_;
	}
}

void COMDevice::Tick()
{
	try {
		DWORD bytes_read = 0;
		if (!ReadFile(handle_, read_buffer_,
			      static_cast<DWORD>(kBufferSize), &bytes_read,
			      nullptr)) {

			DWORD const error_code{GetLastError()};

			Logger::Error(
				"com_ports: Error %i, trying to reconnect");
			if (!TryReconnecting()) {
				constexpr int32_t kRetyLengthMilli{500};
				Logger::Error(
					"com_ports: Failed to reconnect, trying again in %i ms",
					kRetyLengthMilli);
				std::this_thread::sleep_for(
					std::chrono::milliseconds{
						kRetyLengthMilli});
				return;
			}
		}

		if (bytes_read < 1) {
			return;
		}
	} catch (std::exception &e) {
		Logger::Error("com_ports: %s", e.what());
		return;
	}

	if (read_buffer_[kBufferSize - 1] != 0x0A) {
		return;
	}

	set_data_callback_(read_buffer_);
	graphics_update_callback_();
}

bool COMDevice::TryReconnecting()
{
	if (handle_ != INVALID_HANDLE_VALUE) {
		CloseHandle(handle_);
	}

	handle_ = CreateFileA(("COM" + std::to_string(com_index_)).c_str(),
			      GENERIC_READ, 0, 0, OPEN_EXISTING, 0, nullptr);

	if (handle_ == INVALID_HANDLE_VALUE) {
		Logger::Error(
			"com_ports: Invalid handle value when trying to connect to com port %i",
			com_index_);
		return false;
	}

	DCB serial_params{};
	serial_params.DCBlength = sizeof(serial_params);
	if (!GetCommState(handle_, &serial_params)) {
		handle_ = INVALID_HANDLE_VALUE;
		Logger::Error(
			"com_ports: Failed getting parameters for com port %i",
			com_index_);
		return false;
	}
	serial_params.BaudRate = baud_rate_;
	serial_params.ByteSize = 8;
	serial_params.StopBits = TWOSTOPBITS;
	serial_params.Parity = NOPARITY;

	COMMTIMEOUTS cto{};
	GetCommTimeouts(handle_, &cto);
	cto.ReadIntervalTimeout = 10;
	cto.ReadTotalTimeoutConstant = 10;
	cto.ReadTotalTimeoutMultiplier = 10;
	cto.WriteTotalTimeoutConstant = 10;
	cto.WriteTotalTimeoutMultiplier = 10;
	SetCommTimeouts(handle_, &cto);

	if (!SetCommState(handle_, &serial_params)) {
		handle_ = INVALID_HANDLE_VALUE;
		Logger::Error(
			"com_ports: Failed setting parameters for com port %i",
			com_index_);
		return false;
	}

	return true;
}

std::vector<ComPortData> FetchCOMPorts()
{
	char lp_target_path[5000];
	std::vector<ComPortData> port_list{};
	int32_t index{0};

	for (int32_t i{0}; i < kMaxPort; ++i) {
		std::string str{"COM" + std::to_string(i)};
		DWORD result{
			QueryDosDeviceA(str.c_str(), lp_target_path, 5000)};

		if (result != 0) {
			ComPortData const new_port{i, GetFriendlyName(index)};
			port_list.push_back(new_port);
			++index;
		}
	}
	return port_list;
}

std::string GetFriendlyName(int32_t device)
{
	HDEVINFO h_device_info{SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, 0,
						   nullptr, DIGCF_PRESENT)};

	std::string res{""};
	if (h_device_info == INVALID_HANDLE_VALUE) {
		return res;
	}

	SP_DEVINFO_DATA dev_info_data{};
	dev_info_data.cbSize = sizeof(dev_info_data);
	SetupDiEnumDeviceInfo(h_device_info, device, &dev_info_data);

	DWORD req_size{0};
	SetupDiGetDeviceRegistryProperty(h_device_info, &dev_info_data,
					 SPDRP_HARDWAREID, nullptr, nullptr, 0,
					 &req_size);

	auto hardware_id{
		std::make_unique<BYTE[]>((req_size > 1) ? req_size : 1)};

	DWORD reg_data_type{0};
	if (SetupDiGetDeviceRegistryProperty(
		    h_device_info, &dev_info_data, SPDRP_HARDWAREID,
		    &reg_data_type, hardware_id.get(),
		    sizeof(hardware_id) * req_size, nullptr)) {
		req_size = 0;
		SetupDiGetDeviceRegistryProperty(h_device_info, &dev_info_data,
						 SPDRP_FRIENDLYNAME, nullptr,
						 nullptr, 0, &req_size);

		auto friendly_name{std::make_unique<BYTE[]>(
			(req_size > 1) ? req_size : 1)};
		if (!SetupDiGetDeviceRegistryProperty(
			    h_device_info, &dev_info_data, SPDRP_FRIENDLYNAME,
			    nullptr, friendly_name.get(),
			    sizeof(friendly_name) * req_size, nullptr)) {
			// device does not have this property set

			memset(friendly_name.get(), 0,
			       req_size > 1 ? req_size : 1);
		}

		for (size_t i{0}; i < req_size; i += 2) {
			res += static_cast<char>(friendly_name[i]);
		}
	}
	return res;
}
} // namespace com_ports
