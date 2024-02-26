#ifndef INPUT_ITEMS_H
#define INPUT_ITEMS_H

#include <cstdint>

class InputStick {
public:
    InputStick( int32_t range_x, int32_t range_y, float x_divisor, float y_divisor) :
        kRangeX{range_x / x_divisor},
        kRangeY{range_y/ y_divisor}
    {}
    virtual void Update(char x, char y) = 0;
    virtual ~InputStick(){}
protected:
    const float kRangeX;
    const float kRangeY;
};

class InputButton {
public:
    virtual void Update(bool pressed) = 0;
    virtual ~InputButton(){}
};

#endif // INPUT_ITEMS_H
