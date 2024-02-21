#include "viewerwindow.h"
#include "ui_viewerwindow.h"

#include <cstdint>
#include <functional>
#include <QCloseEvent>

#include "viewer.h"

ViewerWindow::ViewerWindow(QWidget *parent, std::function<void()> close_callback, int32_t com_index)
    : QMainWindow(parent)
    , ui(new Ui::ViewerWindow)
    , close_callback_{close_callback}
    , input_viewer_{nullptr}
{
    ui->setupUi(this);
    input_viewer_ = std::make_unique<InputViewer>(ui->graphicsView, [this](int32_t width, int32_t height){setFixedSize(width, height);}, com_index, ViewerType::kN64);
}

bool ViewerWindow::Valid() {
    return input_viewer_->Valid();
}

ViewerWindow::~ViewerWindow()
{
    delete ui;
}

void ViewerWindow::closeEvent(QCloseEvent *event) {
    event->ignore();
    close_callback_();
}
