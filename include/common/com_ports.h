#ifndef COM_PORTS_H
#define COM_PORTS_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <windows.h>

namespace com_ports {
constexpr int32_t kMaxPort{255};

struct ComPortData {
	int32_t index;
	std::string friendly_name;
};

std::vector<ComPortData> FetchCOMPorts();
std::string GetFriendlyName(int32_t device);

class COMDevice {
public:
	COMDevice(int32_t com_index, int32_t baud_rate, size_t read_buffer_size,
		  std::function<void(char *)> const &set_data_callback,
		  std::function<void()> const &graphics_update_callback);
	~COMDevice();

	bool Valid();
	void Tick();

private:
	bool TryReconnecting();

	HANDLE handle_;
	int32_t com_index_;
	int32_t baud_rate_;

	size_t const kBufferSize;
	char *read_buffer_;
	std::function<void(char *)> set_data_callback_;
	std::function<void()> graphics_update_callback_;
};

} // namespace com_ports

#endif // COM_PORTS_H