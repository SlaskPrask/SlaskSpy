#ifndef VIEWERWINDOW_H
#define VIEWERWINDOW_H

#include <QMainWindow>
#include <cstdint>
#include <functional>
#include <string_view>

#include "qt_graphics_wrapper.h"

namespace Ui {
class ViewerWindow;
}

class ViewerWindow : public QMainWindow {
	Q_OBJECT

public:
	ViewerWindow(QWidget *parent = nullptr,
		     std::function<void()> close_callback = {},
		     std::string_view skin_directory, int32_t com_index = 0);
	~ViewerWindow();

	bool Valid() const;

private:
	void closeEvent(QCloseEvent *event);
	Ui::ViewerWindow *ui;
	std::function<void()> close_callback_;
	QTGraphicsWrapper *graphics_wrapper_;
};

#endif // VIEWERWINDOW_H
