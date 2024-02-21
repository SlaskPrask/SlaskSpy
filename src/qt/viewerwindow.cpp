#include "viewerwindow.h"
#include "ui_viewerwindow.h"

#include <cstdint>
#include <iostream>
#include <functional>
#include <QCloseEvent>

#include "qt_graphics_wrapper.h"
#include "viewer.h"

ViewerWindow::ViewerWindow(QWidget *parent, std::function<void()> close_callback, int32_t com_index)
    : QMainWindow(parent)
    , ui(new Ui::ViewerWindow)
    , close_callback_{close_callback}
    , input_viewer_{nullptr}
{
    ui->setupUi(this);
    input_viewer_ = new InputViewer(
        new QTGraphicsWrapper(ui->graphicsView),
        com_index,
        ViewerType::kN64
    );

    if (input_viewer_->Valid()) {
        setFixedSize(input_viewer_->GetWindowWidth(), input_viewer_->GetWindowHeight());
    }
}

bool ViewerWindow::Valid() const {
    return input_viewer_->Valid();
}

ViewerWindow::~ViewerWindow()
{
    delete ui;
}

void ViewerWindow::closeEvent(QCloseEvent *event) {
    event->ignore();
    delete input_viewer_;
    close_callback_();
}
