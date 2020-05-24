#include "FiveAxisPoint.h"
#include "alphanum.hpp"
#include <MainWindow.h>


void fiveAxisPoint::natSort(QString dirctory, vector<string>& fileNameCell)
{
	if (fileNameCell.empty() == false) return;

	DIR* dp;
	struct dirent* ep;
	string fullDir = "../1_GcodeGeneModel/" + dirctory.toStdString;
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

void fiveAxisPoint::readWayPointData(QString packName, GLKObList polygenMeshList, vector<string> wayPointFileCell, GLKLib* pGLK)
{
	
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

		sprintf(filename, "%s%s%s%s", "../1_GcodeGeneModel/", packName.toStdString().c_str(), "/", wayPointFileCell[i].data());
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
	std::cout << "------------------------------------------- WayPoints Load OK!" << std::endl;
}

void fiveAxisPoint::readSliceData(QString sliceSetName, GLKObList polygenMeshList, vector<string> sliceSetFileCell, GLKLib* pGLK)
{

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
		sprintf(filename, "%s%s%s%s", "../1_GcodeGeneModel/", sliceSetName.toStdString().c_str(), "/", sliceSetFileCell[i].data());
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
	cout << "------------------------------------------- Slices Load OK!" << endl;
}

void fiveAxisPoint::readExtruderHeadfile(string extruderHeadName, GLKObList polygenMeshList, GLKLib* pGLK)
{
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

	cout << "------------------------------------------- Extruder Head Load OK!" << endl;
}

void fiveAxisPoint::readPlatformfile(string platformName, GLKObList polygenMeshList, GLKLib* pGLK)
{
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

	cout << "------------------------------------------- Platform Load OK!" << endl;
}

void fiveAxisPoint::getLayerHeight(PolygenMesh* polygenMesh_Slices, PolygenMesh* polygenMesh_Waypoints, bool varyThickness_switch) {

	if (varyThickness_switch == false) return;

	// write the layerHeight for the 1st layer
	QMeshPatch* WayPointPatch_1st = (QMeshPatch*)polygenMesh_Waypoints->GetMeshList().GetHead();

	for (GLKPOSITION Pos = WayPointPatch_1st->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* Node = (QMeshNode*)WayPointPatch_1st->GetNodeList().GetNext(Pos);

		Node->m_layerHeight = 0.500000;
	}

	// topLayer --> layer on the highest place [travel head to tail]
	for (GLKPOSITION Pos = polygenMesh_Slices->GetMeshList().GetHeadPosition(); Pos;) {
		QMeshPatch* topLayer = (QMeshPatch*)polygenMesh_Slices->GetMeshList().GetNext(Pos); // order: get data -> pnt move
		// without bottom layer so quit
		if (topLayer == (QMeshPatch*)polygenMesh_Slices->GetMeshList().GetHead()) { continue; }

		vector<QMeshPatch*> bottomLayers;
		int layerNum = 10;
		// detect the number of bottom layer

		if (topLayer->GetIndexNo() < layerNum) // GetIndexNo = Index velue [0 ... n]
		{
			GLKPOSITION Pos = polygenMesh_Slices->GetMeshList().Find(topLayer)->prev;
			bottomLayers.resize(topLayer->GetIndexNo()); // speed up vector load by allocate space before using;
			for (int i = 0; i < topLayer->GetIndexNo(); i++) {
				bottomLayers[i] = (QMeshPatch*)polygenMesh_Slices->GetMeshList().GetPrev(Pos);
			}
		}
		else {
			bottomLayers.resize(layerNum);
			GLKPOSITION Pos = polygenMesh_Slices->GetMeshList().Find(topLayer)->prev;
			for (int i = 0; i < layerNum; i++) {
				bottomLayers[i] = (QMeshPatch*)polygenMesh_Slices->GetMeshList().GetPrev(Pos);
			}
		}

		//--build PQP model
		vector<PQP_Model*> bLayerPQP;
		bLayerPQP.resize(bottomLayers.size());
		for (int i = 0; i < bottomLayers.size(); i++) {
			if (bottomLayers[i]->GetNodeNumber() < 3) continue;
			// build PQP model for bottom layers
			PQP_Model* pqpModel = new PQP_Model();
			pqpModel->BeginModel();  int index = 0;
			PQP_REAL p1[3], p2[3], p3[3];

			for (GLKPOSITION Pos = bottomLayers[i]->GetFaceList().GetHeadPosition(); Pos;) {
				QMeshFace* Face = (QMeshFace*)bottomLayers[i]->GetFaceList().GetNext(Pos);

				Face->GetNodeRecordPtr(0)->GetCoord3D(p1[0], p1[1], p1[2]);
				Face->GetNodeRecordPtr(1)->GetCoord3D(p2[0], p2[1], p2[2]);
				Face->GetNodeRecordPtr(2)->GetCoord3D(p3[0], p3[1], p3[2]);

				pqpModel->AddTri(p1, p2, p3, index);
				index++;

			}
			pqpModel->EndModel();
			bLayerPQP[i] = pqpModel;
		}

		double pp[3];

		int layerIndex = topLayer->GetIndexNo();

		GLKPOSITION WayPointPatch_Pos = polygenMesh_Waypoints->GetMeshList().FindIndex(layerIndex);
		QMeshPatch* WayPointPatch = (QMeshPatch*)polygenMesh_Waypoints->GetMeshList().GetAt(WayPointPatch_Pos);

		for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
			QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

			pp[0] = Node->m_orginalPostion[0];
			pp[1] = Node->m_orginalPostion[1];
			pp[2] = Node->m_orginalPostion[2];

			double minDist = 99999.99;
			for (int i = 0; i < bottomLayers.size(); i++) {
				if (bottomLayers[i]->GetNodeNumber() < 3) continue;

				PQP_DistanceResult dres; dres.last_tri = bLayerPQP[i]->last_tri;
				PQP_REAL p[3];
				p[0] = pp[0]; p[1] = pp[1]; p[2] = pp[2];
				PQP_Distance(&dres, bLayerPQP[i], p, 0.0, 0.0);

				double Dist = dres.Distance(); // distance of this layer
				if (minDist > Dist) minDist = Dist;
			}

			//cout << minDist << endl;
			Node->m_layerHeight = minDist;
		}

		// Get the final result; Temporary processing
		for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
			QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

			double Dist_value = Node->m_layerHeight;
			if (Dist_value > 0.8) {

				if (Node->GetIndexNo() == 0)
					Dist_value = 0.5;
				else {
					GLKPOSITION prevPos = WayPointPatch->GetNodeList().Find(Node)->prev;
					QMeshNode* prevNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(prevPos);
					Dist_value = prevNode->m_layerHeight;
				}
			}
			Node->m_layerHeight = Dist_value;
		}

		//	free memory
		for (int i = 0; i < bottomLayers.size(); i++) { delete bLayerPQP[i]; }

		/*****  Test layerHeight func ******/
		testLayerHeight(WayPointPatch, "TestLayerHeight", true);
		/**********************/

		cout << ".";
		if (((topLayer->GetIndexNo()) % 100 == 0) ||
			((topLayer->GetIndexNo() + 1) == polygenMesh_Waypoints->GetMeshList().GetCount()))
			cout << endl;
	}
	cout << "------------------------------------------- Layer Height Calculate OK!" << endl;
}

