#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSignalMapper>
#include <QStandardItemModel>
#include "../GLKLib/GLKLib.h"
#include "../QMeshLib/PolygenMesh.h"
#include <omp.h>
#include <QTimer>
#include <QLabel>

using namespace std;

class DeformTet;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
	// Qtimer - defined function
	void doTimerShapeUpDeform();
    void tianGcode2ABB();
private:
    Ui::MainWindow *ui;
    GLKLib *pGLK;
	QTimer deform_timer; //deformation timer
   
	GLKObList polygenMeshList;

private:
    void createActions();
    void createTreeView();
	void showTetraDeformationRatio();
	void MoveHandleRegion();
	void QTgetscreenshoot();

    PolygenMesh *getSelectedPolygenMesh();

    QSignalMapper *signalMapper;
    QStandardItemModel *treeModel;

	DeformTet *Deformation;

private:// functions for G code Generation.
    //void natSort(string dirctory, vector<string>& fileNameCell);	// function for Nature Sorting.
    //void readWayPointData(string packName);                         // function for WayPoint Data loading
    //void readSliceData(string sliceSetName);                        // function for Slice Data loading
    //void readExtruderHeadfile(string extruderHeadName);             // function for Extruder Head file loading
    //void readPlatformfile(string platformName);                     // function for Platform file loading

    vector<string> wayPointFileCell;	                            // Waypoints Dir Files
    vector<string> sliceSetFileCell;	                            // Layers Dir Files

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void open();
    void save();
	void saveSelection();
	void readSelection();

    void signalNavigation(int flag);
    void shiftToOrigin();
    void updateTree();
	void mouseMoveEvent(QMouseEvent *event);
    void on_pushButton_clearAll_clicked();
    void on_treeView_clicked(const QModelIndex &index);

	/*This is shape up deformation*/
	void runShapeUpDeformation(); 
	/*This is Gcode Generation*/
    //void runGcodeGeneration();
};

#endif // MAINWINDOW_H