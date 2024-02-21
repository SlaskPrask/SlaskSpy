#ifndef COM_PORTS_H
#define COM_PORTS_H

#include <cstdint>
#include <functional>
#include <vector>
#include <string>
#include <windows.h>

#include "viewer.h"

#include <QGraphicsView>
#include <QObject>
#include <QString>
#include <QTimer>

namespace com_ports {
constexpr int32_t kMaxPort{255};

struct ComPortData {
    int32_t index;
    QString friendly_name;
};

std::vector<ComPortData> FetchCOMPorts();
std::string GetFriendlyName(int32_t device);

class COMDevice : public QObject
{
public:
    COMDevice(int32_t com_index, int32_t baud_rate, Viewer* viewer, QGraphicsView* graphics_view);
    ~COMDevice();

    void StartReading();
    bool Valid();

private:

    void Tick();
    bool TryReconnecting();

    HANDLE handle_;
    QTimer* dispatch_;
    int32_t com_index_;
    int32_t baud_rate_;

    size_t const kBufferSize;
    char* read_buffer_;
    Viewer* viewer_;
    QGraphicsView* graphics_view_;
};

}

#endif // COM_PORTS_H