void fiveAxisPoint::getUpZwayPnts(PolygenMesh* polygenMesh_Waypoints) {

	for (GLKPOSITION Pos = polygenMesh_Waypoints->GetMeshList().GetHeadPosition(); Pos;) {
		QMeshPatch* WayPointPatch = (QMeshPatch*)polygenMesh_Waypoints->GetMeshList().GetNext(Pos);

		for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
			QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

			double Upz_Px, Upz_Py, Upz_Pz, Upz_Nx, Upz_Ny, Upz_Nz;
			Upz_Px = Node->m_orginalPostion[0];
			Upz_Py = Node->m_orginalPostion[2];
			Upz_Pz = Node->m_orginalPostion[1];
			Upz_Nx = Node->m_orginalNormal[0];
			Upz_Ny = Node->m_orginalNormal[2];
			Upz_Nz = Node->m_orginalNormal[1];

			Node->m_printPostion[0] = Upz_Px;
			Node->m_printPostion[1] = Upz_Py;
			Node->m_printPostion[2] = Upz_Pz + 15.0;
			Node->m_printNormal[0] = Upz_Nx;
			Node->m_printNormal[1] = Upz_Ny;
			Node->m_printNormal[2] = Upz_Nz;

			Node->SetCoord3D(Upz_Px, Upz_Py, Upz_Pz);
			//Node->SetNormal(Upz_Nx, Upz_Ny, Upz_Nz);

			//cout << Upz_Px << " " << Upz_Py << " " << Upz_Pz << " " << Upz_Nx << " " << Upz_Ny << " " << Upz_Nz << endl;
		}
		//int layerNum = WayPointPatch->GetIndexNo();
		//cout << "-------------- layerNum: " << layerNum << endl;

	}

	//int layerNum = wayPntsCell.size();
	//for (int i = 0; i < layerNum; i++) {
	//	int lines = wayPntsCell[i].rows();
	//	for (int ii = 0; ii < lines; ii++) {

	//		//double tempPx = wayPntsCell[i](ii, 0);
	//		double tempPy = wayPntsCell[i](ii, 2);
	//		double tempPz = wayPntsCell[i](ii, 1) - 4;
	//		//double tempNx = wayPntsCell[i](ii, 3);
	//		double tempNy = wayPntsCell[i](ii, 5);
	//		double tempNz = wayPntsCell[i](ii, 4);

	//		wayPntsCell[i](ii, 1) = tempPy;
	//		wayPntsCell[i](ii, 2) = tempPz;
	//		wayPntsCell[i](ii, 4) = tempNy;
	//		wayPntsCell[i](ii, 5) = tempNz;
	//	}
	//}
}

void fiveAxisPoint::singularityOpt(PolygenMesh* polygenMesh_Waypoints) {

	//for (int i = srtLayer - 1; i < endLayer; i++) {
	for (GLKPOSITION Pos = polygenMesh_Waypoints->GetMeshList().GetHeadPosition(); Pos;) {
		QMeshPatch* WayPointPatch = (QMeshPatch*)polygenMesh_Waypoints->GetMeshList().GetNext(Pos);

		//MatrixXf layerPntNor = wayPntsCell[i];
		//MatrixXf layerXYZBCE = MatrixXf::Zero(layerPntNor.rows(), 6);
		//MatrixXf layerJumpflag = MatrixXf::Zero(layerPntNor.rows(), 1);


		//isLargeLength(layerPntNor, layerXYZBCE, layerJumpflag);
		isLargeLength(WayPointPatch);
		//jumpflagCell[i] = layerJumpflag;
		normalSmooth(WayPointPatch, 10, true);

		double lambda = 7.3;
		getRawCdata(WayPointPatch, lambda);
		MatrixXf sectionTable;
		getDangerSec(WayPointPatch, sectionTable);
		getNewPntNor(WayPointPatch, sectionTable, lambda);
		normalSmooth(WayPointPatch, 2, true);

		getXYZBCE(WayPointPatch, 0);
		optimizationC(WayPointPatch);

		/*****  Test func ******/
		testXYZBCE(WayPointPatch, "TestXYZBCE", true);
		/**********************/

		cout << ".";
		if (((WayPointPatch->GetIndexNo() + 1) % 100 == 0) ||
			((WayPointPatch->GetIndexNo() + 1) == polygenMesh_Waypoints->GetMeshList().GetCount())) {
			cout << endl;
		}
	}
	cout << "------------------------------------------- XYZBCE Calculate OK! " << endl;
}

void fiveAxisPoint::isLargeLength(QMeshPatch* WayPointPatch) {

	double largeLength = 4.0;

	//int lines = layerPntNor.rows();
	//for (int i = 0; i < lines; i++) {
	for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

		double Etmp = 0;
		if (Node->GetIndexNo() == 0) {
			Etmp = 1;
		}
		else {
			double Px = Node->m_printPostion[0];
			double Py = Node->m_printPostion[1];
			double Pz = Node->m_printPostion[2];

			GLKPOSITION prevPos = WayPointPatch->GetNodeList().Find(Node)->prev;
			QMeshNode* prevNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(prevPos);
			double Px_prev = prevNode->m_printPostion[0];
			double Py_prev = prevNode->m_printPostion[1];
			double Pz_prev = prevNode->m_printPostion[2];

			// calculate the length of two pnts 
			Etmp = (Px - Px_prev) * (Px - Px_prev) + (Py - Py_prev) * (Py - Py_prev) + (Pz - Pz_prev) * (Pz - Pz_prev);
			Etmp = sqrt(Etmp);

			if (Etmp > largeLength) {
				Etmp = 0;
				Node->m_largeJumpFlag = -1;// start of next section
				prevNode->m_largeJumpFlag = 1;// end of pre section

				//guoxin
				// std::cout << "Large jump happen once!" << std::endl;
			}
		}

		Node->m_XYZBCE[5] = Etmp;
	}
}

