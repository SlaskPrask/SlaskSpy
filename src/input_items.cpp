#include "../include/input_items.h"

#include <cstdint>
#include <QGraphicsPixmapItem>

InputStick::InputStick(const QPixmap &pixmap, int32_t origin_x, int32_t origin_y, int32_t range_x, int32_t range_y) :
    QGraphicsPixmapItem(pixmap),
    kOriginX{origin_x},
    kOriginY{origin_y},
    kRangeX{range_x / 128.f},
    kRangeY{range_y/ 128.f}
{
    setX(kOriginX);
    setY(kOriginY);
}

InputButton::InputButton(const QPixmap &pixmap, int32_t origin_x, int32_t origin_y) :
    QGraphicsPixmapItem(pixmap)
{
    setX(origin_x);
    setY(origin_y);
    hide();
}

void InputStick::Update(char x, char y) {
    setX(kOriginX + x * kRangeX);
    setY(kOriginY - y * kRangeY);
}

void InputButton::Update(bool pressed) {
    setVisible(pressed);
}
