#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Offline EVM System");
    this->resize(1000, 650);

    scanPage = new EvmScanPage(this);
    this->setCentralWidget(scanPage);
}

MainWindow::~MainWindow()
{
    delete ui;
}