void fiveAxisPoint::normalSmooth(QMeshPatch* WayPointPatch, int loop, bool smoothSwitch) {
	if (smoothSwitch == true) {
		double alpha = 0.5;
		int lines = WayPointPatch->GetNodeNumber();//->GetNodeList().GetCount()
		for (int smoothLoop = 0; smoothLoop < loop; smoothLoop++) {

			for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
				QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

				double Nx_prev, Ny_prev, Nz_prev;
				double Nx, Ny, Nz;
				double Nx_next, Ny_next, Nz_next;

				Nx = Node->m_printNormal[0];
				Ny = Node->m_printNormal[1];
				Nz = Node->m_printNormal[2];
				// mark the first line or the start point of one segment
				if ((Node->GetIndexNo() == 0) || (Node->m_largeJumpFlag == -1)) {

					Nx_prev = Nx;
					Ny_prev = Ny;
					Nz_prev = Nz;

					GLKPOSITION nextPos = WayPointPatch->GetNodeList().Find(Node)->next;
					QMeshNode* nextNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(nextPos);

					Nx_next = nextNode->m_printNormal[0];
					Ny_next = nextNode->m_printNormal[1];
					Nz_next = nextNode->m_printNormal[2];
				}
				// mark the last line or the end point of one segment
				else if ((Node->GetIndexNo() == (lines - 1)) || (Node->m_largeJumpFlag == 1)) {

					GLKPOSITION prevPos = WayPointPatch->GetNodeList().Find(Node)->prev;
					QMeshNode* prevNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(prevPos);

					Nx_prev = prevNode->m_printNormal[0];
					Ny_prev = prevNode->m_printNormal[1];
					Nz_prev = prevNode->m_printNormal[2];

					Nx_next = Nx;
					Ny_next = Ny;
					Nz_next = Nz;
				}
				else {
					GLKPOSITION prevPos = WayPointPatch->GetNodeList().Find(Node)->prev;
					QMeshNode* prevNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(prevPos);

					Nx_prev = prevNode->m_printNormal[0];
					Ny_prev = prevNode->m_printNormal[1];
					Nz_prev = prevNode->m_printNormal[2];

					GLKPOSITION nextPos = WayPointPatch->GetNodeList().Find(Node)->next;
					QMeshNode* nextNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(nextPos);

					Nx_next = nextNode->m_printNormal[0];
					Ny_next = nextNode->m_printNormal[1];
					Nz_next = nextNode->m_printNormal[2];
				}

				double smoothedNx = (1 - alpha) * 0.5 * (Nx_prev + Nx_next) + alpha * Nx;
				double smoothedNy = (1 - alpha) * 0.5 * (Ny_prev + Ny_next) + alpha * Ny;
				double smoothedNz = (1 - alpha) * 0.5 * (Nz_prev + Nz_next) + alpha * Nz;

				double norLength = smoothedNx * smoothedNx + smoothedNy * smoothedNy + smoothedNz * smoothedNz;
				norLength = sqrt(norLength);

				Node->m_printNormal[0] = smoothedNx / norLength;
				Node->m_printNormal[1] = smoothedNy / norLength;
				Node->m_printNormal[2] = smoothedNz / norLength;

			}
		}
	}
}

void fiveAxisPoint::getRawCdata(QMeshPatch* WayPointPatch, double lambda) {

	for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

		double Nx = Node->m_printNormal[0];
		double Ny = Node->m_printNormal[1];
		double Nz = Node->m_printNormal[2];

		// Cal C space Coordinate
		double xCspece = Nx / Nz;
		double yCspece = Ny / Nz;
		// Mark denger points
		double R = sqrt(xCspece * xCspece + yCspece * yCspece);
		double radLambda = lambda * 3.141592653589793 / 180;
		int safeFlag = 0;

		if (R < tan(radLambda)) {
			safeFlag = -1;
		}
		else {
			safeFlag = 0;
		}
		// Construct results
		Node->m_safeFlag = safeFlag;
		Node->m_norCspace[0] = xCspece;
		Node->m_norCspace[1] = yCspece;
	}
	//cout << "norCspace\n" << norCspace << endl;
}

void fiveAxisPoint::getDangerSec(QMeshPatch* WayPointPatch, MatrixXf& sectionTable) {

	int lines = WayPointPatch->GetNodeNumber();
	vector<int> srtPntIndTable;
	vector<int> endPntIndTable;

	for (int i = 0; i < lines - 1; i++) {

		GLKPOSITION Node_Pos = WayPointPatch->GetNodeList().FindIndex(i);
		QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(Node_Pos);
		int safeFlag = Node->m_safeFlag;

		GLKPOSITION nextNode_Pos = WayPointPatch->GetNodeList().FindIndex(i)->next;
		QMeshNode* nextNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(nextNode_Pos);
		int nextSafeFlag = nextNode->m_safeFlag;

		if ((safeFlag == 0 && nextSafeFlag == -1) || (safeFlag == -1 && i == 0)) {
			srtPntIndTable.push_back(i);
		}
		if ((safeFlag == -1 && nextSafeFlag == 0) || (nextSafeFlag == -1 && i == lines - 2)) {
			endPntIndTable.push_back(i + 1);
		}
	}

	if (srtPntIndTable.size() == endPntIndTable.size()) {
		sectionTable.resize(srtPntIndTable.size(), 2);
	}
	else {
		cout << "ERROR : srtPntIndTable.size() != endPntIndTable.size()" << endl;
	}

	for (int i = 0; i < srtPntIndTable.size(); i++) {
		sectionTable(i, 0) = srtPntIndTable[i];
		sectionTable(i, 1) = endPntIndTable[i];
	}

	//cout << "sectionTable:\n"<<sectionTable << endl;
}

