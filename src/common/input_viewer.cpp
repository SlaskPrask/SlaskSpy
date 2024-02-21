#include "input_viewer.h"

#include "com_ports.h"
#include "viewer.h"
#include "viewers/n64_viewer.h"

namespace {
const std::string kSkinPath{"../SlaskSpy/InputSkins/Tron-Style/"};
constexpr uint32_t kBaudRate{115200};
Viewer* GetViewer(ViewerType type) {
    switch (type) {
        case ViewerType::kN64:
            return new N64Viewer();
        default:
            return nullptr;
    }
}
}

InputViewer::InputViewer(GraphicsWrapper* graphics_wrapper, int32_t com_port, ViewerType type) :
    initialized_{false},
    graphics_wrapper_{graphics_wrapper},
    viewer_{GetViewer(type)},
    input_{com_ports::COMDevice(
        com_port,
        kBaudRate,
        viewer_->GetDataBytesSize(),
        [this](char* data){viewer_->SetIncommingData(data);},
        [this](){graphics_wrapper_->Update();}
    )}
{

    if (!input_.Valid()) {
        return;
    }

    if (!graphics_wrapper_->SetupScene(kSkinPath, viewer_)) {
        return;
    }
    graphics_wrapper_->StartDispatchThread([this](){ input_.Tick();});
    initialized_ = true;
}

InputViewer::~InputViewer() {
    delete graphics_wrapper_;
    delete viewer_;
}

bool InputViewer::Valid() const {
    return initialized_;
}

int32_t InputViewer::GetWindowWidth() const {
    return graphics_wrapper_->GetWidth();
}

int32_t InputViewer::GetWindowHeight() const {
    return graphics_wrapper_->GetHeight();
}
