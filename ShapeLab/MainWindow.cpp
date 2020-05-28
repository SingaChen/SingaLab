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
#include "FiveAxisPoint.h"
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
	//connect(ui->pushButton_RunDeformation, SIGNAL(released()), this, SLOT(runShapeUpDeformation()));
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

void MainWindow::tianGcode2ABB()
{
	fiveAxisPoint* initialpoint = new fiveAxisPoint();
	bool Yup2Zup_switch = 1;
	double Xoff = 0;
	double Yoff = 0;
	double Zoff = 0;

	initialpoint->natSort(ui->lineEdit_PosNorFileDir->text(), wayPointFileCell);
	initialpoint->readWayPointData(ui->lineEdit_PosNorFileDir->text(), Yup2Zup_switch,Xoff,Yoff,Zoff,&polygenMeshList,wayPointFileCell, pGLK);
	updateTree();
	viewAllWaypointLayers();
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
			initialpoint->readSliceData(ui->lineEdit_OFFLayerFile->text(), Yup2Zup_switch, Xoff, Yoff, Zoff ,&polygenMeshList, sliceSetFileCell,pGLK);
			
			updateTree();
			viewAllWaypointLayers();
		}
	}
	initialpoint->readExtruderHeadfile("extruderHead.obj", &polygenMeshList, pGLK);
	updateTree();
	
	initialpoint->readPlatformfile("printingPlatform.obj", &polygenMeshList, pGLK);
	updateTree();

	std::cout << "Gcode Generation__ Running ..." << std::endl;

	int GcodeGeneRange_From = 0;
	int GcodeGeneRange_To = wayPointFileCell.size() - 1;
	bool varyThickness_switch = ui->checkBox_varyHeight->isChecked();
	double UpZHeight = 0;
	bool varyWidth_switch = false;
	bool collisionDetection_switch = ui->checkBox_collisionDetection->isChecked();
	bool testXYZBCE_switch = false;
	bool testLayerHeight_switch = false;
	bool testW_switch = false;
	string targetFileName = (ui->lineEdit_targetFileName->text()).toStdString();
	double E3_Xoff = 0;
	double E3_Yoff = 0;
	double Xmove = 0;
	double Ymove = 0;
	PolygenMesh* Slices;
	PolygenMesh* Waypoints;
	PolygenMesh* extruderHead;
	PolygenMesh* PrintPlatform;
	for (GLKPOSITION pos = polygenMeshList.GetHeadPosition(); pos != nullptr;) {
		PolygenMesh* polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
		if ("Slices" == polygenMesh->getModelName()) { Slices = polygenMesh; }
		if ("Waypoints" == polygenMesh->getModelName()) { Waypoints = polygenMesh; }
		if ("ExtruderHead" == polygenMesh->getModelName()) { extruderHead = polygenMesh; }
		if ("PrintPlatform" == polygenMesh->getModelName()) { PrintPlatform = polygenMesh; }
	}
	initialpoint->getLayerHeight(Slices, Waypoints, PrintPlatform, varyThickness_switch, GcodeGeneRange_From, GcodeGeneRange_To, false, UpZHeight, Xmove, Ymove);
	initialpoint->singularityOpt(Waypoints, GcodeGeneRange_From, GcodeGeneRange_To);
	initialpoint->height2E(Waypoints, GcodeGeneRange_From, GcodeGeneRange_To, varyThickness_switch);
	initialpoint->writeGcode(Waypoints, targetFileName, GcodeGeneRange_From, GcodeGeneRange_To, E3_Xoff, E3_Yoff);
	initialpoint->writeABBGcode(Waypoints, targetFileName, GcodeGeneRange_From, GcodeGeneRange_To, E3_Xoff, E3_Yoff);
	viewAllWaypointLayers();
	pGLK->refresh(true);
	delete initialpoint;
	
}

void MainWindow::viewAllWaypointLayers() 
{
	for (GLKPOSITION pos = polygenMeshList.GetHeadPosition(); pos != nullptr;) {
		PolygenMesh* polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
		if ("Waypoints" != polygenMesh->getModelName() && "Slices" != polygenMesh->getModelName()) continue;

		for (GLKPOSITION posMesh = polygenMesh->GetMeshList().GetHeadPosition(); posMesh != nullptr;) {
			QMeshPatch* Patch = (QMeshPatch*)polygenMesh->GetMeshList().GetNext(posMesh);
			Patch->drawThisWaypointPatch = true;

			Patch->drawSingularity = false;
			/*if (ui->checkBox_showSingularityNode->isChecked()) {
				Patch->drawSingularity = true;
			}*/
		}
	}
	pGLK->refresh(true);
}