#ifndef QT_INPUT_ITEMS_H
#define QT_INPUT_ITEMS_H

#include "input_items.h"
#include <QGraphicsPixmapItem>
#include <cstdint>

class QtInputStick : public InputStick, public QGraphicsPixmapItem {
public:
  QtInputStick(QPixmap const &pixmap, int32_t origin_x, int32_t origin_y,
               int32_t range_x, int32_t range_y, float x_divisor,
               float y_divisor);
  void Update(char x, char y) override;

private:
  int32_t const kOriginX;
  int32_t const kOriginY;
};

class QtInputButton : public InputButton, public QGraphicsPixmapItem {
public:
  QtInputButton(QPixmap const &pixmap, int32_t origin_x, int32_t origin_y);
  void Update(bool pressed) override;
};

#endif // QT_INPUT_ITEMS_H
