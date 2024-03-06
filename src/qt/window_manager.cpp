#include "window_manager.h"

#include <cstdint>

#include "mainwindow.h"
#include "viewerwindow.h"

WindowManager::WindowManager() : main_window_{nullptr}, viewer_window_{nullptr}
{
	OpenMain();
}

void WindowManager::OpenViewer(int32_t com_index)
{
	viewer_window_ = std::make_unique<ViewerWindow>(
		nullptr, [this]() { OpenMain(); }, com_index);
	if (!viewer_window_->Valid()) {
		viewer_window_ = nullptr;
		return;
	}
	viewer_window_->setWindowFlags(viewer_window_->windowFlags() &
				       (~Qt::WindowMaximizeButtonHint));
	viewer_window_->show();

	main_window_->close();
	main_window_ = nullptr;
}

void WindowManager::OpenMain()
{
	main_window_ = std::make_unique<MainWindow>(
		nullptr, [this](int32_t com_index) { OpenViewer(com_index); });
	main_window_->setWindowFlags(main_window_->windowFlags() &
				     (~Qt::WindowMaximizeButtonHint));
	main_window_->show();

	if (viewer_window_ != nullptr) {
		viewer_window_->close();
		viewer_window_ = nullptr;
	}
}
