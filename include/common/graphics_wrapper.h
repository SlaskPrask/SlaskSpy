#ifndef GRAPHICS_WRAPPER_H
#define GRAPHICS_WRAPPER_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>

#include "viewer.h"

class GraphicsWrapper
{
public:
    virtual void StartDispatchThread(std::function<void()> const& tick_callback) = 0;
    virtual void Update() = 0;
    virtual bool SetupScene(std::string_view skin_directory, Viewer* viewer) = 0;
    virtual int32_t GetWidth() const = 0;
    virtual int32_t GetHeight() const = 0;
    virtual ~GraphicsWrapper() = default;
};

#endif // GRAPHICS_WRAPPER_H