void fiveAxisPoint::getNewPntNor(QMeshPatch* WayPointPatch, const MatrixXf& sectionTable, double lambda) {
	// return;
	double radLambda = lambda * 3.141592653589793 / 180;
	double minR = tan(radLambda);
	int secNum = sectionTable.rows();

	for (int i = 0; i < secNum; i++) {

		int srtPntIndex = sectionTable(i, 0);
		int endPntIndex = sectionTable(i, 1);
		vector<double> srtPnt = { 0,0 };
		vector<double> endPnt = { 0,0 };

		getCspacePnt(srtPnt, srtPntIndex, WayPointPatch);
		getCspacePnt(endPnt, endPntIndex, WayPointPatch);
		//cout << "srtPnt " << srtPnt[0] << " , " << srtPnt[1] << endl;
		//cout << "endPnt " << endPnt[0] << " , " << endPnt[1] << endl;
		//cout << "srtPntIndex " << srtPntIndex << " endPntIndex " << endPntIndex << endl;
		//cout << "------------" << endl;

		vector<double> srtPnt_temp = { 0,0 };
		vector<double> endPnt_temp = { 0,0 };
		double R = getRotRadius(srtPnt, endPnt, minR, srtPnt_temp, endPnt_temp);
		//cout << R << endl;

		int quadSrt = getQuadrant(srtPnt_temp);
		int quadEnd = getQuadrant(endPnt_temp);
		int sign = getRotSign(quadSrt, quadEnd, srtPnt_temp, endPnt_temp);

		double angle = getVec2Angle(srtPnt_temp, endPnt_temp);
		//cout << angle << endl;

		int divideNum = endPntIndex - srtPntIndex;
		double deltaAngle = angle / divideNum;
		double startAngle = atan2(srtPnt_temp[1], srtPnt_temp[0]);
		//cout << startAngle << endl;

		for (int ii = srtPntIndex; ii <= endPntIndex; ii++) {

			double xCspace, yCspace;
			if (ii == srtPntIndex) {
				// start point optimize Normal
				if (ii == 0) {
					xCspace = srtPnt_temp[0];
					yCspace = srtPnt_temp[1];
				}// other points do not
				else {
					xCspace = srtPnt[0];
					yCspace = srtPnt[1];
				}

			}
			else if (ii == endPntIndex) {
				// end point optimize Normal
				if (ii == (WayPointPatch->GetNodeNumber() - 1)) {
					xCspace = endPnt_temp[0];
					yCspace = endPnt_temp[1];
				}
				// other points do not
				else {
					xCspace = endPnt[0];
					yCspace = endPnt[1];
				}
			}
			else {
				int tempNum = ii - srtPntIndex;
				double tempAngle = startAngle + deltaAngle * sign * tempNum;
				xCspace = R * cos(tempAngle);
				yCspace = R * sin(tempAngle);
			}

			double newNz = 1 / sqrt(1 + (xCspace * xCspace) + (yCspace * yCspace));
			double newNx = xCspace * newNz;
			double newNy = yCspace * newNz;

			GLKPOSITION Node_Pos = WayPointPatch->GetNodeList().FindIndex(ii);
			QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(Node_Pos);

			Node->m_printNormal[0] = newNx;
			Node->m_printNormal[1] = newNy;
			Node->m_printNormal[2] = newNz;
		}
	}
}

void fiveAxisPoint::getCspacePnt(vector<double>& CspacePnt, int PntIndex, QMeshPatch* WayPointPatch) {

	GLKPOSITION Node_Pos = WayPointPatch->GetNodeList().FindIndex(PntIndex);
	QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(Node_Pos);

	CspacePnt[0] = Node->m_norCspace[0];
	CspacePnt[1] = Node->m_norCspace[1];

	//cout << "CspaceX " << CspacePnt[0] << " CspaceY " << CspacePnt[1] << endl;
}

double fiveAxisPoint::getRotRadius(
	vector<double>& srtPnt, vector<double>& endPnt, double minR, vector<double>& srtPnt_temp, vector<double>& endPnt_temp) {

	double R = 0.0;
	double Rs = sqrt(srtPnt[0] * srtPnt[0] + srtPnt[1] * srtPnt[1]);// [Nx/Nz = srtPnt[0];  Ny/Nz = srtPnt[1]]
	double Re = sqrt(endPnt[0] * endPnt[0] + endPnt[1] * endPnt[1]);

	if ((Rs >= minR) && (Re >= minR)) {
		//R = min(Rs, Re);
		R = minR;
		srtPnt_temp[0] = R / Rs * srtPnt[0];
		srtPnt_temp[1] = R / Rs * srtPnt[1];
		endPnt_temp[0] = R / Re * endPnt[0];
		endPnt_temp[1] = R / Re * endPnt[1];
	}
	else if ((Rs < minR) && (Re >= minR)) {
		R = minR;
		endPnt_temp[0] = R / Re * endPnt[0];
		endPnt_temp[1] = R / Re * endPnt[1];
		srtPnt_temp[0] = endPnt_temp[0];
		srtPnt_temp[1] = endPnt_temp[1];
	}
	else if ((Rs >= minR) && (Re < minR)) {
		R = minR;
		srtPnt_temp[0] = R / Rs * srtPnt[0];
		srtPnt_temp[1] = R / Rs * srtPnt[1];
		endPnt_temp[0] = srtPnt_temp[0];
		endPnt_temp[1] = srtPnt_temp[1];
	}
	else {
		R = 0;
		srtPnt_temp[0] = 0;
		srtPnt_temp[1] = 0;
		endPnt_temp[0] = 0;
		endPnt_temp[1] = 0;
	}
	return R;
}

int fiveAxisPoint::getQuadrant(vector<double>& quadPnt) {
	double x = quadPnt[0];
	double y = quadPnt[1];
	int quadNum = 0;
	if (x >= 0 && y >= 0) {
		quadNum = 1;
	}
	if (x < 0 && y >= 0) {
		quadNum = 2;
	}

	if (x < 0 && y < 0) {
		quadNum = 3;
	}
	if (x >= 0 && y < 0) {
		quadNum = 4;
	}
	return quadNum;
}

int fiveAxisPoint::getRotSign(int quadSrt, int quadEnd, vector<double>& srtPnt, vector<double>& endPnt) {

	int signTable[4][4] = { { 0,  1,  0, -1},
						   {-1,  0,  1,  0},
						   { 0, -1,  0,  1},
						   { 1,  0, -1,  0} };
	int sign = 0;
	if (signTable[quadSrt - 1][quadEnd - 1] != 0) {
		sign = signTable[quadSrt - 1][quadEnd - 1];
	}
	else {
		if (quadSrt == quadEnd) {
			if (quadSrt <= 2) {
				if (srtPnt[0] <= endPnt[0]) {
					sign = -1;
				}
				else {
					sign = 1;
				}
			}
			else {
				if (srtPnt[0] <= endPnt[0]) {
					sign = 1;
				}
				else {
					sign = -1;
				}
			}

		}
		else if (quadSrt > quadEnd) {
			if (-srtPnt[0] <= endPnt[0]) {
				sign = 1;
			}
			else {
				sign = -1;
			}
		}
		else {
			if (-srtPnt[0] <= endPnt[0]) {
				sign = -1;
			}
			else {
				sign = 1;
			}
		}
	}
	//cout << sign << endl;
	return sign;
}

double fiveAxisPoint::getVec2Angle(vector<double>& srtPnt, vector<double>& endPnt) {

	double angle;

	if ((srtPnt[0] == endPnt[0]) && (srtPnt[1] == endPnt[1])) {
		angle = 0;
	}
	else {

		double normSrtPnt = sqrt(srtPnt[0] * srtPnt[0] + srtPnt[1] * srtPnt[1]);
		double normEndPnt = sqrt(endPnt[0] * endPnt[0] + endPnt[1] * endPnt[1]);
		double dotX = srtPnt[0] * endPnt[0] + srtPnt[1] * endPnt[1];

		angle = acos(dotX / (normSrtPnt * normEndPnt));
	}

	return angle;
}

