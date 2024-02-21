#ifndef VIEWERWINDOW_H
#define VIEWERWINDOW_H

#include <cstdint>
#include <functional>
#include <QMainWindow>

#include "input_viewer.h"

namespace Ui {
class ViewerWindow;
}

class ViewerWindow : public QMainWindow
{
    Q_OBJECT

public:
    ViewerWindow(QWidget *parent = nullptr, std::function<void()> close_callback = {}, int32_t com_index = 0);
    ~ViewerWindow();

    bool Valid() const;

private:
    void closeEvent(QCloseEvent *event);
    Ui::ViewerWindow *ui;
    std::function<void()> close_callback_;
    InputViewer* input_viewer_;
};

#endif // VIEWERWINDOW_H
