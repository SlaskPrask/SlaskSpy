#include "qt_input_items.h"
#include <QGraphicsPixmapItem>
#include <cstdint>

QtInputStick::QtInputStick(QPixmap const &pixmap, int32_t origin_x,
			   int32_t origin_y, int32_t range_x, int32_t range_y,
			   float x_divisor, float y_divisor)
	: InputStick(range_x, range_y, x_divisor, y_divisor),
	  QGraphicsPixmapItem(pixmap),
	  kOriginX{origin_x},
	  kOriginY{origin_y}
{
	setX(origin_x);
	setY(origin_y);
}

void QtInputStick::Update(char x, char y)
{
	setX(kOriginX + x * kRangeX);
	setY(kOriginY - y * kRangeY);
}

QtInputButton::QtInputButton(QPixmap const &pixmap, int32_t origin_x,
			     int32_t origin_y)
	: QGraphicsPixmapItem(pixmap)
{
	setX(origin_x);
	setY(origin_y);
	hide();
}

void QtInputButton::Update(bool pressed)
{
	setVisible(pressed);
}