void fiveAxisPoint::getXYZBCE(QMeshPatch* WayPointPatch, int layersNoSolve) {

	double Wx = 0; double Wy = 0; double Wz = 0;

	for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

		//int lines = layerPntNor.rows();
		//for (int i = 0; i < lines; i++) {

		double Px = Node->m_printPostion[0];
		double Py = Node->m_printPostion[1];
		double Pz = Node->m_printPostion[2];
		double Nx = Node->m_printNormal[0];
		double Ny = Node->m_printNormal[1];
		double Nz = Node->m_printNormal[2];

		if (WayPointPatch->GetIndexNo() < layersNoSolve) { // When layersNoSolve = 0, there will be no special actions.
			Nx = 0; Ny = 0; Nz = 1;
		}

		double B = -acos(Nz);
		double C = -atan2(Ny, Nx);
		double X = cos(B) * cos(C) * Px - cos(B) * sin(C) * Py + sin(B) * Pz + Wx;
		double Y = sin(C) * Px + cos(C) * Py + Wy;
		double Z = -sin(B) * cos(C) * Px + sin(B) * sin(C) * Py + cos(B) * Pz + Wz;
		// Translate from rad to dreg
		B = B * 180 / 3.141592653589793;
		C = C * 180 / 3.141592653589793;

		Node->m_XYZBCE[0] = X;
		Node->m_XYZBCE[1] = Y;
		Node->m_XYZBCE[2] = Z;
		Node->m_XYZBCE[3] = B;
		Node->m_XYZBCE[4] = C;
	}
}

void fiveAxisPoint::optimizationC(QMeshPatch* WayPointPatch) {

	for (int loop = 0; loop < 7; loop++) {

		double threshhold = 180.0;

		for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
			QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

			//int lines = layerXYZBCE.rows();
			//for (int i = 1; i < lines; i++) {

			double C = Node->m_XYZBCE[4];

			if (Node->GetIndexNo() == 0) continue;
			GLKPOSITION prevPos = WayPointPatch->GetNodeList().Find(Node)->prev;
			QMeshNode* prevNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(prevPos);
			double preC = prevNode->m_XYZBCE[4];

			if (C - preC < -threshhold) {
				C = C + 360;
			}
			else if (C - preC > threshhold) {
				C = C - 360;
			}
			else {}

			Node->m_XYZBCE[4] = C;
		}
	}
}

