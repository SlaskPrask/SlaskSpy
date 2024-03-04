#ifndef INPUT_ITEMS_H
#define INPUT_ITEMS_H

#include <cstdint>

#include "skin_settings.h"

namespace slask_spy {

class InputAnalog {
public:
    InputAnalog(AnalogSetting const* settings) : kDirection{ settings->direction }, kReverse{ settings->reverse }, kIndex{ settings->index } {
    }
    virtual void Update(uint8_t analog) = 0;

    int32_t Index() const {
        return kIndex;
    }
private:
    const AnalogDirection kDirection;
    const bool kReverse;
    const int32_t kIndex;
};

class InputStick {
public:
    InputStick(StickSetting const* settings, float x_divisor, float y_divisor) :
        kRangeX{settings->x_range / x_divisor},
        kRangeY{settings->y_range / y_divisor},
        kIndexX{ settings->x_index},
        kIndexY{ settings->y_index}
    {}
    virtual void Update(int8_t x, int8_t y) = 0;
    virtual ~InputStick(){}

    int32_t IndexX() const {
        return kIndexX;
    }

    int32_t IndexY() const {
        return kIndexY;
    }
protected:
    const float kRangeX;
    const float kRangeY;
    const int32_t kIndexX;
    const int32_t kIndexY;
};

class InputButton {
public:
    InputButton(ButtonSetting const* settings) : kIndex{settings->index} {}
    virtual void Update(bool pressed) = 0;
    virtual ~InputButton(){}
    int32_t Index() const {
        return kIndex;
    }

private:
    const int32_t kIndex;
};
} // namespace slask_spy
#endif // INPUT_ITEMS_H
