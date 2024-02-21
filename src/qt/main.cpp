#include <QApplication>

#include "window_manager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WindowManager window{};
    return a.exec();
}
