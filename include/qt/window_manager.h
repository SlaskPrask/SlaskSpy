#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "mainwindow.h"
#include "viewerwindow.h"

class WindowManager {
public:
	WindowManager();

private:
	void OpenViewer(int32_t com_index);
	void OpenMain();

	std::unique_ptr<MainWindow> main_window_{nullptr};
	std::unique_ptr<ViewerWindow> viewer_window_{nullptr};
};

#endif // WINDOW_MANAGER_H
