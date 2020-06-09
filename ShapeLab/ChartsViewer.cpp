//#include "stdafx.h"

#include "ui_ChartsViewer.h"
///*#include <QFileDialog>
//#include <QtDebug>
//#include <QDesktopWidget>
//#include <QCoreApplication>
//#include <QMimeData>
//#include <QTreeView>
//#include <QThread>
//#include <QTimer>
//#include <QDateTime>
//#include <QMessageBox>
//#include <QScreen>*/

////#include <fstream>
////#include <dirent.h>
#include "../ChartsViewer.h"


ChartsViewer::ChartsViewer( QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::ChartsViewer)
{
    ui->setupUi(this);
    createActions();
    
}



ChartsViewer::~ChartsViewer()
{
    delete ui;
}


void ChartsViewer::createActions()
{
    connect(ui->pushButton_chartActive, SIGNAL(released()), this, SLOT(drawchart()));
	connect(ui->comboBox_polygenMesh, SIGNAL(activated(const QString&)), this, SLOT(update()));
	connect(ui->comboBox_qmeshPatch, SIGNAL(activated(const QString&)), this, SLOT(update()));
	connect(ui->comboBox_workStep, SIGNAL(activated(const QString&)), this, SLOT(update()));
	connect(ui->comboBox_parameter, SIGNAL(activated(const QString&)), this, SLOT(update()));
	

}

void ChartsViewer::getPolygenMeshList(GLKObList* polygenMeshList)
{
    this->polygenMeshList = *polygenMeshList;
}