void fiveAxisPoint::writeGcode(PolygenMesh* polygenMesh_Waypoints, string rltDir) {

	// Define the basic parameter
	double Z_home = 200;						// The hight of Home point; / mm
	double Z_high = 30;							// The hight of G1 point(for safety); / mm

	int E2temperature = 200;					// The temperature of Extruder 2
	int E3temperature = 200;					// The temperature of Extruder 3

	int F_G0_XYBC = 4500;						// Speed of G0 move of XYBC
	int F_G0_Z = 4000;							// Speed of G0 move of Z
	int F_G1_support = 1500;					// Speed of G1 support material (normal 2ed~layers)
	int F_G1_original = 1500;					// Speed of G1 original material (normal 2ed~layers)
	int F_G1_1stlayer = 1500;					// Speed of G1(special 1st layer)
	int F_PumpCompensate = 3000;				// Speed of PumpCompensate

	double E_PumpBack = -4.5;					// The extruder pump back Xmm
	double E_PumpCompensate = 3.5 + E_PumpBack; // The extruder pump compensate Xmm
	double E_PumpCompensateL1 = 10;				// The extruder pump compensate for 1st layer Xmm
	double E_PumpCompensateNewE = 6;			// The extruder pump compensate for new type layer Xmm

	// important parameter for extruder 3
	double xExtuderOffset = -2.0;
	double yExtuderOffset = 0.0;

	char targetFilename[1024];
	sprintf(targetFilename, "%s%s", "../2_GenratedGcode/", rltDir.c_str());
	FILE* fp = fopen(targetFilename, "w");
	if (!fp) {
		perror("Couldn't open the directory");
		return;
	}

	// Record the max Z for security consideration (1st layer)
	GLKPOSITION layer1st_Pos = polygenMesh_Waypoints->GetMeshList().GetHeadPosition();
	QMeshPatch* layer1st_WayPointPatch = (QMeshPatch*)polygenMesh_Waypoints->GetMeshList().GetNext(layer1st_Pos);

	double Z_max = -99999.9;
	for (GLKPOSITION Pos = layer1st_WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* Node = (QMeshNode*)layer1st_WayPointPatch->GetNodeList().GetNext(Pos);

		double Pz = Node->m_XYZBCE[2];
		if (Pz > Z_max) { Z_max = Pz; }
	}
	// Record the layer type of 1st Layer
	bool IsSupportLayer_last = layer1st_WayPointPatch->isSupportLayer;

	// Give the start message of G_code
	if (layer1st_WayPointPatch->isSupportLayer == true) {
		std::fprintf(fp, "G90\n");
		std::fprintf(fp, "M104 S%d T3\n", E3temperature);
		std::fprintf(fp, "G28 X0.000 Y0.000 Z%.3f B0.000 C0.000\n", Z_home);
		std::fprintf(fp, "M109 S%d T3\n", E3temperature);
	}
	else {
		std::fprintf(fp, "G90\n");
		std::fprintf(fp, "M104 S%d T2\n", E2temperature);
		std::fprintf(fp, "G28 X0.000 Y0.000 Z%.3f B0.000 C0.000\n", Z_home);
		std::fprintf(fp, "M109 S%d T2\n", E2temperature);
	}


	for (GLKPOSITION Pos = polygenMesh_Waypoints->GetMeshList().GetHeadPosition(); Pos;) {
		QMeshPatch* WayPointPatch = (QMeshPatch*)polygenMesh_Waypoints->GetMeshList().GetNext(Pos);

		for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
			QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);
			double X = Node->m_XYZBCE[0]; double Y = Node->m_XYZBCE[1]; double Z = Node->m_XYZBCE[2];
			double B = Node->m_XYZBCE[3]; double C = Node->m_XYZBCE[4]; double E = Node->m_XYZBCE[5];

			if (Z < 0.0) { Node->negativeZ = true; }
			if (WayPointPatch->GetIndexNo() == 0) { E = E * 1.1; }

			//modify the xy-offset according to the extruder
			if (WayPointPatch->isSupportLayer == true) {
				X = X + xExtuderOffset;
				Y = Y + yExtuderOffset;
			}
			// Record the max Z for security consideration (rest layers)
			if (Z > Z_max) { Z_max = Z; }

			// Add some auxiliary G code
			if (Node->GetIndexNo() == 0) {

				if (WayPointPatch->GetIndexNo() == 0) {
					// move to start of printing location
					std::fprintf(fp, "G0 X%.3f Y%.3f B%.3f C%.3f F%d\n", X, Y, B, C, F_G0_XYBC);
					// slowly lower for printing
					std::fprintf(fp, "G0 Z%.3f F%d\n", (Z_max + Z_high), F_G0_Z);
					// zero extruded length(set E axis to 0)
					std::fprintf(fp, "G92 E0\n");
					std::fprintf(fp, "G1 E%.3f F%d\n", E_PumpCompensateL1, F_PumpCompensate);
					std::fprintf(fp, "G92 E0\n");
					std::fprintf(fp, "G1 F%d\n", F_G1_1stlayer);
				}
				else if (WayPointPatch->isSupportLayer == IsSupportLayer_last) {
					std::fprintf(fp, "G92 E0\n");
					std::fprintf(fp, "G0 E%.3f F%d\n", E_PumpBack * 0.8, F_G0_XYBC);
					// return to the safe point Z_max + Z_high + 2
					std::fprintf(fp, "G0 Z%.3f\n", (Z_max + Z_high + 2));
					// move to start of printing location
					std::fprintf(fp, "G0 X%.3f Y%.3f B%.3f C%.3f E%.3f F%d\n", X, Y, B, C, E_PumpBack * 0.9, F_G0_XYBC);
					// slowly lower for printing
					std::fprintf(fp, "G0 Z%.3f E%.3f F%d\n", (Z + Z_high), E_PumpBack, F_G0_Z);
					std::fprintf(fp, "G1 E%.3f F%d\n", E_PumpCompensate, F_PumpCompensate);
					std::fprintf(fp, "G92 E0\n");

					if (WayPointPatch->isSupportLayer == true) {
						std::fprintf(fp, "G1 F%d\n", F_G1_support);
					}
					else {
						std::fprintf(fp, "G1 F%d\n", F_G1_original);
					}

				}
				// the special case: exchange extruder
				else {
					std::fprintf(fp, "G92 E0\n");
					std::fprintf(fp, "G0 E%.3f F%d\n", E_PumpBack, F_G0_XYBC);
					// return to the home point Z_home
					std::fprintf(fp, "G0 Z%.3f\n", Z_home);

					// change extruder
					if (WayPointPatch->isSupportLayer == true)
					{
						std::fprintf(fp, "M104 S%d T3\n", E3temperature);
					}
					else { std::fprintf(fp, "M104 S%d T2\n", E2temperature); }
					std::fprintf(fp, "G92 E0\n");

					// move to start of printing location
					std::fprintf(fp, "G0 X%.3f Y%.3f B%.3f C%.3f E%.3f F%d\n", X, Y, B, C, E_PumpBack * 0.25, F_G0_XYBC);
					std::fprintf(fp, "G0 Z%.3f E%.3f F%d\n", (Z + Z_high), E_PumpBack * 0.5, F_G0_Z);
					// slowly lower for printing
					std::fprintf(fp, "G92 E0\n");
					std::fprintf(fp, "G1 E%.3f F%d\n", E_PumpCompensateNewE, F_PumpCompensate);
					std::fprintf(fp, "G92 E0\n");

					if (WayPointPatch->isSupportLayer == true) {
						std::fprintf(fp, "G1 F%d\n", F_G1_support);
					}
					else {
						std::fprintf(fp, "G1 F%d\n", F_G1_original);
					}
				}
				std::fprintf(fp, "G1 X%.3f Y%.3f Z%.3f B%.3f C%.3f E%.3f\n", X, Y, Z, B, C, E);
			}
			else {
				// Consider the waypoints with too large Length
				if (Node->m_largeJumpFlag == -1) {

					GLKPOSITION prevPos = WayPointPatch->GetNodeList().Find(Node)->prev;
					QMeshNode* prevNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(prevPos);
					double oldZ = prevNode->m_XYZBCE[2];

					std::fprintf(fp, "G0 E%.3f F5000\n", (E - 3));
					std::fprintf(fp, "G0 Z%.3f F5000\n", (max(Z_max, oldZ) + 20));
					std::fprintf(fp, "G0 X%.3f Y%.3f B%.3f C%.3f F5000\n", X, Y, B, C);

					// std::fprintf(fp, "G0 Z%.3f E%.3f\n", Z, E);

					if (WayPointPatch->isSupportLayer == true) {
						std::fprintf(fp, "G0 Z%.3f E%.3f F%d\n", Z, E, F_G1_support);
					}
					else {
						std::fprintf(fp, "G0 Z%.3f E%.3f F%d\n", Z, E, F_G1_original);
					}
				}
				std::fprintf(fp, "G1 X%.3f Y%.3f Z%.3f B%.3f C%.3f E%.3f\n", X, Y, Z, B, C, E);
			}
		}

		//std::cout << WayPointPatch->isSupportLayer << std::endl;
		std::cout << ".";
		if (((WayPointPatch->GetIndexNo() + 1) % 100 == 0) ||
			((WayPointPatch->GetIndexNo() + 1) == polygenMesh_Waypoints->GetMeshList().GetCount())) {
			std::cout << std::endl;
		}

		IsSupportLayer_last = WayPointPatch->isSupportLayer;
	}

	std::fprintf(fp, "G92 E0\n");
	std::fprintf(fp, "G0 E%.3f F%d\n", E_PumpBack, F_G0_XYBC); // PumpBack
	std::fprintf(fp, "G0 Z%.3f\n", Z_home); // return to the home point Z_home
	std::fprintf(fp, "G0 X0 Y0 B0 C0 F%d\n", F_G0_XYBC);
	std::fprintf(fp, "M30");// Stop all of the motion

	std::fclose(fp);

	std::cout << "------------------------------------------- " << rltDir << " Write OK!" << endl;
}

void fiveAxisPoint::testXYZBCE(QMeshPatch* WayPointPatch, string Dir, bool testSwitch) {

	if (testSwitch == true) {
		char targetFilename[1024];

		sprintf(targetFilename, "%s%s%s%d%s", "../3_TestData/", Dir.c_str(), "/", WayPointPatch->GetIndexNo() + 1, ".txt");

		// cout << targetFilename << endl;

		FILE* fp = fopen(targetFilename, "w");
		if (!fp)	return;

		//int lines = layerData.rows();
		//for (int i = 0; i < lines; i++) {
		for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
			QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

			double X = Node->m_XYZBCE[0]; double Y = Node->m_XYZBCE[1]; double Z = Node->m_XYZBCE[2];
			double B = Node->m_XYZBCE[3]; double C = Node->m_XYZBCE[4]; double E = Node->m_XYZBCE[5];
			fprintf(fp, "%f %f %f %f %f %f\n", X, Y, Z, B, C, E);
		}

		fclose(fp);

		cout << "------------------------------------------- Test open" << endl;
	}

}

void fiveAxisPoint::testLayerHeight(QMeshPatch* WayPointPatch, string Dir, bool testSwitch) {

	if (testSwitch == true) {
		char targetFilename[1024];

		sprintf(targetFilename, "%s%s%s%d%s", "../3_TestData/", Dir.c_str(), "/", WayPointPatch->GetIndexNo() + 1, ".txt");

		// cout << targetFilename << endl;

		FILE* fp = fopen(targetFilename, "w");
		if (!fp)	return;

		//int lines = layerData.rows();
		//for (int i = 0; i < lines; i++) {
		for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
			QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

			double layerHeight = Node->m_layerHeight;
			fprintf(fp, "%f\n", layerHeight);
		}

		fclose(fp);

		cout << "------------------------------------------- Test open" << endl;
	}

}

