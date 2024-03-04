#include "viewer.h"

#include <cstdint>
#include <string_view>
#include <unordered_map>

#include "logger.h"
#include "viewers/n64_viewer.h"
#include "viewers/gamecube_viewer.h"

namespace slask_spy {
Viewer* Viewer::CreateViewer(ViewerType type) {
    switch (type) {
        case ViewerType::kN64:
            return new N64Viewer();
        case ViewerType::kGC:
            return new GamecubeViewer();
        case ViewerType::kNull:
        default:
            return nullptr;
    }
}

std::string Viewer::StringFromType(ViewerType type) {
    switch (type)
    {
        case ViewerType::kN64:
            return "N64";
        case ViewerType::kGC:
            return "GameCube";
        case ViewerType::kNull:
        default:
            return "None";
    }
}

ViewerType Viewer::TypeFromString(std::string_view type_string) {
    static const std::unordered_map<std::string_view, ViewerType> kTypeMap{
        {"n64", ViewerType::kN64},
        {"gamecube", ViewerType::kGC}
    };

    auto const it = kTypeMap.find(type_string);
    if (it == kTypeMap.end()) {
        return ViewerType::kNull;
    }
    return it->second;
}

int32_t Viewer::GetMappingIndex(std::string_view name, ViewerType type) {
    std::unordered_map<std::string_view, int32_t> const* mappings{nullptr};
    switch (type)
    {
        case slask_spy::ViewerType::kN64:
            mappings = &N64Viewer::GetMapping();
            break;
        case slask_spy::ViewerType::kGC:
            mappings = &GamecubeViewer::GetMapping();
            break;
        default:
        case slask_spy::ViewerType::kNull:
            Logger::Error("viewer: Viewer type not found");
            return -1;
    }

    auto const it1 = mappings->find(name);
    if (it1 == mappings->end()) {
        Logger::Error("viewer: Invalid name: %s", name);
        return -1;
    }
    return it1->second;
}

void Viewer::SetIncommingData(char* data) {
    for (auto it : assigned_buttons_) {
        it->Update(data[it->Index()]);
    }

    for (auto it : assigned_sticks_) {
        int8_t x{ 0 };
        int8_t y{ 0 };
        const int32_t x_index{ it->IndexX() };
        const int32_t y_index{ it->IndexY() };
        for (int32_t i{ 0 }; i < 8; ++i) {
            x |= data[x_index + i] ? (0x80 >> i) : 0;
            y |= data[y_index + i] ? (0x80 >> i) : 0;
        }
        it->Update(x, y);
    }

    for (auto it : assigned_analogs_) {
        uint8_t analog{ 0 };
        const int32_t index{ it->Index() };
        for (int32_t i{ 0 }; i < 8; ++i) {
            analog |= data[index + i] ? (0x80 >> i) : 0;
        }
        it->Update(analog);
    }
}


void Viewer::AssignButton(InputButton* button_item) {
    assigned_buttons_.push_back(button_item);
}

void Viewer::AssignStick(InputStick* stick_item) {
    assigned_sticks_.push_back(stick_item);
}

void Viewer::AssignAnalog(InputAnalog* analog) {
    assigned_analogs_.push_back(analog);
}
} // namespace slask_spy
