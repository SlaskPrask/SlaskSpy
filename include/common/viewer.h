#ifndef VIEWER_H
#define VIEWER_H

#include <string_view>
#include "input_items.h"

enum class ViewerType {
    kN64 = 0
};

class Viewer {
  public:
    virtual void SetIncommingData(char* data) = 0;
    virtual size_t GetDataBytesSize() = 0;
    virtual bool AssignButton(std::string_view name, InputButton* button_item) = 0;
    virtual bool AssignStick(std::string_view x_name, std::string_view y_name, InputStick* stick_item) = 0;
    virtual ~Viewer() = default;
};

#endif // VIEWER_H