void ChartsViewer::drawchart()
{
	QString chartTitle = currentPolygenMesh + " " + currentQmeshPatch + " " + currentWorkStep + " " + currentParameter;
	QList<QGridLayout*> layouts = ui->widget->findChildren<QGridLayout*>();
	if (layouts.count() != 0)
	{
		QGridLayout* layout = layouts.first();
		delete layout;
	}
	QChart* chart = new QChart();
	for (GLKPOSITION pos = this->polygenMeshList.GetHeadPosition(); pos != nullptr;)
	{
		PolygenMesh* polygenMesh = (PolygenMesh*)this->polygenMeshList.GetNext(pos);
		if (polygenMesh->getModelName() == currentPolygenMesh.toStdString())
		{
			if (polygenMesh->getModelName() != "Waypoints")
			{
				cout << "ERROR DATA, WAYPOINTS ONLY" << endl;
				return;
			}
			int nodeCount = 0;
			double showValue;
			for (GLKPOSITION pos_QmeshPatch = polygenMesh->GetMeshList().GetHeadPosition(); pos_QmeshPatch != nullptr;)
			{
				QMeshPatch* qmeshPatch = (QMeshPatch*)polygenMesh->GetMeshList().GetNext(pos_QmeshPatch);
				if (ui->checkBox_patchRange->checkState()== (Qt::Checked))
				{
					QString chartTitle = currentPolygenMesh + " " + currentQmeshPatch + " " + ui->spinBox_rangeFrom->value() + "~" + ui->spinBox_rangeTo->value() + " " + currentParameter;
					if (qmeshPatch->GetIndexNo() >= ui->spinBox_rangeFrom->value() - 1 && qmeshPatch->GetIndexNo() <= ui->spinBox_rangeTo->value() - 1)
					{
						QScatterSeries* seriesMark = new QScatterSeries();
						
						QLineSeries* series = new QLineSeries();
						for (GLKPOSITION pos_QmeshNode = qmeshPatch->GetNodeList().GetHeadPosition(); pos_QmeshNode != nullptr;)
						{
							QMeshNode* qmeshNode = (QMeshNode*)qmeshPatch->GetNodeList().GetNext(pos_QmeshNode);
							//orginalData
							if (ui->comboBox_workStep->currentText() == "InitailPor" && ui->comboBox_parameter->currentText() == "X") showValue = qmeshNode->m_orginalPostion[0];
							else if (ui->comboBox_workStep->currentText() == "InitailPor" && ui->comboBox_parameter->currentText() == "Y") showValue = qmeshNode->m_orginalPostion[1];
							else if (ui->comboBox_workStep->currentText() == "InitailPor" && ui->comboBox_parameter->currentText() == "Z") showValue = qmeshNode->m_orginalPostion[2];
							else if (ui->comboBox_workStep->currentText() == "InitailPor" && ui->comboBox_parameter->currentText() == "nX") showValue = qmeshNode->m_orginalNormal[0];
							else if (ui->comboBox_workStep->currentText() == "InitailPor" && ui->comboBox_parameter->currentText() == "nY") showValue = qmeshNode->m_orginalNormal[1];
							else if (ui->comboBox_workStep->currentText() == "InitailPor" && ui->comboBox_parameter->currentText() == "nZ") showValue = qmeshNode->m_orginalNormal[2];
							//after fliping and YZturn
							else if (ui->comboBox_workStep->currentText() == "CNC_Data" && ui->comboBox_parameter->currentText() == "X") showValue = qmeshNode->m_YZ_rotatePostion[0];
							else if (ui->comboBox_workStep->currentText() == "CNC_Data" && ui->comboBox_parameter->currentText() == "Y") showValue = qmeshNode->m_YZ_rotatePostion[1];
							else if (ui->comboBox_workStep->currentText() == "CNC_Data" && ui->comboBox_parameter->currentText() == "Z") showValue = qmeshNode->m_YZ_rotatePostion[2];
							else if (ui->comboBox_workStep->currentText() == "CNC_Data" && ui->comboBox_parameter->currentText() == "nX") showValue = qmeshNode->m_YZ_rotateNormal[0];
							else if (ui->comboBox_workStep->currentText() == "CNC_Data" && ui->comboBox_parameter->currentText() == "nY") showValue = qmeshNode->m_YZ_rotateNormal[1];
							else if (ui->comboBox_workStep->currentText() == "CNC_Data" && ui->comboBox_parameter->currentText() == "nZ") showValue = qmeshNode->m_YZ_rotateNormal[2];
							//after moving to new position
							else if (ui->comboBox_workStep->currentText() == "PrintData" && ui->comboBox_parameter->currentText() == "X") showValue = qmeshNode->m_printPostion[0];
							else if (ui->comboBox_workStep->currentText() == "PrintData" && ui->comboBox_parameter->currentText() == "Y") showValue = qmeshNode->m_printPostion[1];
							else if (ui->comboBox_workStep->currentText() == "PrintData" && ui->comboBox_parameter->currentText() == "Z") showValue = qmeshNode->m_printPostion[2];
							else if (ui->comboBox_workStep->currentText() == "PrintData" && ui->comboBox_parameter->currentText() == "nX") showValue = qmeshNode->m_printNormal[0];
							else if (ui->comboBox_workStep->currentText() == "PrintData" && ui->comboBox_parameter->currentText() == "nY") showValue = qmeshNode->m_printNormal[1];
							else if (ui->comboBox_workStep->currentText() == "PrintData" && ui->comboBox_parameter->currentText() == "nZ") showValue = qmeshNode->m_printNormal[2];
							//ater singularity solve in CNC and Extrusion calculation
							else if (ui->comboBox_workStep->currentText() == "IK_CNC_Data" && ui->comboBox_parameter->currentText() == "X") showValue = qmeshNode->m_XYZBCE[0];
							else if (ui->comboBox_workStep->currentText() == "IK_CNC_Data" && ui->comboBox_parameter->currentText() == "Y") showValue = qmeshNode->m_XYZBCE[1];
							else if (ui->comboBox_workStep->currentText() == "IK_CNC_Data" && ui->comboBox_parameter->currentText() == "Z") showValue = qmeshNode->m_XYZBCE[2];
							else if (ui->comboBox_workStep->currentText() == "IK_CNC_Data" && ui->comboBox_parameter->currentText() == "B") showValue = qmeshNode->m_XYZBCE[3];
							else if (ui->comboBox_workStep->currentText() == "IK_CNC_Data" && ui->comboBox_parameter->currentText() == "C") showValue = qmeshNode->m_XYZBCE[4];
							else if (ui->comboBox_workStep->currentText() == "IK_CNC_Data" && ui->comboBox_parameter->currentText() == "E") showValue = qmeshNode->m_XYZBCE[5];
							else if (ui->comboBox_workStep->currentText() == "IK_CNC_Data" && ui->comboBox_parameter->currentText() == "Solver") showValue = qmeshNode->m_XYZBCE[6];
							//after ABB ik calculation
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "X") showValue = qmeshNode->m_ABB_XYZBCEF[0];
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "Y") showValue = qmeshNode->m_ABB_XYZBCEF[1];
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "Z") showValue = qmeshNode->m_ABB_XYZBCEF[2];
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "B") showValue = qmeshNode->m_ABB_XYZBCEF[3];
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "C") showValue = qmeshNode->m_ABB_XYZBCEF[4];
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "E") showValue = qmeshNode->m_ABB_XYZBCEF[5];
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "F") showValue = qmeshNode->m_ABB_XYZBCEF[6];
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "V_end-effect") showValue = qmeshNode->m_ABB_Velocty[0];
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "V_B") showValue = qmeshNode->m_ABB_Velocty[1];
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "V_C") showValue = qmeshNode->m_ABB_Velocty[2];
							series->append(nodeCount, showValue);
							seriesMark->append(nodeCount++, showValue);
						}
						connect(series, &QScatterSeries::hovered, this, &ChartsViewer::slotPointHoverd);
						seriesMark->setMarkerShape(QScatterSeries::MarkerShapeCircle);//圆形的点
						seriesMark->setMarkerSize(1);
						seriesMark->setBorderColor(QColor(0, 0, 0));
						seriesMark->setColor(QColor(0, 0, 0));
						seriesMark->setName(QString::fromStdString("OTHERS"));
						series->setName(QString::fromStdString(qmeshPatch->waypointPatchName));
						chart->addSeries(series);
						chart->addSeries(seriesMark);
						if (qmeshPatch->GetIndexNo() == ui->spinBox_rangeTo->value() - 1)
							break;
					}
				}
				else
				{
					if (qmeshPatch->GetIndexNo() == ui->comboBox_qmeshPatch->currentIndex())
					{
						QScatterSeries* seriesMark = new QScatterSeries();
						QLineSeries* series = new QLineSeries();
						for (GLKPOSITION pos_QmeshNode = qmeshPatch->GetNodeList().GetHeadPosition(); pos_QmeshNode != nullptr;)
						{
							QMeshNode* qmeshNode = (QMeshNode*)qmeshPatch->GetNodeList().GetNext(pos_QmeshNode);
							//orginalData
							if (ui->comboBox_workStep->currentText() == "InitailPor" && ui->comboBox_parameter->currentText() == "X") showValue = qmeshNode->m_orginalPostion[0];
							else if (ui->comboBox_workStep->currentText() == "InitailPor" && ui->comboBox_parameter->currentText() == "Y") showValue = qmeshNode->m_orginalPostion[1];
							else if (ui->comboBox_workStep->currentText() == "InitailPor" && ui->comboBox_parameter->currentText() == "Z") showValue = qmeshNode->m_orginalPostion[2];
							else if (ui->comboBox_workStep->currentText() == "InitailPor" && ui->comboBox_parameter->currentText() == "nX") showValue = qmeshNode->m_orginalNormal[0];
							else if (ui->comboBox_workStep->currentText() == "InitailPor" && ui->comboBox_parameter->currentText() == "nY") showValue = qmeshNode->m_orginalNormal[1];
							else if (ui->comboBox_workStep->currentText() == "InitailPor" && ui->comboBox_parameter->currentText() == "nZ") showValue = qmeshNode->m_orginalNormal[2];
							//after fliping and YZturn
							else if (ui->comboBox_workStep->currentText() == "CNC_Data" && ui->comboBox_parameter->currentText() == "X") showValue = qmeshNode->m_YZ_rotatePostion[0];
							else if (ui->comboBox_workStep->currentText() == "CNC_Data" && ui->comboBox_parameter->currentText() == "Y") showValue = qmeshNode->m_YZ_rotatePostion[1];
							else if (ui->comboBox_workStep->currentText() == "CNC_Data" && ui->comboBox_parameter->currentText() == "Z") showValue = qmeshNode->m_YZ_rotatePostion[2];
							else if (ui->comboBox_workStep->currentText() == "CNC_Data" && ui->comboBox_parameter->currentText() == "nX") showValue = qmeshNode->m_YZ_rotateNormal[0];
							else if (ui->comboBox_workStep->currentText() == "CNC_Data" && ui->comboBox_parameter->currentText() == "nY") showValue = qmeshNode->m_YZ_rotateNormal[1];
							else if (ui->comboBox_workStep->currentText() == "CNC_Data" && ui->comboBox_parameter->currentText() == "nZ") showValue = qmeshNode->m_YZ_rotateNormal[2];
							//after moving to new position
							else if (ui->comboBox_workStep->currentText() == "PrintData" && ui->comboBox_parameter->currentText() == "X") showValue = qmeshNode->m_printPostion[0];
							else if (ui->comboBox_workStep->currentText() == "PrintData" && ui->comboBox_parameter->currentText() == "Y") showValue = qmeshNode->m_printPostion[1];
							else if (ui->comboBox_workStep->currentText() == "PrintData" && ui->comboBox_parameter->currentText() == "Z") showValue = qmeshNode->m_printPostion[2];
							else if (ui->comboBox_workStep->currentText() == "PrintData" && ui->comboBox_parameter->currentText() == "nX") showValue = qmeshNode->m_printNormal[0];
							else if (ui->comboBox_workStep->currentText() == "PrintData" && ui->comboBox_parameter->currentText() == "nY") showValue = qmeshNode->m_printNormal[1];
							else if (ui->comboBox_workStep->currentText() == "PrintData" && ui->comboBox_parameter->currentText() == "nZ") showValue = qmeshNode->m_printNormal[2];
							//ater singularity solve in CNC and Extrusion calculation
							else if (ui->comboBox_workStep->currentText() == "IK_CNC_Data" && ui->comboBox_parameter->currentText() == "X") showValue = qmeshNode->m_XYZBCE[0];
							else if (ui->comboBox_workStep->currentText() == "IK_CNC_Data" && ui->comboBox_parameter->currentText() == "Y") showValue = qmeshNode->m_XYZBCE[1];
							else if (ui->comboBox_workStep->currentText() == "IK_CNC_Data" && ui->comboBox_parameter->currentText() == "Z") showValue = qmeshNode->m_XYZBCE[2];
							else if (ui->comboBox_workStep->currentText() == "IK_CNC_Data" && ui->comboBox_parameter->currentText() == "B") showValue = qmeshNode->m_XYZBCE[3];
							else if (ui->comboBox_workStep->currentText() == "IK_CNC_Data" && ui->comboBox_parameter->currentText() == "C") showValue = qmeshNode->m_XYZBCE[4];
							else if (ui->comboBox_workStep->currentText() == "IK_CNC_Data" && ui->comboBox_parameter->currentText() == "E") showValue = qmeshNode->m_XYZBCE[5];
							else if (ui->comboBox_workStep->currentText() == "IK_CNC_Data" && ui->comboBox_parameter->currentText() == "Solver") showValue = qmeshNode->m_XYZBCE[6];
							//after ABB ik calculation
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "X") showValue = qmeshNode->m_ABB_XYZBCEF[0];
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "Y") showValue = qmeshNode->m_ABB_XYZBCEF[1];
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "Z") showValue = qmeshNode->m_ABB_XYZBCEF[2];
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "B") showValue = qmeshNode->m_ABB_XYZBCEF[3];
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "C") showValue = qmeshNode->m_ABB_XYZBCEF[4];
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "E") showValue = qmeshNode->m_ABB_XYZBCEF[5];
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "F") showValue = qmeshNode->m_ABB_XYZBCEF[6];
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "V_end-effect") showValue = qmeshNode->m_ABB_Velocty[0];
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "V_B") showValue = qmeshNode->m_ABB_Velocty[1];
							else if (ui->comboBox_workStep->currentText() == "IK_ABB_Data" && ui->comboBox_parameter->currentText() == "V_C") showValue = qmeshNode->m_ABB_Velocty[2];
							series->append(nodeCount, showValue);
							seriesMark->append(nodeCount++, showValue);
						}
						connect(series, &QScatterSeries::hovered, this, &ChartsViewer::slotPointHoverd);
						seriesMark->setMarkerShape(QScatterSeries::MarkerShapeCircle);//圆形的点
						seriesMark->setMarkerSize(1);
						seriesMark->setBorderColor(QColor(0, 0, 0));
						seriesMark->setColor(QColor(0, 0, 0));
						seriesMark->setName(QString::fromStdString("OTHERS"));
						series->setName(QString::fromStdString(qmeshPatch->waypointPatchName));
						chart->addSeries(series);
						chart->addSeries(seriesMark);
						break;
					}
				}
			}
			chart->setTitle(chartTitle);
			chart->createDefaultAxes();
			chart->axisX()->setTitleText("nodeCount");
			chart->axisY()->setTitleText(ui->comboBox_parameter->currentText());
			QList<QLegendMarker*> legendMarkers = chart->legend()->markers();
			cout << legendMarkers.count() << endl;
			for (int i = 0; i < legendMarkers.count(); i++)
			{
				QLegendMarker* legendMarker = legendMarkers.at(i);
				if (legendMarker->label().toStdString().find("OTHERS") == string::npos)
					legendMarkers.at(i)->setVisible(true);
				else legendMarkers.at(i)->setVisible(false);
			}
			QChartView* chartView = new QChartView(chart);
			chartView->setRenderHint(QPainter::Antialiasing);
			chartView->setMouseTracking(true);
			chartView->setInteractive(true);
			chartView->setRubberBand(QChartView::RectangleRubberBand);
			QGridLayout* baseLayout = new QGridLayout();
			baseLayout->addWidget(chartView, 1, 0);
			ui->widget->setLayout(baseLayout);
			break;
		}
	}
}


