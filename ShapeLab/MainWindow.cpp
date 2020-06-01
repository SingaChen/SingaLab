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

#include <QChart>

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
	//connect(&deform_timer, SIGNAL(timeout()), this, SLOT(doTimerShapeUpDeform()));
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

	ui->label_RangeFrom->setDigitCount(3);
	ui->label_RangeTo->setDigitCount(3);
	
	connect(ui->horizontalSlider_RangeFrom, SIGNAL(valueChanged(int)), ui->label_RangeFrom, SLOT(display(int)));
	connect(ui->horizontalSlider_RangeTo, SIGNAL(valueChanged(int)), ui->label_RangeTo, SLOT(display(int)));
	//Button
	connect(ui->pushButton_selectWaypointFile, SIGNAL(released()), this, SLOT(selectDir()));
	connect(ui->pushButton_read, SIGNAL(released()), this, SLOT(tianGcode2ABB()));
	connect(ui->pushButton_test, SIGNAL(released()), this, SLOT(test_in_mainwindow()));
	connect(ui->pushButton_rangeActive, SIGNAL(released()), this, SLOT(QMeshPatchRangeActive()));
	
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
    ui->treeView->setHeaderHidden(false);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeView->expandAll();

	QStringList fonts;
	fonts << "ModelName" << "ModelType" << "NodelNum" << "FaceNum";
	treeModel->setHorizontalHeaderLabels(fonts);
	ui->treeView->setModel(treeModel);

}

void MainWindow::updateTree()
{
    treeModel->clear();
	QStringList fonts;
	fonts << "ModelName" << "ModelType" << "NodelNum" << "FaceNum";
	treeModel->setHorizontalHeaderLabels(fonts);
	ui->treeView->setModel(treeModel);

	


	int i = 0;
    for (GLKPOSITION pos = polygenMeshList.GetHeadPosition(); pos!=nullptr;){
        PolygenMesh *polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
        QString modelName = QString::fromStdString(polygenMesh->getModelName());
		
        QStandardItem *modelListItem = new QStandardItem(modelName);
		modelListItem->setData(QStringLiteral("POLYGENMESH"));
		
        modelListItem->setCheckable(true);
		//modelListItem->setTristate(true);
        modelListItem->setCheckState(Qt::Checked);
		treeModel->appendRow(modelListItem);
		//modelListItem->modelType = POLYGENMESH;
		treeModel->setItem(i++, 1, new QStandardItem(QStringLiteral("polygenMesh"))); //父节点的兄弟节点
		int j = 0;
		for (GLKPOSITION posChild = polygenMesh->GetMeshList().GetHeadPosition(); posChild;) 
		{
			QMeshPatch *qMeshPatch = (QMeshPatch*) polygenMesh->GetMeshList().GetNext(posChild);
			
			QString modelNameChild = QString::fromStdString(qMeshPatch->waypointPatchName);
			QStandardItem* modelListItemChild = new QStandardItem(modelNameChild);
			
			modelListItemChild->setData(QStringLiteral("QMESHPACTH"));
			modelListItemChild->setCheckable(true);
			modelListItemChild->setCheckState(Qt::Checked);
			modelListItem->appendRow(modelListItemChild);
			modelListItem->setChild(j++, 1, new QStandardItem(QStringLiteral("QMeshPatch")));
		}
    }
	pGLK->refresh(true);
}

PolygenMesh *MainWindow::getSelectedPolygenMesh()
{
    /*if (!treeModel->hasChildren())
        return nullptr;*/
    QModelIndex index = ui->treeView->currentIndex();
	QStandardItem* selectedModel = treeModel->itemFromIndex(index);
    QString selectedModelName = index.data(Qt::DisplayRole).toString();
    for (GLKPOSITION pos=polygenMeshList.GetHeadPosition(); pos!=nullptr;){
        PolygenMesh *polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
        QString modelName = QString::fromStdString(polygenMesh->getModelName());
        if (QString::compare(selectedModelName,modelName) == 0)
            return polygenMesh;
    }
    return nullptr;
}

