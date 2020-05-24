#include "stdafx.h"

#include "MainWindow.h"
#include "ui_MainWindow.h"
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

#include <fstream>

#include "../GLKLib/GLKCameraTool.h"
#include "../GLKLib/InteractiveTool.h"
#include "../GLKLib/GLKMatrixLib.h"
#include "../GLKLib/GLKGeometry.h"
#include "../QMeshLib/QMeshPatch.h"
#include "../QMeshLib/QMeshTetra.h"
#include "../QMeshLib/QMeshFace.h"
#include "../QMeshLib/QMeshEdge.h"
#include "../QMeshLib/QMeshNode.h"

#include "DeformTet.h"
#include "GcodeGeneration.h"
#include "FiveAxisPoint.h"
#include "alphanum.hpp"
#include <dirent.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    signalMapper = new QSignalMapper(this);
    addToolBar(ui->toolBar);
    addToolBar(ui->navigationToolBar);
    addToolBar(ui->selectionToolBar);

    createTreeView();
    createActions();

    pGLK = new GLKLib();
    ui->horizontalLayout->addWidget(pGLK);
    ui->horizontalLayout->setMargin(0);
    pGLK->setFocus();

    pGLK->clear_tools();
    pGLK->set_tool(new GLKCameraTool(pGLK,ORBITPAN));
	
	//connect timer with timer function
	connect(&deform_timer, SIGNAL(timeout()), this, SLOT(doTimerShapeUpDeform()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createActions()
{
    // file IO
    connect(ui->actionOpen, SIGNAL(triggered(bool)), this, SLOT(open()));
    connect(ui->actionSave, SIGNAL(triggered(bool)), this, SLOT(save()));
	connect(ui->actionSaveSelection, SIGNAL(triggered(bool)), this, SLOT(saveSelection()));
	connect(ui->actionReadSelection, SIGNAL(triggered(bool)), this, SLOT(readSelection()));

    // navigation
    connect(ui->actionFront, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionBack, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionTop, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionBottom, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionLeft, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionRight, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionIsometric, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionZoom_In, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionZoom_Out, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionZoom_All, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionZoom_Window, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    signalMapper->setMapping (ui->actionFront, 0);
    signalMapper->setMapping (ui->actionBack, 1);
    signalMapper->setMapping (ui->actionTop, 2);
    signalMapper->setMapping (ui->actionBottom, 3);
    signalMapper->setMapping (ui->actionLeft, 4);
    signalMapper->setMapping (ui->actionRight, 5);
    signalMapper->setMapping (ui->actionIsometric, 6);
    signalMapper->setMapping (ui->actionZoom_In, 7);
    signalMapper->setMapping (ui->actionZoom_Out, 8);
    signalMapper->setMapping (ui->actionZoom_All, 9);
    signalMapper->setMapping (ui->actionZoom_Window, 10);

    // view
    connect(ui->actionShade, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionMesh, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionNode, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionProfile, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionFaceNormal, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionNodeNormal, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    signalMapper->setMapping (ui->actionShade, 20);
    signalMapper->setMapping (ui->actionMesh, 21);
    signalMapper->setMapping (ui->actionNode, 22);
    signalMapper->setMapping (ui->actionProfile, 23);
    signalMapper->setMapping (ui->actionFaceNormal, 24);
    signalMapper->setMapping (ui->actionNodeNormal, 25);
    ui->actionShade->setChecked(true);

    connect(ui->actionShifttoOrigin, SIGNAL(triggered(bool)), this, SLOT(shiftToOrigin()));

    // select
    connect(ui->actionSelectNode, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionSelectEdge, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionSelectFace, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
	connect(ui->actionSelectFix, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
	connect(ui->actionSelectHandle, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));

	signalMapper->setMapping (ui->actionSelectNode, 30);
    signalMapper->setMapping (ui->actionSelectEdge, 31);
    signalMapper->setMapping (ui->actionSelectFace, 32);
	signalMapper->setMapping(ui->actionSelectFix, 33);
	signalMapper->setMapping(ui->actionSelectHandle, 34);


    connect (signalMapper, SIGNAL(mapped(int)), this, SLOT(signalNavigation(int)));

	//Button
	connect(ui->pushButton_RunDeformation, SIGNAL(released()), this, SLOT(runShapeUpDeformation()));
	//connect(ui->pushButton_thicknessVariation, SIGNAL(released()), this, SLOT(LoadSlices()));
	//connect(ui->pushButton_thicknessVariation, SIGNAL(released()), this, SLOT(extrude2ThicknessVariation()));	
	connect(ui->pushButton_GcodeGeneration, SIGNAL(released()), this, SLOT(runGcodeGeneration()));
	connect(ui->pushButton_read, SIGNAL(released()), this, SLOT(tianGcode2ABB()));

	
}

void MainWindow::open()
{
    QString filenameStr = QFileDialog::getOpenFileName(this, tr("Open File,"), "..", tr(""));
    QFileInfo fileInfo(filenameStr);
    QString fileSuffix = fileInfo.suffix();
    QByteArray filenameArray = filenameStr.toLatin1();
    char *filename = filenameArray.data();

    // set polygen name
    std::string strFilename(filename);
    std::size_t foundStart = strFilename.find_last_of("/");
    std::size_t foundEnd = strFilename.find_last_of(".");
    std::string modelName;
    modelName = strFilename.substr(0,foundEnd);
    modelName = modelName.substr(foundStart+1);
    
    if (QString::compare(fileSuffix,"obj") == 0){
        PolygenMesh *polygenMesh = new PolygenMesh;
        polygenMesh->ImportOBJFile(filename,modelName);
        polygenMesh->BuildGLList(polygenMesh->m_bVertexNormalShading);
        pGLK->AddDisplayObj(polygenMesh,true);
        polygenMeshList.AddTail(polygenMesh);
    }

	else if (QString::compare(fileSuffix, "tet") == 0) {
		PolygenMesh *polygenMesh = new PolygenMesh;
		std::cout << filename << std::endl;
		std::cout << modelName << std::endl;
		polygenMesh->ImportTETFile(filename, modelName);
		polygenMesh->BuildGLList(polygenMesh->m_bVertexNormalShading);
		pGLK->AddDisplayObj(polygenMesh, true);
		polygenMeshList.AddTail(polygenMesh);
	}

    updateTree();

    shiftToOrigin();
    pGLK->refresh(true);
}

void MainWindow::save()
{
	PolygenMesh *polygenMesh = getSelectedPolygenMesh();
	if (!polygenMesh)
		polygenMesh = (PolygenMesh*)polygenMeshList.GetHead();
	if (!polygenMesh)
		return;
	QString filenameStr = QFileDialog::getSaveFileName(this, tr("OBJ File Export,"), "..", tr("OBJ(*.obj)"));
	QFileInfo fileInfo(filenameStr);
	QString fileSuffix = fileInfo.suffix();

	if (QString::compare(fileSuffix, "obj") == 0) {
		QFile exportFile(filenameStr);
		if (exportFile.open(QFile::WriteOnly | QFile::Truncate)) {
			QTextStream out(&exportFile);
			for (GLKPOSITION posMesh = polygenMesh->GetMeshList().GetHeadPosition(); posMesh != nullptr;) {
				QMeshPatch *patch = (QMeshPatch*)polygenMesh->GetMeshList().GetNext(posMesh);
				for (GLKPOSITION posNode = patch->GetNodeList().GetHeadPosition(); posNode != nullptr;) {
					QMeshNode *node = (QMeshNode*)patch->GetNodeList().GetNext(posNode);
					double xx, yy, zz;
					node->GetCoord3D(xx, yy, zz);
					float r, g, b;
					node->GetColor(r, g, b);
					out << "v " << xx << " " << yy << " " << zz << " " << node->value1 << endl;
				}
				for (GLKPOSITION posFace = patch->GetFaceList().GetHeadPosition(); posFace != nullptr;) {
					QMeshFace *face = (QMeshFace*)patch->GetFaceList().GetNext(posFace);
					out << "f " << face->GetNodeRecordPtr(0)->GetIndexNo() << " " << face->GetNodeRecordPtr(1)->GetIndexNo() << " " << face->GetNodeRecordPtr(2)->GetIndexNo() << endl;
				}
			}
		}
		exportFile.close();
	}
}

void MainWindow::saveSelection()
{
	//printf("%s exported\n", Model->ModelName);

	PolygenMesh *polygenMesh = getSelectedPolygenMesh();
	if (!polygenMesh)
		polygenMesh = (PolygenMesh*)polygenMeshList.GetHead();
	QMeshPatch *patch = (QMeshPatch*)polygenMesh->GetMeshList().GetHead();

	std::string filename = polygenMesh->getModelName();
	const char * c = filename.c_str();
	char *cstr = new char[filename.length() + 1];
	strcpy(cstr, filename.c_str());

	const char * split = ".";
	char* p = strtok(cstr, split);

	char output_filename[256];
	strcpy(output_filename, "..\\selection_file\\");
	strcat(output_filename, cstr);
	char filetype[64];
	strcpy(filetype, ".txt");
	strcat(output_filename, filetype);

	ofstream nodeSelection(output_filename);
	if (!nodeSelection)
		cerr << "Sorry!We were unable to build the file NodeSelect!\n";
	for (GLKPOSITION Pos = patch->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode *CheckNode = (QMeshNode*)patch->GetNodeList().GetNext(Pos);
		nodeSelection << CheckNode->GetIndexNo() << ":";
		//for the selection of fixing part
		if (CheckNode->isFixed == true) nodeSelection << "1:";
		else nodeSelection << "0:";
		//for the selection of hard part
		if (CheckNode->isHandle == true) nodeSelection << "1:" << endl;
		else nodeSelection << "0:" << endl;
	}

	nodeSelection.close();
	printf("Finish output selection \n");
}

void MainWindow::readSelection()
{
	PolygenMesh *polygenMesh = getSelectedPolygenMesh();
	if (!polygenMesh)
		polygenMesh = (PolygenMesh*)polygenMeshList.GetHead();
	QMeshPatch *patch = (QMeshPatch*)polygenMesh->GetMeshList().GetHead();

	std::string filename = polygenMesh->getModelName();
	const char * c = filename.c_str();

	char *cstr = new char[filename.length() + 1];
	strcpy(cstr, filename.c_str());

	const char * split = ".";
	char* p = strtok(cstr, split);

	char input_filename[256];
	strcpy(input_filename, "..\\selection_file\\");
	strcat(input_filename, cstr);
	char filetype[64];
	strcpy(filetype, ".txt");
	strcat(input_filename, filetype);

	ifstream nodeSelect(input_filename);
	if (!nodeSelect)
		cerr << "Sorry!We were unable to open the file!\n";
	vector<int> NodeIndex(patch->GetNodeNumber()), checkNodeFixed(patch->GetNodeNumber()), checkNodeHandle(patch->GetNodeNumber());
	//string line;
	int LineIndex1 = 0;
	string sss;
	while (getline(nodeSelect, sss)){
		const char * c = sss.c_str();
		sscanf(c, "%d:%d:%d", &NodeIndex[LineIndex1], &checkNodeFixed[LineIndex1], &checkNodeHandle[LineIndex1]);
		LineIndex1++;
	}

	nodeSelect.close();
	for (GLKPOSITION Pos = patch->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode *CheckNode = (QMeshNode*)patch->GetNodeList().GetNext(Pos);
		if (checkNodeFixed[CheckNode->GetIndexNo() - 1] == 1) CheckNode->isFixed = true;
		if (checkNodeHandle[CheckNode->GetIndexNo() - 1] == 1) CheckNode->isHandle = true;
	}

	for (GLKPOSITION Pos = patch->GetFaceList().GetHeadPosition(); Pos != NULL;)
	{
		QMeshFace* Face = (QMeshFace*)patch->GetFaceList().GetNext(Pos);
		if (Face->GetNodeRecordPtr(0)->isHandle == true &&
			Face->GetNodeRecordPtr(1)->isHandle == true &&
			Face->GetNodeRecordPtr(2)->isHandle == true)
			Face->isHandleDraw = true;
		else Face->isHandleDraw = false;

		if (Face->GetNodeRecordPtr(0)->isFixed == true &&
			Face->GetNodeRecordPtr(1)->isFixed == true &&
			Face->GetNodeRecordPtr(2)->isFixed == true)
			Face->isFixedDraw = true;
		else Face->isFixedDraw = false;
	}
	printf("Finish input selection \n");
	pGLK->refresh(true);

}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
	//QMouseEvent *e = (QMouseEvent*)event;
	//QPoint pos = e->pos();
	//cout << "Mouse position updated" << endl;
	//double wx, wy, wz;
	//pGLK->screen_to_wcl(100.0, 100.0, wx, wy, wz);
	//ui->CorrdinateMouse->setText(QString("X = %1").arg(wx));

	//QString text;
	//text = QString("%1 X %2").arg(event->pos().x()).arg(event->pos().y());
	///** Update the info text */
	//ui->statusBar->showMessage(text);
}

void MainWindow::signalNavigation(int flag)
{
    if (flag <= 10)
        pGLK->setNavigation(flag);
    if (flag >=20 && flag <=25){
        pGLK->setViewModel(flag-20);
        switch (flag) {
        case 20:
            ui->actionShade->setChecked(pGLK->getViewModel(0));
            break;
        case 21:
            ui->actionMesh->setChecked(pGLK->getViewModel(1));
            break;
        case 22:
            ui->actionNode->setChecked(pGLK->getViewModel(2));
            break;
        case 23:
            ui->actionProfile->setChecked(pGLK->getViewModel(3));
            break;
        case 24:
            ui->actionFaceNormal->setChecked(pGLK->getViewModel(4));
            break;
        case 25:
            ui->actionNodeNormal->setChecked(pGLK->getViewModel(5));
            break;
        }
    }
    if (flag==30 || flag==31 || flag==32 || flag == 33 || flag == 34){
        InteractiveTool *tool;
        switch (flag) {
        case 30:
            tool = new InteractiveTool(pGLK, &polygenMeshList, (GLKMouseTool*)pGLK->GetCurrentTool(), NODE, ui->boxDeselect->isChecked());
            break;
        case 31:
            tool = new InteractiveTool(pGLK, &polygenMeshList, (GLKMouseTool*)pGLK->GetCurrentTool(), EDGE, ui->boxDeselect->isChecked());
            break;
        case 32:
            tool = new InteractiveTool(pGLK, &polygenMeshList, (GLKMouseTool*)pGLK->GetCurrentTool(), FACE, ui->boxDeselect->isChecked());
            break;
		case 33:
			tool = new InteractiveTool(pGLK, &polygenMeshList, (GLKMouseTool*)pGLK->GetCurrentTool(), FIX, ui->boxDeselect->isChecked());
			break;
		case 34:
			tool = new InteractiveTool(pGLK, &polygenMeshList, (GLKMouseTool*)pGLK->GetCurrentTool(), NHANDLE, ui->boxDeselect->isChecked());
			break;
        }
        pGLK->set_tool(tool);
    }
}

void MainWindow::shiftToOrigin()
{
    
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QString filenameStr;
    foreach (const QUrl &url, event->mimeData()->urls())
        filenameStr = url.toLocalFile();
    QByteArray filenameArray = filenameStr.toLatin1();
    char *filename = filenameArray.data();

    PolygenMesh *polygenMesh = new PolygenMesh;

    // set polygen name
    std::string strFilename(filename);
    std::size_t foundStart = strFilename.find_last_of("/");
    std::size_t foundEnd = strFilename.find_last_of(".");
    std::string modelName;
    modelName = strFilename.substr(0,foundEnd);
    modelName = modelName.substr(foundStart+1);
    int i = 0;
    for (GLKPOSITION pos=polygenMeshList.GetHeadPosition(); pos!=nullptr;){
        PolygenMesh *polygen = (PolygenMesh*)polygenMeshList.GetNext(pos);
        std::string name = (polygen->getModelName()).substr(0,(polygen->getModelName()).find(' '));
        if (name == modelName)
            i++;
    }
    if (i > 0)
        modelName += " "+std::to_string(i);

	QFileInfo fileInfo(filenameStr);
	QString fileSuffix = fileInfo.suffix();
	if (QString::compare(fileSuffix, "obj") == 0) {
		polygenMesh->ImportOBJFile(filename, modelName);
	}
	else if (QString::compare(fileSuffix, "tet") == 0) {
		polygenMesh->ImportTETFile(filename, modelName);
	}
	polygenMesh->m_bVertexNormalShading = false;	
    polygenMesh->BuildGLList(polygenMesh->m_bVertexNormalShading);
    pGLK->AddDisplayObj(polygenMesh,true);
    polygenMeshList.AddTail(polygenMesh);

    updateTree();
}

void MainWindow::createTreeView()
{
    treeModel = new QStandardItemModel();
    ui->treeView->setModel(treeModel);
    ui->treeView->setHeaderHidden(true);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeView->expandAll();
}

void MainWindow::updateTree()
{
    treeModel->clear();
    for (GLKPOSITION pos=polygenMeshList.GetHeadPosition(); pos!=nullptr;){
        PolygenMesh *polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
        QString modelName = QString::fromStdString(polygenMesh->getModelName());
        QStandardItem *modelListItem = new QStandardItem(modelName);
        modelListItem->setCheckable(true);
        modelListItem->setCheckState(Qt::Checked);
        treeModel->appendRow(modelListItem);
    }
	pGLK->refresh(true);
}

PolygenMesh *MainWindow::getSelectedPolygenMesh()
{
    if (!treeModel->hasChildren())
        return nullptr;
    QModelIndex index = ui->treeView->currentIndex();
    QString selectedModelName = index.data(Qt::DisplayRole).toString();
    for (GLKPOSITION pos=polygenMeshList.GetHeadPosition(); pos!=nullptr;){
        PolygenMesh *polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
        QString modelName = QString::fromStdString(polygenMesh->getModelName());
        if (QString::compare(selectedModelName,modelName) == 0)
            return polygenMesh;
    }
    return nullptr;
}

void MainWindow::on_pushButton_clearAll_clicked()
{
    int i = 0;
    for (GLKPOSITION pos=polygenMeshList.GetHeadPosition(); pos!=nullptr; i++){
        PolygenMesh *polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
        QMeshPatch *patch = (QMeshPatch*)polygenMesh->GetMeshList().GetHead();
        if (i<2)
            continue;
        for (GLKPOSITION pos2=patch->GetFaceList().GetHeadPosition(); pos2!=nullptr;){
            QMeshFace *face = (QMeshFace*)patch->GetFaceList().GetNext(pos2);
            face->m_nIdentifiedPatchIndex = 0;
        }
    }
    pGLK->refresh(true);
}

void MainWindow::on_treeView_clicked(const QModelIndex &index)
{
    ui->treeView->currentIndex();
    QStandardItem *modelListItem = treeModel->itemFromIndex(index);
    ui->treeView->setCurrentIndex(index);
    PolygenMesh *polygenMesh = getSelectedPolygenMesh();
    if (modelListItem->checkState() == Qt::Checked)
        polygenMesh->bShow = true;
    else
        polygenMesh->bShow = false;
    pGLK->refresh(true);
}

void MainWindow::runShapeUpDeformation() {

	printf("RunShapeUpDeformation!\n");
	PolygenMesh *polygenMesh = getSelectedPolygenMesh();
	if (!polygenMesh)
		polygenMesh = (PolygenMesh*)polygenMeshList.GetHead();
	QMeshPatch *patch = (QMeshPatch*)polygenMesh->GetMeshList().GetHead();

	Deformation = new DeformTet();
	Deformation->SetMesh(patch);

	Deformation->PreProcess();

	//SORODeform->Run(100);
	//pGLK->refresh(true);

	deform_timer.start(1);
	//QMessageBox::information(this, "title", "This is a test");
	cout << "The end of this function" << endl;
}

void MainWindow::doTimerShapeUpDeform() 
{
	static int itertime = 0;
	static int total_iter = 70;
	if (itertime % 10 == 0 && itertime < total_iter) MoveHandleRegion();

	Deformation->Run(1);
	itertime++;

	if (itertime >= total_iter+20) {
		Deformation->DoTopologyAnalysis(false);
		deform_timer.stop();
		pGLK->refresh(true);
	}
}

void MainWindow::MoveHandleRegion() {
	PolygenMesh *polygenMesh = getSelectedPolygenMesh();
	if (!polygenMesh)
		polygenMesh = (PolygenMesh*)polygenMeshList.GetHead();
	QMeshPatch *patch = (QMeshPatch*)polygenMesh->GetMeshList().GetHead();

	//Linearly move the handle region
	for (GLKPOSITION Pos = patch->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode *node = (QMeshNode*)patch->GetNodeList().GetNext(Pos);
		if (node->isHandle) {
			double pp[3];
			node->GetCoord3D(pp[0], pp[1], pp[2]);
			pp[1] -= 0.2;
			node->SetCoord3D(pp[0], pp[1], pp[2]);
		}
	}
}

void MainWindow::runGcodeGeneration() {

	cout << "Function __GcodeGeneration__ Start." << endl;

	string PosNorFileDir = (ui->lineEdit_PosNorFileDir->text()).toStdString();
	string OFFLayerFileDir = (ui->lineEdit_OFFLayerFile->text()).toStdString();
	bool varyThickness_switch = ui->checkBox_varyHeight->isChecked();
	string targetFileName = (ui->lineEdit_targetFileName->text()).toStdString();
	bool collisionDetection_switch = ui->checkBox_collisionDetection->isChecked();


	natSort(PosNorFileDir, wayPointFileCell);
	//test for sort
	//cout << "There are " << wayPointFileCell.size() << " files in the current directory." << endl;
	//for (vector<string>::const_iterator iter = wayPointFileCell.cbegin(); iter != wayPointFileCell.cend(); iter++){
	//cout << (*iter) << endl;
	//}

	readWayPointData(PosNorFileDir);

	if (varyThickness_switch == true) {
		natSort(OFFLayerFileDir, sliceSetFileCell);
		if (wayPointFileCell.size() != sliceSetFileCell.size()) {
			cout << "Error: Layers file num != Waypoints file num" << endl;
			return;
		}
		else { readSliceData(OFFLayerFileDir); }
	}

	readExtruderHeadfile("extruderHead.obj");
	readPlatformfile("printingPlatform.obj");
	
	PolygenMesh* polygenMesh_Slices;
	PolygenMesh* polygenMesh_Waypoints;
	PolygenMesh* polygenMesh_extruderHead;
	for (GLKPOSITION pos = polygenMeshList.GetHeadPosition(); pos != nullptr;) {
		PolygenMesh* polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
		if ("Slices" == polygenMesh->getModelName()) { polygenMesh_Slices = polygenMesh; }
		if ("Waypoints" == polygenMesh->getModelName()) { polygenMesh_Waypoints = polygenMesh; }
		if ("ExtruderHead" == polygenMesh->getModelName()) { polygenMesh_extruderHead = polygenMesh; }
	}

	GcodeGeneration* GcodeGene = new GcodeGeneration();
	GcodeGene->getLayerHeight(polygenMesh_Slices, polygenMesh_Waypoints, varyThickness_switch);
	GcodeGene->getUpZwayPnts(polygenMesh_Waypoints);
	GcodeGene->singularityOpt(polygenMesh_Waypoints);
	GcodeGene->detectCollision(polygenMesh_Waypoints, polygenMesh_extruderHead, collisionDetection_switch);
	GcodeGene->height2E(polygenMesh_Waypoints,varyThickness_switch);
	GcodeGene->writeGcode(polygenMesh_Waypoints, targetFileName);

	/*

	//for (GLKPOSITION Pos = polygenMesh_Waypoints->GetMeshList().GetHeadPosition(); Pos;) {
	//	QMeshPatch* WayPointPatch = (QMeshPatch*)polygenMesh_Waypoints->GetMeshList().GetNext(Pos);

	//	for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
	//		QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

	//		double Px, Py, Pz, Nx, Ny, Nz;
	//		Px = Node->m_orginalPostion[0];
	//		Py = Node->m_orginalPostion[1];
	//		Pz = Node->m_orginalPostion[2];
	//		Nx = Node->m_orginalNormal[0];
	//		Ny = Node->m_orginalNormal[1];
	//		Nz = Node->m_orginalNormal[2];
	//		
	//		//cout << Px << " " << Py << " " << Pz << " " << Nx << " " << Ny << " " << Nz << endl;
	//	}
	//	int a = WayPointPatch->GetIndexNo();
	//	cout << "--------------" << endl;

	//}

	*/
	pGLK->refresh(true);
	delete GcodeGene; 

	std::cout << "Function __GcodeGeneration__ End." << std::endl;
}

void MainWindow::natSort(string dirctory, vector<string>& fileNameCell) {
	
	if (fileNameCell.empty() == false) return;
	
	DIR* dp;
	struct dirent* ep;
	string fullDir = "../1_GcodeGeneModel/" + dirctory;
	//cout << fullDir << endl;
	dp = opendir(fullDir.c_str());
	//dp = opendir("../Waypoints");

	if (dp != NULL) {
		while (ep = readdir(dp)) {
			//cout << ep->d_name << endl;
			if ((string(ep->d_name) != ".") && (string(ep->d_name) != "..")) {
				//cout << ep->d_name << endl;
				fileNameCell.push_back(string(ep->d_name));
			}
		}
		(void)closedir(dp);
	}
	else {
		perror("Couldn't open the directory");
	}
	//resort the files with nature order
	sort(fileNameCell.begin(), fileNameCell.end(), doj::alphanum_less<std::string>());

}

void MainWindow::readWayPointData(string packName) {

	// isbuiled
	for (GLKPOSITION pos = polygenMeshList.GetHeadPosition(); pos != nullptr;) {
		PolygenMesh* polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
		if ("Waypoints" == polygenMesh->getModelName()) return;
	}

	PolygenMesh* waypointSet = new PolygenMesh;
	waypointSet->setModelName("Waypoints");

	//read slice files and build mesh_patches
	char filename[1024];

	for (int i = 0; i < wayPointFileCell.size(); i++) {

		sprintf(filename, "%s%s%s%s", "../1_GcodeGeneModel/", packName.c_str(), "/", wayPointFileCell[i].data());
		// cout << wayPointFileCell[i].data() << endl;

		QMeshPatch* waypoint = new QMeshPatch;
		waypoint->SetIndexNo(waypointSet->GetMeshList().GetCount()); //index begin from 0
		waypointSet->GetMeshList().AddTail(waypoint);
		
		// isSupportLayer
		string::size_type supportFlag = wayPointFileCell[i].find("S");
		if (supportFlag == string::npos)	waypoint->isSupportLayer = false;
		else { waypoint->isSupportLayer = true; }

		//cout << waypoint->isSupportLayer << endl;

		waypoint->inputPosNorFile(filename, waypoint->isSupportLayer);
			
		std::cout << ".";
		if (((i + 1) % 100 == 0) || ((i + 1) == wayPointFileCell.size())) std::cout << std::endl;
	}
	////Display
	waypointSet->BuildGLList(waypointSet->m_bVertexNormalShading);
	polygenMeshList.AddTail(waypointSet);
	pGLK->AddDisplayObj(waypointSet, true);
	updateTree();
	pGLK->refresh(true);
	std::cout << "------------------------------------------- WayPoints Load OK!" << std::endl;
}

void MainWindow::readSliceData(string sliceSetName) {

	// isbuiled
	for (GLKPOSITION pos = polygenMeshList.GetHeadPosition(); pos != nullptr;) {
		PolygenMesh* polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
		if ("Slices" == polygenMesh->getModelName()) return;
	}

	PolygenMesh* sliceSet = new PolygenMesh;
	sliceSet->setModelName("Slices");

	//read slice files and build mesh_patches
	char filename[1024];
	for (int i = 0; i < sliceSetFileCell.size(); i++)
	{
		sprintf(filename, "%s%s%s%s", "../1_GcodeGeneModel/", sliceSetName.c_str(), "/", sliceSetFileCell[i].data());
		//cout << filename << endl;

		QMeshPatch* slice = new QMeshPatch;
		slice->SetIndexNo(sliceSet->GetMeshList().GetCount()); //index begin from 0
		sliceSet->GetMeshList().AddTail(slice);
		slice->inputOFFFile(filename, false);

		cout << ".";
		if (((i + 1) % 100 == 0) || ((i + 1) == sliceSetFileCell.size())) cout << endl;
	}
	//Display
	sliceSet->BuildGLList(sliceSet->m_bVertexNormalShading);
	polygenMeshList.AddTail(sliceSet);
	pGLK->AddDisplayObj(sliceSet, true);
	updateTree();
	// pGLK->refresh(true);

	cout << "------------------------------------------- Slices Load OK!" << endl;

}

void MainWindow::readExtruderHeadfile(string extruderHeadName) {

	// isbuiled
	for (GLKPOSITION pos = polygenMeshList.GetHeadPosition(); pos != nullptr;) {
		PolygenMesh* polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
		if ("ExtruderHead" == polygenMesh->getModelName()) return;
	}

	PolygenMesh* extruderHead = new PolygenMesh;
	extruderHead->setModelName("ExtruderHead");

	//read slice files and build mesh_patches
	char filename[1024];

	sprintf(filename, "%s%s", "../1_GcodeGeneModel/", extruderHeadName.c_str());
	//cout << filename << endl;

	QMeshPatch* eHead = new QMeshPatch;
	eHead->SetIndexNo(extruderHead->GetMeshList().GetCount()); //index begin from 0
	extruderHead->GetMeshList().AddTail(eHead);
	eHead->inputOBJFile(filename, false);

	////Display
	extruderHead->BuildGLList(extruderHead->m_bVertexNormalShading);
	polygenMeshList.AddTail(extruderHead);
	pGLK->AddDisplayObj(extruderHead, true);
	updateTree();
	pGLK->refresh(true);

	cout << "------------------------------------------- Extruder Head Load OK!" << endl;

}

void MainWindow::readPlatformfile(string platformName) {

	// isbuiled
	for (GLKPOSITION pos = polygenMeshList.GetHeadPosition(); pos != nullptr;) {
		PolygenMesh* polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
		if ("PrintPlatform" == polygenMesh->getModelName()) return;
	}

	PolygenMesh* extruderHead = new PolygenMesh;
	extruderHead->setModelName("PrintPlatform");

	//read slice files and build mesh_patches
	char filename[1024];

	sprintf(filename, "%s%s", "../1_GcodeGeneModel/", platformName.c_str());
	//cout << filename << endl;

	QMeshPatch* eHead = new QMeshPatch;
	eHead->SetIndexNo(extruderHead->GetMeshList().GetCount()); //index begin from 0
	extruderHead->GetMeshList().AddTail(eHead);
	eHead->inputOBJFile(filename, false);

	////Display
	extruderHead->BuildGLList(extruderHead->m_bVertexNormalShading);
	polygenMeshList.AddTail(extruderHead);
	pGLK->AddDisplayObj(extruderHead, true);
	updateTree();
	pGLK->refresh(true);

	cout << "------------------------------------------- Platform Load OK!" << endl;

}

void MainWindow::tianGcode2ABB()
{
	fiveAxisPoint* initialpoint = new fiveAxisPoint();
	initialpoint->natSort(ui->lineEdit_PosNorFileDir->text(), wayPointFileCell);
	initialpoint->readWayPointData(ui->lineEdit_PosNorFileDir->text(),polygenMeshList,wayPointFileCell, pGLK);
	updateTree();
	pGLK->refresh(true);
	updateTree();
	if (ui->checkBox_varyHeight->isChecked() == true) 
	{
		initialpoint->natSort((ui->lineEdit_OFFLayerFile->text()), sliceSetFileCell);
		if (wayPointFileCell.size() != sliceSetFileCell.size()) 
		{
			cout << "Error: Layers file num != Waypoints file num" << endl;
			return;
		}
		else 
		{
			initialpoint->readSliceData(ui->lineEdit_OFFLayerFile->text(),polygenMeshList, sliceSetFileCell,pGLK); 
		}
	}
	initialpoint->readExtruderHeadfile("extruderHead.obj", polygenMeshList, pGLK);
	updateTree();
	pGLK->refresh(true);
	initialpoint->readPlatformfile("printingPlatform.obj", polygenMeshList, pGLK);
	updateTree();
	pGLK->refresh(true);

	PolygenMesh* polygenMesh_Slices;
	PolygenMesh* polygenMesh_Waypoints;
	PolygenMesh* polygenMesh_extruderHead;
	for (GLKPOSITION pos = polygenMeshList.GetHeadPosition(); pos != nullptr;) {
		PolygenMesh* polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
		if ("Slices" == polygenMesh->getModelName()) { polygenMesh_Slices = polygenMesh; }
		if ("Waypoints" == polygenMesh->getModelName()) { polygenMesh_Waypoints = polygenMesh; }
		if ("ExtruderHead" == polygenMesh->getModelName()) { polygenMesh_extruderHead = polygenMesh; }
	}

	initialpoint->getLayerHeight(polygenMesh_Slices, polygenMesh_Waypoints, ui->checkBox_varyHeight->isChecked());
	initialpoint->getUpZwayPnts(polygenMesh_Waypoints);
	initialpoint->singularityOpt(polygenMesh_Waypoints);
	initialpoint->detectCollision(polygenMesh_Waypoints, polygenMesh_extruderHead, collisionDetection_switch);
	initialpoint->height2E(polygenMesh_Waypoints, varyThickness_switch);
	initialpoint->writeGcode(polygenMesh_Waypoints, targetFileName);
}