void ChartsViewer::slotPointHoverd(const QPointF& point, bool state)
{
	if (state) {

		ui->m_valueLabel->setText(QString::asprintf("(%3.0f,%3.3f)", point.x(),point.y()));
		QPoint curPos = mapFromGlobal(QCursor::pos());
		ui->m_valueLabel->move(curPos.x() - ui->m_valueLabel->width() / 2, curPos.y() - ui->m_valueLabel->height() * 1.5);//移动数值
		ui->m_valueLabel->hide();
		ui->m_valueLabel->setWindowFlags(Qt::WindowStaysOnTopHint);
		ui->m_valueLabel->show();//显示出来
	}
	else
		ui->m_valueLabel->hide();//进行隐藏
}


void ChartsViewer::update()
{
	if (ui->comboBox_polygenMesh->count() == 0)
	{
		for (GLKPOSITION pos = this->polygenMeshList.GetHeadPosition(); pos != nullptr;)
		{
			PolygenMesh* polygenMesh = (PolygenMesh*)this->polygenMeshList.GetNext(pos);
			ui->comboBox_polygenMesh->addItem(QString::fromStdString(polygenMesh->getModelName()));
		}
		currentPolygenMesh = ui->comboBox_polygenMesh->currentText();
		for (GLKPOSITION pos = this->polygenMeshList.GetHeadPosition(); pos != nullptr;)
		{
			PolygenMesh* polygenMesh = (PolygenMesh*)this->polygenMeshList.GetNext(pos);
			if (polygenMesh->getModelName() == currentPolygenMesh.toStdString())
			{
				ui->spinBox_rangeFrom->setMaximum(polygenMesh->GetMeshList().GetCount());
				ui->spinBox_rangeTo->setMaximum(polygenMesh->GetMeshList().GetCount());
				ui->spinBox_rangeFrom->setValue(1);
				ui->spinBox_rangeTo->setValue(polygenMesh->GetMeshList().GetCount());
				for (GLKPOSITION pos_QmeshPatch = polygenMesh->GetMeshList().GetHeadPosition(); pos_QmeshPatch != nullptr;)
				{
					QMeshPatch* qmeshPatch = (QMeshPatch*)polygenMesh->GetMeshList().GetNext(pos_QmeshPatch);
					if (polygenMesh->getModelName() == "Waypoints")
						ui->comboBox_qmeshPatch->addItem(QString::fromStdString(qmeshPatch->waypointPatchName));
					else
						ui->comboBox_qmeshPatch->addItem(QString::fromStdString("undefined"));
				}
				break;
			}
		}
		ui->comboBox_workStep->addItem(QString::fromStdString("InitailPor"));
		ui->comboBox_workStep->addItem(QString::fromStdString("CNC_Data"));
		ui->comboBox_workStep->addItem(QString::fromStdString("PrintData"));
		ui->comboBox_workStep->addItem(QString::fromStdString("IK_CNC_Data"));
		ui->comboBox_workStep->addItem(QString::fromStdString("IK_ABB_Data"));
		ui->comboBox_parameter->addItem(QString::fromStdString("X"));
		ui->comboBox_parameter->addItem(QString::fromStdString("Y"));
		ui->comboBox_parameter->addItem(QString::fromStdString("Z"));
		ui->comboBox_parameter->addItem(QString::fromStdString("nX"));
		ui->comboBox_parameter->addItem(QString::fromStdString("nY"));
		ui->comboBox_parameter->addItem(QString::fromStdString("nZ"));
	}
	else
	{
		for (GLKPOSITION pos = this->polygenMeshList.GetHeadPosition(); pos != nullptr;)
		{
			PolygenMesh* polygenMesh = (PolygenMesh*)this->polygenMeshList.GetNext(pos);
			if (ui->comboBox_polygenMesh->currentText() != currentPolygenMesh)
			{
				ui->comboBox_qmeshPatch->clear();
				if (polygenMesh->getModelName() == ui->comboBox_polygenMesh->currentText().toStdString())
				{
					ui->spinBox_rangeFrom->setMaximum(polygenMesh->GetMeshList().GetCount());
					ui->spinBox_rangeTo->setMaximum(polygenMesh->GetMeshList().GetCount());
					ui->spinBox_rangeFrom->setValue(1);
					ui->spinBox_rangeTo->setValue(polygenMesh->GetMeshList().GetCount());
					for (GLKPOSITION pos_QmeshPatch = polygenMesh->GetMeshList().GetHeadPosition(); pos_QmeshPatch != nullptr;)
					{
						QMeshPatch* qmeshPatch = (QMeshPatch*)polygenMesh->GetMeshList().GetNext(pos_QmeshPatch);
						if (polygenMesh->getModelName() == "Waypoints")
							ui->comboBox_qmeshPatch->addItem(QString::fromStdString(qmeshPatch->waypointPatchName));
						else
							ui->comboBox_qmeshPatch->addItem(QString::fromStdString("undefined"));
					}
					break;
				}
			}
			if (ui->comboBox_workStep->currentText() != currentWorkStep)
			{
				ui->comboBox_parameter->clear();
				if (ui->comboBox_workStep->currentText().toStdString() == "InitailPor")
				{
					ui->comboBox_parameter->addItem(QString::fromStdString("X"));
					ui->comboBox_parameter->addItem(QString::fromStdString("Y"));
					ui->comboBox_parameter->addItem(QString::fromStdString("Z"));
					ui->comboBox_parameter->addItem(QString::fromStdString("nX"));
					ui->comboBox_parameter->addItem(QString::fromStdString("nY"));
					ui->comboBox_parameter->addItem(QString::fromStdString("nZ"));
				}
				else if (ui->comboBox_workStep->currentText().toStdString() == "CNC_Data")
				{
					ui->comboBox_parameter->addItem(QString::fromStdString("X"));
					ui->comboBox_parameter->addItem(QString::fromStdString("Y"));
					ui->comboBox_parameter->addItem(QString::fromStdString("Z"));
					ui->comboBox_parameter->addItem(QString::fromStdString("nX"));
					ui->comboBox_parameter->addItem(QString::fromStdString("nY"));
					ui->comboBox_parameter->addItem(QString::fromStdString("nZ"));
				}
				else if (ui->comboBox_workStep->currentText().toStdString() == "PrintData")
				{
					ui->comboBox_parameter->addItem(QString::fromStdString("X"));
					ui->comboBox_parameter->addItem(QString::fromStdString("Y"));
					ui->comboBox_parameter->addItem(QString::fromStdString("Z"));
					ui->comboBox_parameter->addItem(QString::fromStdString("nX"));
					ui->comboBox_parameter->addItem(QString::fromStdString("nY"));
					ui->comboBox_parameter->addItem(QString::fromStdString("nZ"));
				}
				else if (ui->comboBox_workStep->currentText().toStdString() == "IK_CNC_Data")
				{
					ui->comboBox_parameter->addItem(QString::fromStdString("X"));
					ui->comboBox_parameter->addItem(QString::fromStdString("Y"));
					ui->comboBox_parameter->addItem(QString::fromStdString("Z"));
					ui->comboBox_parameter->addItem(QString::fromStdString("B"));
					ui->comboBox_parameter->addItem(QString::fromStdString("C"));
					ui->comboBox_parameter->addItem(QString::fromStdString("E"));
					ui->comboBox_parameter->addItem(QString::fromStdString("Solver"));
				}
				else if (ui->comboBox_workStep->currentText().toStdString() == "IK_ABB_Data")
				{
					ui->comboBox_parameter->addItem(QString::fromStdString("X"));
					ui->comboBox_parameter->addItem(QString::fromStdString("Y"));
					ui->comboBox_parameter->addItem(QString::fromStdString("Z"));
					ui->comboBox_parameter->addItem(QString::fromStdString("B"));
					ui->comboBox_parameter->addItem(QString::fromStdString("C"));
					ui->comboBox_parameter->addItem(QString::fromStdString("E"));
					ui->comboBox_parameter->addItem(QString::fromStdString("F"));
					ui->comboBox_parameter->addItem(QString::fromStdString("V_end-effect"));
					ui->comboBox_parameter->addItem(QString::fromStdString("V_B"));
					ui->comboBox_parameter->addItem(QString::fromStdString("V_C"));
				}
			}
		}
	}
	currentPolygenMesh = ui->comboBox_polygenMesh->currentText();
	currentQmeshPatch = ui->comboBox_qmeshPatch->currentText();
	currentWorkStep = ui->comboBox_workStep->currentText();
	currentParameter = ui->comboBox_parameter->currentText();
}

	
	