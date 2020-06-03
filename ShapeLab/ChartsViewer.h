#pragma once
#ifndef CHARTSVIEWER_H
#define CHARTSVIEWER_H

#include <QMainWindow>
#include <QSignalMapper>
#include <QStandardItemModel>
#include <omp.h>
#include <QTimer>
#include <QLabel>


using namespace std;

class DeformTet;

namespace Ui {
    class ChartsViewer;
}

class ChartsViewer : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChartsViewer(QWidget* parent = 0);
    ~ChartsViewer();

public slots:



private:
    Ui::ChartsViewer* ui;

private:

private:// functions for G code Generation.
                          // Layers Dir Files

protected:

private slots:
};

#endif // CHARTSVIEWER_H