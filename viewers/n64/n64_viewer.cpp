#include "n64/n64_viewer.h"

#include <cstdint>
#include <iostream>
#include <string_view>

std::unordered_map<std::string_view, int32_t> const N64Viewer::kMapping{
    {"a", 0},
    {"b", 1},
    {"z", 2},
    {"start", 3},
    {"up", 4},
    {"down", 5},
    {"left", 6},
    {"right", 7},
    {"l", 10},
    {"r", 11},
    {"cup", 12},
    {"cdown", 13},
    {"cleft", 14},
    {"cright", 15},
    {"stick_x", 16},
    {"stick_y", 24}
};

N64Viewer::N64Viewer() :
    assigned_buttons_{},
    buttons_{},
    stick_x_{-1},
    stick_y_{-1},
    stick_{nullptr}
{

}

void N64Viewer::SetIncommingData(char* data) {
    for (auto it : assigned_buttons_) {
        buttons_[it]->Update(data[it]);
    }

    if (stick_ != nullptr) {
        char x{0};
        char y{0};
        for (int32_t i{0}; i < 8; ++i) {
            x |= data[stick_x_ + i] ? (0x80 >> i) : 0;
            y |= data[stick_y_ + i] ? (0x80 >> i) : 0;
        }
        stick_->Update(x, y);
    }
}

size_t N64Viewer::GetDataBytesSize() {
    return 33;
}

bool N64Viewer::AssignButton(std::string_view name, InputButton* button_item) {
    auto const it = kMapping.find(name);
    if (it == kMapping.end()) {
        std::cerr << "Invalid button name: " << name << std::endl;
        return false;
    }

    assigned_buttons_.push_back(it->second);
    buttons_[it->second] = button_item;
    return true;
}

bool N64Viewer::AssignStick(std::string_view x_name, std::string_view y_name, InputStick* stick_item) {

    auto const it1 = kMapping.find(x_name);
    if (it1 == kMapping.end()) {
        std::cerr << "Invalid stick name: " << x_name << std::endl;
        return false;
    }

    auto const it2 = kMapping.find(y_name);
    if (it2 == kMapping.end()) {
        std::cerr << "Invalid stick name: " << y_name << std::endl;
        return false;
    }

    stick_x_ = it1->second;
    stick_y_ = it2->second;
    stick_ = stick_item;
    return true;
}
