#ifndef INPUT_VIEWER_H
#define INPUT_VIEWER_H

#include <cstdint>
#include <memory>

#include "com_ports.h"
#include "graphics_wrapper.h"
#include "viewer.h"

class InputViewer
{
public:
    InputViewer(GraphicsWrapper* graphics_wrapper, int32_t com_port, ViewerType type);
    ~InputViewer();
    bool Valid() const;
    int32_t GetWindowWidth() const;
    int32_t GetWindowHeight() const;

private:

    bool initialized_;
    GraphicsWrapper* graphics_wrapper_;
    Viewer* viewer_;
    com_ports::COMDevice input_;
};

#endif // INPUT_VIEWER_H
