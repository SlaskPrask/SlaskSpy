#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <cstdint>

#include "com_ports.h"
#include <functional>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr,
		   std::function<void(int32_t)> close_callback = {});
	~MainWindow();

private slots:
	void on_comRescan_clicked();

	void on_startButton_clicked();

private:
	void RescanPorts();

	Ui::MainWindow *ui;
	std::vector<com_ports::ComPortData> ports_;
	std::function<void(int32_t)> close_callback_;
};
#endif // MAINWINDOW_H
