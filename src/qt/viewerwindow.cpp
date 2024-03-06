#include "viewerwindow.h"
#include "ui_viewerwindow.h"

#include <QCloseEvent>
#include <cstdint>
#include <functional>
#include <iostream>

#include "qt_graphics_wrapper.h"
#include "skin_settings.h"
#include "viewer.h"

using slask_spy::SkinSettings;

ViewerWindow::ViewerWindow(QWidget *parent,
			   std::function<void()> close_callback,
			   std::string_view skin_directory, int32_t com_index)
	: QMainWindow(parent),
	  ui(new Ui::ViewerWindow),
	  close_callback_{close_callback},
	  graphics_wrapper_{nullptr}
{
	ui->setupUi(this);
	SkinSettings const *settings =
		SkinSettings::LoadSkinSettings(skin_directory);
	graphics_wrapper_ = new QTGraphicsWrapper(ui->graphicsView);
	graphics_wrapper_->SetupScene(settings, viewer);

	input_viewer_ = new InputViewer(new QTGraphicsWrapper(ui->graphicsView),
					com_index, ViewerType::kN64);

	if (input_viewer_->Valid()) {
		setFixedSize(input_viewer_->GetWindowWidth(),
			     input_viewer_->GetWindowHeight());
	}
}

bool ViewerWindow::Valid() const
{
	return input_viewer_->Valid();
}

ViewerWindow::~ViewerWindow()
{
	delete ui;
}

void ViewerWindow::closeEvent(QCloseEvent *event)
{
	event->ignore();
	close_callback_();
}
