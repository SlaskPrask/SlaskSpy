#ifndef VIEWERWINDOW_H
#define VIEWERWINDOW_H

#include <cstdint>
#include <functional>
#include <QMainWindow>

#include "inputviewer.h"

namespace Ui {
class ViewerWindow;
}

class ViewerWindow : public QMainWindow
{
    Q_OBJECT

public:
    ViewerWindow(QWidget *parent = nullptr, std::function<void()> close_callback = {}, int32_t com_index = 0);
    ~ViewerWindow();

    bool Valid();

private:
    void closeEvent(QCloseEvent *bar);
    Ui::ViewerWindow *ui;
    std::function<void()> close_callback_;
    std::unique_ptr<InputViewer> input_viewer_;
};

#endif // VIEWERWINDOW_H
