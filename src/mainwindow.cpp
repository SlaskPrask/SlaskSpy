#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "viewerwindow.h"

#include <functional>
#include <vector>
#include "com_ports.h"


MainWindow::MainWindow(QWidget *parent, std::function<void(int32_t)> close_callback)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , ports_{}
    , close_callback_{close_callback}
{
    ui->setupUi(this);
    RescanPorts();
}

void MainWindow::RescanPorts() {
    ports_ = com_ports::FetchCOMPorts();
    ui->comBox->clear();
    for (int32_t i{0}; i < ports_.size(); ++i) {
        ui->comBox->addItem(ports_.at(i).friendly_name);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_comRescan_clicked()
{
    RescanPorts();
}


void MainWindow::on_startButton_clicked()
{
    close_callback_(ports_.at(ui->comBox->currentIndex()).index);
}