void fiveAxisPoint::height2E(PolygenMesh* polygenMesh_Waypoints, bool func_switch) {

	// E = E + ratio * height * length * width;
	double ratio = 0.5; // Dicided by CNC W.R.T(E:Volume:E = 0.48)
	double width = 0.68;

	for (GLKPOSITION Pos = polygenMesh_Waypoints->GetMeshList().GetHeadPosition(); Pos;) {
		QMeshPatch* WayPointPatch = (QMeshPatch*)polygenMesh_Waypoints->GetMeshList().GetNext(Pos);

		double E = 0;

		for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
			QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

			double deltaE;
			double height;
			double length = Node->m_XYZBCE[5];

			if (func_switch == true) {
				height = Node->m_layerHeight;
				deltaE = ratio * height * length * width;
			}
			else {
				deltaE = 0.25 * length;
			}

			E = E + deltaE;
			Node->m_XYZBCE[5] = E;
		}
		std::cout << ".";
		if (((WayPointPatch->GetIndexNo() + 1) % 100 == 0) ||
			((WayPointPatch->GetIndexNo() + 1) == polygenMesh_Waypoints->GetMeshList().GetCount())) {
			std::cout << std::endl;
		}
	}

	if (func_switch == true) {
		std::cout << "------------------------------------------- Height2E OK!" << std::endl;
	}
	else {
		std::cout << "------------------------------------------- DeltaE2E OK!" << std::endl;
	}
}

void fiveAxisPoint::detectCollision(
	PolygenMesh* polygenMesh_Waypoints, PolygenMesh* polygenMesh_extruderHead, bool func_switch) {

	if (func_switch == false) return;

	QMeshPatch* eHead = (QMeshPatch*)polygenMesh_extruderHead->GetMeshList().GetHead();
	QHULLSET* eHeadConvexFront = buildConvexHull_extruderHead(eHead);

	int layerDepth = 6;
	//int layers = xyzbceCell.size();
	//for (int layerInd = 0; layerInd < layers; layerInd++) {

	for (GLKPOSITION Pos = polygenMesh_Waypoints->GetMeshList().GetHeadPosition(); Pos;) {
		QMeshPatch* WayPointPatch = (QMeshPatch*)polygenMesh_Waypoints->GetMeshList().GetNext(Pos);

		//int lines = xyzbceCell[layerInd].rows();
		int detetedLayerInd = max(WayPointPatch->GetIndexNo() - layerDepth, 0);

		for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
			QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

			//for (int lineInd = 0; lineInd < lines; lineInd++) {
			double printHeadPos_x = Node->m_XYZBCE[0];
			double printHeadPos_y = Node->m_XYZBCE[1];
			double printHeadPos_z = Node->m_XYZBCE[2];
			double platformRad_B = Node->m_XYZBCE[3] * 3.141592653589793 / 180;
			double platformRad_C = Node->m_XYZBCE[4] * 3.141592653589793 / 180;

			// test previous layer
			//for (int preLayerInd = WayPointPatch->GetIndexNo() - 1; preLayerInd >= detetedLayerInd; preLayerInd--) {
			int layerLoop = 0;
			for (GLKPOSITION prevLayerPos = polygenMesh_Waypoints->GetMeshList().Find(WayPointPatch)->prev; prevLayerPos;) {
				QMeshPatch* prevLayerWayPointPatch = (QMeshPatch*)polygenMesh_Waypoints->GetMeshList().GetPrev(prevLayerPos);

				if (layerLoop > 6) break;

				for (GLKPOSITION prevLayerNodePos = prevLayerWayPointPatch->GetNodeList().GetHeadPosition(); prevLayerNodePos;) {
					QMeshNode* prevLayerNode = (QMeshNode*)prevLayerWayPointPatch->GetNodeList().GetNext(prevLayerNodePos);

					//int preLayerlines = xyzbceCell[preLayerInd].rows();
					//for (int preLayerlineInd = 0; preLayerlineInd < preLayerlines; preLayerlineInd++) {

					double Px = prevLayerNode->m_printPostion[0];
					double Py = prevLayerNode->m_printPostion[1];
					double Pz = prevLayerNode->m_printPostion[2];
					double B = platformRad_B;
					double C = platformRad_C;

					double X = cos(B) * cos(C) * Px - cos(B) * sin(C) * Py + sin(B) * Pz - printHeadPos_x;
					double Y = sin(C) * Px + cos(C) * Py - printHeadPos_y;
					double Z = -sin(B) * cos(C) * Px + sin(B) * sin(C) * Py + cos(B) * Pz - printHeadPos_z;

					double pnt[3] = { X,Y,Z };
					bool isInHull = _isPntInsideConvexHull(eHeadConvexFront, pnt);
					if (isInHull) {
						Node->isCollision = true;
						prevLayerNode->iscollided = true;
						cout << "Layer:[" << WayPointPatch->GetIndexNo() << "]\tPnt Index:[" << Node->GetIndexNo() << "]\t     COLLISION (different Layer)." << endl;
						cout << "Collided point: Layer:[" << prevLayerWayPointPatch->GetIndexNo() << "]\tPnt Index:[" << prevLayerNode->GetIndexNo() << "]" << endl;
						//cout << "/";
						//break;
					}
				}
				layerLoop++;
			}

			// test previous points in the same layer
			GLKPOSITION prevPos = polygenMesh_Waypoints->GetMeshList().Find(WayPointPatch);
			QMeshPatch* prevWayPointPatch = (QMeshPatch*)polygenMesh_Waypoints->GetMeshList().GetAt(prevPos);

			for (GLKPOSITION prevNodePos = prevWayPointPatch->GetNodeList().GetHeadPosition(); prevNodePos;) {
				QMeshNode* prevNode = (QMeshNode*)prevWayPointPatch->GetNodeList().GetNext(prevNodePos);

				//for (int prelineInd = 0; prelineInd < lineInd; prelineInd++) {
				if (prevNode->GetIndexNo() >= Node->GetIndexNo()) break;

				double Px = prevNode->m_printPostion[0];
				double Py = prevNode->m_printPostion[1];
				double Pz = prevNode->m_printPostion[2];
				double B = platformRad_B;
				double C = platformRad_C;

				double X = cos(B) * cos(C) * Px - cos(B) * sin(C) * Py + sin(B) * Pz - printHeadPos_x;
				double Y = sin(C) * Px + cos(C) * Py - printHeadPos_y;
				double Z = -sin(B) * cos(C) * Px + sin(B) * sin(C) * Py + cos(B) * Pz - printHeadPos_z;

				double pnt[3] = { X,Y,Z };
				bool isInHull = _isPntInsideConvexHull(eHeadConvexFront, pnt);
				if (isInHull) {
					Node->isCollision = true;
					prevNode->iscollided = true;
					cout << "Layer:[" << WayPointPatch->GetIndexNo() << "]\tPnt Index:[" << Node->GetIndexNo() << "]\t     COLLISION (same layer)." << endl;
					//cout << "*";
					//break;
				}
			}
			//cout << "*";
		}
		//cout << ".";
		//cout << "." << endl;
		cout << "\n----------------------- Layer " << WayPointPatch->GetIndexNo() << " detected." << endl;
		//if (((layerInd + 1) % 100 == 0) || ((layerInd + 1) == layers)) cout << endl;

	}
	cout << "------------------------------------------- Collision detection OK!" << endl;
}

