#include "stdafx.h"

#include "ui_ChartsViewer.h"
#include <QFileDialog>
#include <QtDebug>
#include <QDesktopWidget>
#include <QCoreApplication>
#include <QMimeData>
#include <QTreeView>
#include <QThread>
#include <QTimer>
#include <QDateTime>
#include <QMessageBox>
#include <QScreen>
#include <QtCharts>
QT_CHARTS_USE_NAMESPACE
#include <fstream>
#include "ChartsViewer.h"
#include <dirent.h>


ChartsViewer::ChartsViewer(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::ChartsViewer)
{
    ui->setupUi(this);
}



ChartsViewer::~ChartsViewer()
{
    delete ui;
}