int MainWindow::treeViewSelectType()
{
	QModelIndex index = ui->treeView->currentIndex();
	QStandardItem* selectedModel = treeModel->itemFromIndex(index);
	string selectedModelType = selectedModel->data().toString().toStdString();
	if (selectedModelType == "POLYGENMESH") return 1;
	else if (selectedModelType == "QMESHPATCH") return 2;
	else return 0;
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
    //ui->treeView->currentIndex();
    QStandardItem *modelListItem = treeModel->itemFromIndex(index);
	ui->treeView->setCurrentIndex(index);
	QString selectedModelName = index.data(Qt::DisplayRole).toString();
	switch (treeViewSelectType())
	{
	case 1:
		for (GLKPOSITION pos = polygenMeshList.GetHeadPosition(); pos != nullptr;) 
		{
			PolygenMesh* polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
			QString modelName = QString::fromStdString(polygenMesh->getModelName());
			if (QString::compare(selectedModelName, modelName) == 0)
			{
				if (modelListItem->checkState() == Qt::Checked)
				{
					for (int i = 0; i < polygenMesh->GetMeshList().GetCount(); i++)
						modelListItem->child(i)->setCheckState(Qt::Unchecked);
					//need notshow
					polygenMesh->bShow = true;
				}
				else
				{
					for (int i = 0; i < polygenMesh->GetMeshList().GetCount(); i++)
						modelListItem->child(i)->setCheckState(Qt::Unchecked);
					//need show
					polygenMesh->bShow = false;
				}
				ui->horizontalSlider_RangeFrom->setMaximum(polygenMesh->GetMeshList().GetCount());
				ui->horizontalSlider_RangeFrom->setMinimum(1);
				ui->horizontalSlider_RangeFrom->setValue(1);
				ui->horizontalSlider_RangeFrom->setSingleStep(1);
				ui->label_RangeFrom->display(1);
				ui->horizontalSlider_RangeTo->setMaximum(polygenMesh->GetMeshList().GetCount());
				ui->horizontalSlider_RangeTo->setMinimum(1);
				ui->horizontalSlider_RangeTo->setValue(polygenMesh->GetMeshList().GetCount());
				ui->horizontalSlider_RangeTo->setSingleStep(1);
				ui->label_RangeTo->display(((polygenMesh->GetMeshList().GetCount())));
			}

		};
		break;
	case 2:
		for (GLKPOSITION pos = polygenMeshList.GetHeadPosition(); pos != nullptr;) {
			PolygenMesh* polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
			for (GLKPOSITION posChild = polygenMesh->GetMeshList().GetHeadPosition(); posChild;)
			{
				QMeshPatch* qMeshPatch = (QMeshPatch*)polygenMesh->GetMeshList().GetNext(posChild);
				QString modelNameChild = QString::fromStdString(qMeshPatch->waypointPatchName);
				if (QString::compare(selectedModelName, modelNameChild) == 0)
				{
					//QStandardItem* selectedModel = treeModel->itemFromIndex(qMeshPatch->index.);
					
				}
			}
		}
		break;
	default:
		break;

	}
	
    
    pGLK->refresh(true);
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
/*This is built by Singa*/

void MainWindow::tianGcode2ABB()
{
	fiveAxisPoint* initialPoint = new fiveAxisPoint();
	bool Yup2Zup_switch = 1;
	double Xoff = 0;
	double Yoff = 0;
	double Zoff = 0;
	/*First step is natsort the filedir like 100->101->200
	  Then read them and display the Layer and Point in Mainview
	  
	  */
	initialPoint->natSort(ui->lineEdit_PosNorFileDir->text(), wayPointFileCell);
	initialPoint->readWayPointData(ui->lineEdit_PosNorFileDir->text(), Yup2Zup_switch,Xoff,Yoff,Zoff,&polygenMeshList,wayPointFileCell, pGLK);
	updateTree();
	viewAllWaypointLayers();
	updateTree();
	if (ui->checkBox_varyHeight->isChecked() == true) 
	{
		initialPoint->natSort((ui->lineEdit_OFFLayerFile->text()), sliceSetFileCell);
		if (wayPointFileCell.size() != sliceSetFileCell.size()) 
		{
			cout << "Error: Layers file num != Waypoints file num" << endl;
			return;
		}
		else 
		{
			initialPoint->readSliceData(ui->lineEdit_OFFLayerFile->text(), Yup2Zup_switch, Xoff, Yoff, Zoff ,&polygenMeshList, sliceSetFileCell,pGLK);
			
			updateTree();
			viewAllWaypointLayers();
		}
	}
	initialPoint->readExtruderHeadfile("extruderHead.obj", &polygenMeshList, pGLK);
	updateTree();
	
	initialPoint->readPlatformfile("printingPlatform.obj", &polygenMeshList, pGLK);
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
	initialPoint->getLayerHeight(Slices, Waypoints, PrintPlatform, varyThickness_switch, GcodeGeneRange_From, GcodeGeneRange_To, false, UpZHeight, Xmove, Ymove);
	initialPoint->singularityOpt(Waypoints, GcodeGeneRange_From, GcodeGeneRange_To);
	initialPoint->height2E(Waypoints, GcodeGeneRange_From, GcodeGeneRange_To, varyThickness_switch);
	//initialPoint->writeGcode(Waypoints, targetFileName, GcodeGeneRange_From, GcodeGeneRange_To, E3_Xoff, E3_Yoff);
	initialPoint->writeABBGcode(Waypoints, targetFileName, GcodeGeneRange_From, GcodeGeneRange_To, E3_Xoff, E3_Yoff);
	viewAllWaypointLayers();
	pGLK->refresh(true);
	delete initialPoint;
	
}

void MainWindow::selectDir()
{
	QString dirStr = QFileDialog::getExistingDirectory(this, tr("PosNorFileDir"), "../1_GcodeGeneModel");
	QByteArray dirnameArray = dirStr.toLatin1();
	char* dirname = dirnameArray.data();
	std::string strDirname(dirname);
	std::size_t foundStart = strDirname.find_last_of("/");
	std::size_t foundEnd = strDirname.find_last_of(".");
	std::string dirName;
	dirName = strDirname.substr(0, foundEnd);
	dirName = strDirname.substr(foundStart + 1);
	ui->lineEdit_PosNorFileDir->setText(QString::fromStdString((dirName)));

	dirStr = QFileDialog::getExistingDirectory(this, tr("OFFLayerFile)"));
	dirnameArray = dirStr.toLatin1();
	dirname = dirnameArray.data();
	string strDirname1(dirname);
	foundStart = strDirname1.find_last_of("/");
	foundEnd = strDirname1.find_last_of(".");
	dirName = strDirname1.substr(0, foundEnd);
	dirName = strDirname1.substr(foundStart + 1);
	ui->lineEdit_OFFLayerFile->setText(QString::fromStdString((dirName)));

}

void MainWindow::QMeshPatchRangeActive()
{
	QModelIndex index = ui->treeView->currentIndex();
	QStandardItem* selectedModel = treeModel->itemFromIndex(index);
	string selectedModelType = selectedModel->data().toString().toStdString();
	if (selectedModelType == "POLYGENMESH")
	{
		int rangeFromIndex = ui->label_RangeFrom->value() - 1;
		int rangeToIndex = ui->label_RangeTo->value() -1;
		
		for (GLKPOSITION pos = polygenMeshList.GetHeadPosition(); pos != nullptr;) {
			PolygenMesh* polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
			QString modelName = QString::fromStdString(polygenMesh->getModelName());
			QString selectedModelName = index.data(Qt::DisplayRole).toString();
			if (QString::compare(selectedModelName, modelName) == 0)
			{
				selectedModel->setCheckState(Qt::Unchecked);
				polygenMesh->bShow = false;
				int i = 0;
				for (GLKPOSITION posChild = polygenMesh->GetMeshList().GetHeadPosition(); posChild;)
				{
					QMeshPatch* qMeshPatch = (QMeshPatch*)polygenMesh->GetMeshList().GetNext(posChild);
					if (i<= rangeToIndex &&i>= rangeFromIndex)
					{
						selectedModel->child(i)->setCheckState(Qt::Checked);
						//need notshow
					}
					else
					{
						selectedModel->child(i)->setCheckState(Qt::Unchecked);
						//need notshow
					}
					i++;
				}
			}
		}
	}
	
}
void MainWindow::test_in_mainwindow()
{
	//QChart* chart = new QChart();

	//// 构建折线系列对象
	//QLineSeries* series = new QLineSeries();
	//for (quint32 i = 0; i < 100; i++)
	//{
	//	// 参数 x 为循环自增变量 i，参数 y 为正弦函数Y值
	//	series->append(i, sin(static_cast<double>(0.6f * i)));
	//}

	//// 将系列添加到图表
	//chart->addSeries(series);
	//// 基于已添加到图表的 series 来创建默认的坐标轴
	//chart->createDefaultAxes();

	//// 将图表绑定到视图
	//ui->widget->setChart(chart);




	//PolygenMesh* waypointSet = new PolygenMesh;
	//waypointSet->setModelName("Waypoints");
	//waypointSet->meshType = WAYPOINT;
	//waypointSet->BuildGLList(waypointSet->m_bVertexNormalShading);
	//polygenMeshList.AddTail(waypointSet);
	//pGLK->AddDisplayObj(waypointSet, true);
	//PolygenMesh* waypointSet1 = new PolygenMesh;
	//waypointSet1->setModelName("Waypoints1");
	//waypointSet1->meshType = WAYPOINT;
	//waypointSet1->BuildGLList(waypointSet1->m_bVertexNormalShading);
	//polygenMeshList.AddTail(waypointSet1);
	//pGLK->AddDisplayObj(waypointSet1, true);



	////treeModel->appendRow(itemProject);
	////delete waypointSet;
	//updateTree();

	/*PolygenMesh* waypointSet1 = new PolygenMesh;
	waypointSet1->setModelName("Waypoints1");
	waypointSet1->meshType = WAYPOINT;
	waypointSet1->BuildGLList(waypointSet1->m_bVertexNormalShading);
	polygenMeshList.AddTail(waypointSet1);
	pGLK->AddDisplayObj(waypointSet1, true);
	updateTree();

	PolygenMesh* waypointSet2 = new PolygenMesh;
	waypointSet2->setModelName("Waypoints2");
	waypointSet2->meshType = WAYPOINT;
	waypointSet2->BuildGLList(waypointSet2->m_bVertexNormalShading);
	polygenMeshList.AddTail(waypointSet2);
	pGLK->AddDisplayObj(waypointSet2, true);
	updateTree();*/

	//QStandardItem* itemProject = new QStandardItem(QStringLiteral("四川省"));
	//treeModel->appendRow(itemProject);
	//treeModel->setItem(0, 1, new QStandardItem(QStringLiteral("省份信息说明1"))); //父节点的兄弟节点
	//QStandardItem* itemChild = new QStandardItem(QStringLiteral("成都市")); //子节点
	//itemProject->appendRow(itemChild);
	//itemChild = new QStandardItem(QStringLiteral("绵阳市"));
	//itemProject->appendRow(itemChild);
	//itemProject->setChild(0, 1, new QStandardItem(QStringLiteral("成都信息")));

	//itemProject = new QStandardItem(QStringLiteral("广东省"));
	//treeModel->appendRow(itemProject);
	//treeModel->setItem(1, 1, new QStandardItem(QStringLiteral("省份信息说明2")));
	//itemChild = new QStandardItem(QStringLiteral("深圳市"));

	//QColor qc(Qt::red);
	//itemChild->setCheckable(true);//出现方框可勾选

	//itemProject->appendRow(itemChild);//子节点
	//itemProject->setChild(0, 1, new QStandardItem(QStringLiteral("深圳信息1")));
	//itemProject->setChild(0, 2, new QStandardItem(QStringLiteral("深圳信息2")));

	//itemChild->parent()->child(0, 2)->setForeground(QBrush(qc));//字体颜色
	//itemChild->parent()->child(0, 1)->setBackground(QBrush(qc));//背景颜色




}
