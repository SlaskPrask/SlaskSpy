#ifndef N64_VIEWER_H
#define N64_VIEWER_H

#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "input_items.h"
#include "viewer.h"

class N64Viewer : public Viewer
{
public:
    N64Viewer();

    void SetIncommingData(char* data) override;
    size_t GetDataBytesSize() override;
    bool AssignButton(std::string_view name, InputButton* button_item) override;
    bool AssignStick(std::string_view x_name, std::string_view y_name, InputStick* stick_item) override;
private: 
    static std::unordered_map<std::string_view, int32_t> const kMapping;

    std::vector<int32_t> assigned_buttons_;
    std::unordered_map<int32_t, InputButton*> buttons_;

    int32_t stick_x_;
    int32_t stick_y_;
    InputStick* stick_;
};

#endif // N64_VIEWER_H
