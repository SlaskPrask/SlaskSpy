#ifndef INPUT_ITEMS_H
#define INPUT_ITEMS_H

#include <cstdint>
#include <QGraphicsPixmapItem>

class InputStick : public QGraphicsPixmapItem {
public:
    InputStick(const QPixmap &pixmap, int32_t origin_x, int32_t origin_y, int32_t range_x, int32_t range_y);
    void Update(char x, char y);
private:
    const int32_t kOriginX;
    const int32_t kOriginY;
    const float kRangeX;
    const float kRangeY;
};

class InputButton : public QGraphicsPixmapItem {
public:
    InputButton(const QPixmap &pixmap, int32_t origin_x, int32_t origin_y);
    void Update(bool pressed);
};

#endif // INPUT_ITEMS_H