QHULLSET* fiveAxisPoint::buildConvexHull_extruderHead(QMeshPatch* eHead) {
	facetT* facet;		vertexT* vertex, ** vertexp;
	int i, index, pntNum, num, stIndex;			float pos[3];
	double vec[3][3], dir[3], v1[3], v2[3], pp[3];
	QHULLSET* newConvexFront = NULL; // new convexhull used for checking

	//-------------------------------------------------------------------------------------
	//	Step 1: initialization

	pntNum = eHead->GetNodeNumber();
	double* pntArray = (double*)malloc(sizeof(double) * 3 * pntNum); //points use to compute convex hull

	int nodeIndex = 0;
	for (GLKPOSITION posMesh = eHead->GetNodeList().GetHeadPosition(); posMesh != nullptr;) {
		QMeshNode* node = (QMeshNode*)eHead->GetNodeList().GetNext(posMesh);
		node->GetCoord3D(pp[0], pp[1], pp[2]);
		for (int i = 0; i < 3; i++)
			pntArray[nodeIndex * 3 + i] = pp[i];
		nodeIndex++;
	}

	//-------------------------------------------------------------------------------------
	//	Step 2: computaing the convex-hull
	qh_init_A(stdin, stdout, stderr, 0, NULL);
	qh_initflags("Qt Qx");
	qh_init_B(pntArray, pntNum, 3, false);
	qh_qhull();
	qh_check_output();
	qh_triangulate();
	if (qh VERIFYoutput && !qh STOPpoint && !qh STOPcone) qh_check_points();

	//-------------------------------------------------------------------------------------
	//	Step 3: output the results of convex-hull computation
	int nodeNum = 0, faceNum = 0;
	faceNum = qh_qh.num_facets;		nodeNum = qh_qh.num_vertices;
	//printf("Convex-Hull: %d faces with %d vertices\n",faceNum,nodeNum);
	if (faceNum > 0 && nodeNum > 0) {
		newConvexFront = _mallocMemoryConvexHull(faceNum, nodeNum);
		//---------------------------------------------------------------------------------
		index = 0;
		FORALLvertices{
			vertex->id = index;	// before this assignment, "vertex->id" contains the id of input vertices
		newConvexFront->vertPos[index * 3] = vertex->point[0];
		newConvexFront->vertPos[index * 3 + 1] = vertex->point[1];
		newConvexFront->vertPos[index * 3 + 2] = vertex->point[2];
		index++;
		}
			//---------------------------------------------------------------------------------
		index = 0;
		FORALLfacets{
			newConvexFront->normalVec[index * 3] = facet->normal[0];
		newConvexFront->normalVec[index * 3 + 1] = facet->normal[1];
		newConvexFront->normalVec[index * 3 + 2] = facet->normal[2];
		newConvexFront->offset[index] = facet->offset;
		//	It has been verified all normal[] vectors generated by qhull library are pointing outwards and are unit-vectors 
		//		(verified by the function -- QuadTrglMesh* convexHullGeneration(QuadTrglMesh* inputMesh)  ).

		int i = 0;
		FOREACHvertex_(facet->vertices) {
			newConvexFront->faceTable[index * 3 + i] = vertex->id + 1; //index start from 1;
			//newConvexFront->faceTable[index * 3 + i] = vertex->id; //index start from 0;

			SET(vec[i],vertex->point);
			i++;
			if (i >= 3) break; // Note that it could be a facet with more than 3 vertices if not applying "qh_triangulate();"
		}

		//-----------------------------------------------------------------------------
		//	Check if the vertices on this face is given in the anti-clockwise order
		SUB(v1,vec[1],vec[0]);
		SUB(v2,vec[2],vec[0]);
		CROSS(dir,v1,v2);
		if (DOT(dir,facet->normal) < 0) {
			unsigned int temp = newConvexFront->faceTable[index * 3];
			newConvexFront->faceTable[index * 3] = newConvexFront->faceTable[index * 3 + 2];
			newConvexFront->faceTable[index * 3 + 2] = temp;
		}

		index++;
		}
	}

	//-------------------------------------------------------------------------------------
	//	Step 4: free the memory
	int curlong, totlong;
	qh_freeqhull(false);
	qh_memfreeshort(&curlong, &totlong);
	if (curlong || totlong) fprintf(stderr, "qhull internal warning (main): did not free %d bytes of long memory (%d pieces)\n", totlong, curlong);
	//-------------------------------------------------------------------------------------
	free(pntArray);

	return newConvexFront;
}

QHULLSET* fiveAxisPoint::_mallocMemoryConvexHull(int faceNum, int vertNum)
{
	QHULLSET* pConvexHull;

	pConvexHull = (QHULLSET*)malloc(sizeof(QHULLSET));
	pConvexHull->faceNum = faceNum;
	pConvexHull->normalVec = (double*)malloc(sizeof(double) * 3 * faceNum);
	pConvexHull->offset = (double*)malloc(sizeof(double) * faceNum);

	pConvexHull->faceTable = (unsigned int*)malloc(sizeof(unsigned int) * 3 * faceNum);

	pConvexHull->vertNum = vertNum;
	pConvexHull->vertPos = (double*)malloc(sizeof(double) * 3 * vertNum);

	return pConvexHull;
}

bool fiveAxisPoint::_isPntInsideConvexHull(QHULLSET* pConvexHull, double pnt[]) {
	double normVec[3], offValue;

	for (int i = 0; i < pConvexHull->faceNum; i++) {
		normVec[0] = pConvexHull->normalVec[i * 3];
		normVec[1] = pConvexHull->normalVec[i * 3 + 1];
		normVec[2] = pConvexHull->normalVec[i * 3 + 2];
		offValue = pConvexHull->offset[i];
		if ((DOT(pnt, normVec) + offValue) >= 0.0) return false;

	}
	return true;
}

void fiveAxisPoint::_freeMemoryConvexHull(QHULLSET*& pConvexHull)
{
	free((pConvexHull->normalVec));
	free((pConvexHull->offset));
	free((pConvexHull->faceTable));
	free((pConvexHull->vertPos));
	free(pConvexHull);

	pConvexHull = NULL;
}