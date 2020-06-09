
#ifndef CHARTSVIEWER_H
#define CHARTSVIEWER_H

//#include <QMainWindow>
//#include <QSignalMapper>
//#include <QStandardItemModel>
#include "../GLKLib/GLKLib.h"
#include "../QMeshLib/PolygenMesh.h"
#include <QtCharts>
QT_CHARTS_USE_NAMESPACE
//#include <omp.h>
//#include <QTimer>
//#include <QLabel>



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
    void getPolygenMeshList(GLKObList* polygenMeshList);


    QString currentPolygenMesh;
    QString currentQmeshPatch;
    QString currentWorkStep;
    QString currentParameter;
public slots:
    void drawchart();
    void update();
    void slotPointHoverd(const QPointF& point, bool state);

private:
    Ui::ChartsViewer* ui;
    
    GLKObList polygenMeshList;
private:
    void createActions();


private:// functions for G code Generation.
                          // Layers Dir Files

protected:

private slots:
};

#endif // CHARTSVIEWER_H