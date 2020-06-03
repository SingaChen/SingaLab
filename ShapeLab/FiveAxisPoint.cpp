#include "FiveAxisPoint.h"
#include "alphanum.hpp"

#define PI		3.141592654
#define DEGREE_TO_ROTATE(x)		0.0174532922222*x
#define ROTATE_TO_DEGREE(x)		57.295780490443*x

void fiveAxisPoint::natSort(QString dirctory, vector<string>& fileNameCell)
{
	if (fileNameCell.empty() == false) return;

	DIR* dp;
	struct dirent* ep;
	string fullDir = "../1_GcodeGeneModel/" + dirctory.toStdString();
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

void fiveAxisPoint::readWayPointData(QString packName, bool Yup2Zup_switch, GLKObList* polygenMeshList, vector<string> wayPointFileCell, GLKLib* pGLK)
{
	// isbuiled
	
	for (GLKPOSITION pos = polygenMeshList->GetHeadPosition(); pos != nullptr;) 
	{
		PolygenMesh* polygenMesh = (PolygenMesh*)polygenMeshList->GetNext(pos);
		if ("Waypoints" == polygenMesh->getModelName()) return;
	}

	// get all num of waypoints
	long waypntNum = 0;

	PolygenMesh* waypointSet = new PolygenMesh;
	waypointSet->setModelName("Waypoints");
	waypointSet->meshType = WAYPOINT;

	//read slice files and build mesh_patches
	char filename[1024];

	for (int i = 0; i < wayPointFileCell.size(); i++) 
	{

		sprintf(filename, "%s%s%s%s", "../1_GcodeGeneModel/", packName.toStdString().c_str(), "/", wayPointFileCell[i].data());
		// cout << wayPointFileCell[i].data() << endl;

		QMeshPatch* waypoint = new QMeshPatch;
		waypoint->SetIndexNo(waypointSet->GetMeshList().GetCount()); //index begin from 0
		waypointSet->GetMeshList().AddTail(waypoint);
		waypoint->waypointPatchName = wayPointFileCell[i].data();

		// isSupportLayer
		string::size_type supportFlag = wayPointFileCell[i].find("S");
		if (supportFlag == string::npos)	waypoint->isSupportLayer = false;
		else { waypoint->isSupportLayer = true; }

		//cout << waypoint->isSupportLayer << endl;

		waypoint->inputPosNorFile(filename, waypoint->isSupportLayer, Yup2Zup_switch);
		waypntNum += waypoint->GetNodeNumber();
	}
	////Display
	waypointSet->BuildGLList(waypointSet->m_bVertexNormalShading);
	polygenMeshList->AddTail(waypointSet);
	pGLK->AddDisplayObj(waypointSet, true);
	std::cout << "------------------------------------------- WayPoints Load OK!" << std::endl;
}

void fiveAxisPoint::readSliceData(QString sliceSetName, bool Yup2Zup_switch, double Zoff, double Xoff, double Yoff, GLKObList* polygenMeshList, vector<string> sliceSetFileCell, GLKLib* pGLK)
{
	// isbuiled
	for (GLKPOSITION pos = polygenMeshList->GetHeadPosition(); pos != nullptr;) {
		PolygenMesh* polygenMesh = (PolygenMesh*)polygenMeshList->GetNext(pos);
		if ("Slices" == polygenMesh->getModelName()) return;
	}

	PolygenMesh* sliceSet = new PolygenMesh;
	sliceSet->setModelName("Slices");
	sliceSet->meshType = LAYER;

	//read slice files and build mesh_patches
	char filename[1024];
	for (int i = 0; i < sliceSetFileCell.size(); i++)
	{
		sprintf(filename, "%s%s%s%s", "../1_GcodeGeneModel/", sliceSetName.toStdString().c_str(), "/", sliceSetFileCell[i].data());

		QMeshPatch* slice = new QMeshPatch;
		slice->SetIndexNo(sliceSet->GetMeshList().GetCount()); //index begin from 0
		sliceSet->GetMeshList().AddTail(slice);
		slice->layerPatchName = sliceSetFileCell[i].data();

		// isSupportLayer
		string::size_type supportFlag = sliceSetFileCell[i].find("S");
		if (supportFlag == string::npos)
			slice->isSupportLayer = false;
		else
			slice->isSupportLayer = true;

		//choose the method for reading files with different types
		string targetFileType = ".obj";
		string fileName = sliceSetFileCell[i].data();
		if (fileName.find(targetFileType)) {
			slice->inputOBJFile(filename, false, Yup2Zup_switch, Zoff, Xoff, Yoff);
		}
		else {
			slice->inputOFFFile(filename, false, Yup2Zup_switch, Zoff, Xoff, Yoff);
		}
	}
	//Display
	sliceSet->BuildGLList(sliceSet->m_bVertexNormalShading);
	polygenMeshList->AddTail(sliceSet);
	pGLK->AddDisplayObj(sliceSet, true);
	cout << "------------------------------------------- Slices Load OK!" << endl;
}

void fiveAxisPoint::readExtruderHeadfile(string extruderHeadName, GLKObList* polygenMeshList, GLKLib* pGLK)
{
	// isbuiled
	for (GLKPOSITION pos = polygenMeshList->GetHeadPosition(); pos != nullptr;) {
		PolygenMesh* polygenMesh = (PolygenMesh*)polygenMeshList->GetNext(pos);
		if ("ExtruderHead" == polygenMesh->getModelName()) return;
	}

	PolygenMesh* extruderHead = new PolygenMesh;
	extruderHead->setModelName("ExtruderHead");
	extruderHead->meshType = CNC_PARTS;

	//read slice files and build mesh_patches
	char filename[1024];

	sprintf(filename, "%s%s", "../1_GcodeGeneModel/", extruderHeadName.c_str());
	//cout << filename << endl;

	QMeshPatch* eHead = new QMeshPatch;
	eHead->SetIndexNo(extruderHead->GetMeshList().GetCount()); //index begin from 0
	extruderHead->GetMeshList().AddTail(eHead);
	eHead->inputOBJFile(filename, false, false, 0.0);

	// enlarge 1.1x eHead
	for (GLKPOSITION eHeadPos = eHead->GetNodeList().GetHeadPosition(); eHeadPos;) {
		QMeshNode* eHeadNode = (QMeshNode*)eHead->GetNodeList().GetNext(eHeadPos);

		double xx, yy, zz;
		eHeadNode->GetCoord3D(xx, yy, zz);
		if (xx == 0.0 && yy == 0.0 && zz == 0.0) {
			eHeadNode->isPrintPnt = true;
		}

		xx *= 1.1; yy *= 1.1; zz *= 1.1;
		eHeadNode->SetCoord3D(xx, yy, zz);
	}
	// record the original(= print type: Zup + 0,0,0 ) message
	for (GLKPOSITION eHeadPos = eHead->GetNodeList().GetHeadPosition(); eHeadPos;) {
		QMeshNode* node = (QMeshNode*)eHead->GetNodeList().GetNext(eHeadPos);
		double pp[3];
		node->GetCoord3D(pp[0], pp[1], pp[2]);
		node->m_printPostion[0] = pp[0];
		node->m_printPostion[1] = pp[1];
		node->m_printPostion[2] = pp[2];
	}

	////Display
	extruderHead->BuildGLList(extruderHead->m_bVertexNormalShading);
	polygenMeshList->AddTail(extruderHead);
	pGLK->AddDisplayObj(extruderHead, true);

	cout << "------------------------------------------- Extruder Head Load OK!" << endl;
}

void fiveAxisPoint::readPlatformfile(string platformName, GLKObList* polygenMeshList, GLKLib* pGLK)
{
	// isbuiled
	for (GLKPOSITION pos = polygenMeshList->GetHeadPosition(); pos != nullptr;) {
		PolygenMesh* polygenMesh = (PolygenMesh*)polygenMeshList->GetNext(pos);
		if ("PrintPlatform" == polygenMesh->getModelName()) return;
	}

	PolygenMesh* platform = new PolygenMesh;
	platform->setModelName("PrintPlatform");
	platform->meshType = CNC_PARTS;

	//read slice files and build mesh_patches
	char filename[1024];

	sprintf(filename, "%s%s", "../1_GcodeGeneModel/", platformName.c_str());
	//cout << filename << endl;

	QMeshPatch* platformPatch = new QMeshPatch;
	platformPatch->SetIndexNo(platform->GetMeshList().GetCount()); //index begin from 0
	platform->GetMeshList().AddTail(platformPatch);
	platformPatch->inputOBJFile(filename, false, false, 0.0);

	// record the original(= print type: Zup + 0,0,0) message
	for (GLKPOSITION Pos = platformPatch->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* node = (QMeshNode*)platformPatch->GetNodeList().GetNext(Pos);
		double pp[3];
		node->GetCoord3D(pp[0], pp[1], pp[2]);
		node->m_printPostion[0] = pp[0];
		node->m_printPostion[1] = pp[1];
		node->m_printPostion[2] = pp[2];
	}

	////Display
	platform->BuildGLList(platform->m_bVertexNormalShading);
	polygenMeshList->AddTail(platform);
	pGLK->AddDisplayObj(platform, true);

	cout << "------------------------------------------- Platform Load OK!" << endl;
}

void fiveAxisPoint::getLayerHeight(PolygenMesh* polygenMesh_Slices, PolygenMesh* polygenMesh_Waypoints, PolygenMesh* polygenMesh_PrintPlatform,bool varyThickness_switch, int GcodeGeneRange_From, int GcodeGeneRange_To, bool upPlate2Height_switch, double upZdist, double Xmove, double Ymove) {

	if (varyThickness_switch == false) return;

	cout << "------------------------------------------- Layer Height Calculation Running ..." << endl;
	long time = clock();

	// get the patch point of polygenMesh_PrintPlatform
	QMeshPatch* patch_PrintPlatform = (QMeshPatch*)polygenMesh_PrintPlatform->GetMeshList().GetHead();
	// move up platform for LayerHeight calculation, if need
	if (upPlate2Height_switch == true) {
		for (GLKPOSITION Pos = patch_PrintPlatform->GetNodeList().GetHeadPosition(); Pos;) {
			QMeshNode* Node = (QMeshNode*)patch_PrintPlatform->GetNodeList().GetNext(Pos);
			double pp[3];
			Node->GetCoord3D(pp[0], pp[1], pp[2]);
			pp[0] += Xmove;
			pp[1] += Ymove;
			pp[2] += upZdist;//+
			Node->SetCoord3D(pp[0], pp[1], pp[2]);
		}
	}
	int Core = 12;
#pragma omp parallel
	{
#pragma omp for  
		for (int omptime = 0; omptime < Core; omptime++) {

			// topLayer --> layer on the highest place [travel head to tail]
			for (GLKPOSITION Pos = polygenMesh_Slices->GetMeshList().GetHeadPosition(); Pos;) {
				QMeshPatch* topLayer = (QMeshPatch*)polygenMesh_Slices->GetMeshList().GetNext(Pos); // order: get data -> pnt move

				if (topLayer->GetIndexNo() < GcodeGeneRange_From || topLayer->GetIndexNo() > GcodeGeneRange_To) continue;

				if (topLayer->GetIndexNo() % Core != omptime) continue;

				vector<QMeshPatch*> bottomLayers;
				int layerNum = 30; //the number of detected bottom layer

				bottomLayers.push_back(patch_PrintPlatform);
				// construct a bottomLayers[i] to store the point of bottom layers for every toplayer
				for (GLKPOSITION beforePos = polygenMesh_Slices->GetMeshList().Find(topLayer)->prev; beforePos;) {
					QMeshPatch* beforePatch = (QMeshPatch*)polygenMesh_Slices->GetMeshList().GetPrev(beforePos);

					bottomLayers.push_back(beforePatch);
					if (bottomLayers.size() > layerNum) break;
					//std::cout << "current bottomLayers Index: " << beforePatch->GetIndexNo() << std::endl;
					//std::cout << "current topLayers Index: " << topLayer->GetIndexNo() << std::endl;
				}
				//std::cout << "------bottomLayers all size: " << bottomLayers.size() << std::endl << std::endl;


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
				}//--build PQP model END

				double pp[3];
				int layerIndex = topLayer->GetIndexNo();

				GLKPOSITION WayPointPatch_Pos = polygenMesh_Waypoints->GetMeshList().FindIndex(layerIndex);
				QMeshPatch* WayPointPatch = (QMeshPatch*)polygenMesh_Waypoints->GetMeshList().GetAt(WayPointPatch_Pos);

				//GLKPOSITION topLayerPatch_Pos = polygenMesh_Waypoints->GetMeshList().FindIndex(layerIndex);
				//QMeshPatch* WayPointPatch = (QMeshPatch*)polygenMesh_Waypoints->GetMeshList().GetAt(topLayerPatch_Pos);

				for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
					QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

					pp[0] = Node->m_printPostion[0];	pp[1] = Node->m_printPostion[1];	pp[2] = Node->m_printPostion[2];

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
					// cout << minDist << endl;
					Node->m_layerHeight = minDist;
				}

				//	free memory
				for (int i = 0; i < bottomLayers.size(); i++) { delete bLayerPQP[i]; }

			}
		}
	}

	// resume platform after LayerHeight calculation, if need
	if (upPlate2Height_switch == true) {
		for (GLKPOSITION Pos = patch_PrintPlatform->GetNodeList().GetHeadPosition(); Pos;) {
			QMeshNode* Node = (QMeshNode*)patch_PrintPlatform->GetNodeList().GetNext(Pos);
			double pp[3];
			Node->GetCoord3D(pp[0], pp[1], pp[2]);
			pp[0] -= Xmove;
			pp[1] -= Ymove;
			pp[2] -= upZdist;//-
			Node->SetCoord3D(pp[0], pp[1], pp[2]);
		}
	}
	printf(" TIMER -- Layer Height Calculation takes %ld ms.\n", clock() - time);
	cout << "------------------------------------------- Layer Height Calculation Finish!\n" << endl;
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

	
}

void fiveAxisPoint::singularityOpt(PolygenMesh* polygenMesh_Waypoints, int GcodeGeneRange_From, int GcodeGeneRange_To) {

	std::cout << "------------------------------------------- XYZBCD Calculation running ... " << std::endl;
	long time = clock();

	int Core = 12;
#pragma omp parallel
	{
#pragma omp for  
		for (int omptime = 0; omptime < Core; omptime++) {

			for (GLKPOSITION Pos = polygenMesh_Waypoints->GetMeshList().GetHeadPosition(); Pos;) {
				QMeshPatch* WayPointPatch = (QMeshPatch*)polygenMesh_Waypoints->GetMeshList().GetNext(Pos);

				if (WayPointPatch->GetIndexNo() < GcodeGeneRange_From || WayPointPatch->GetIndexNo() > GcodeGeneRange_To) continue;

				if (WayPointPatch->GetIndexNo() % Core != omptime) continue;

				getD(WayPointPatch);
				normalSmooth(WayPointPatch, 10, true);

				double lambda = 7;
				getRawCdata(WayPointPatch, lambda);

				//std::vector<QMeshPatch*> layerPatchSet;
				// 
				//int setIndex = 0;
				//for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
				//	QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);
				//	if (Node->m_largeJumpFlag == -1) setIndex++;							
				//	layerPatchSet[setIndex]->GetNodeList().AddTail(Node);
				//}
				//for (int i = 0; i < layerPatchSet.size(); i++) {
				//	MatrixXf sectionTable;
				//	getDangerSec(layerPatchSet[i], sectionTable);

				//	// ------------------ Method 1 ------------------ //
				//	//getNewPntNor(WayPointPatch, sectionTable, lambda, true);// true -> new singularity method breakSigSmoothPath
				//	//getXYZBCE(WayPointPatch, 0);
				//	//optimizationC(WayPointPatch);

				//	// ------------------ Method 2 ------------------ //
				//	MatrixXf B1C1table, B2C2table;
				//	getBC2(layerPatchSet[i], 0, B1C1table, B2C2table);
				//	motionPlanning(layerPatchSet[i], sectionTable, B1C1table, B2C2table);
				//	getXYZ(layerPatchSet[i]);
				//	optimizationC(layerPatchSet[i]);
				//}

				MatrixXf sectionTable;
				getDangerSec(WayPointPatch, sectionTable);

				// ------------------ Method 1 ------------------ //
				//getNewPntNor(WayPointPatch, sectionTable, lambda, true);// true -> new singularity method breakSigSmoothPath
				//getXYZBCE(WayPointPatch, 0);
				//optimizationC(WayPointPatch);

				// ------------------ Method 2 ------------------ //
				MatrixXf B1C1table, B2C2table;
				getBC2(WayPointPatch, 0, B1C1table, B2C2table);
				motionPlanning(WayPointPatch, sectionTable, B1C1table, B2C2table);
				getXYZ(WayPointPatch);
				optimizationC(WayPointPatch);

			}
		}
	}
	printf(" TIMER -- XYZBCD Calculation takes %ld ms.\n", clock() - time);
	std::cout << "------------------------------------------- XYZBCD Calculation Finish!\n " << std::endl;
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

void fiveAxisPoint::getD(QMeshPatch* WayPointPatch) {

	double Initial_large_Length = 4.0;
	double Support_larger_Length = 8.0;
	double largeLength = 0.0;

	if (WayPointPatch->isSupportLayer) { largeLength = Support_larger_Length; }
	else { largeLength = Initial_large_Length; }

	for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

		double D = 0;
		int lines = WayPointPatch->GetNodeNumber();
		if (Node->GetIndexNo() == (lines - 1)) { D = 0; }
		else {
			double Px = Node->m_printPostion[0];	double Py = Node->m_printPostion[1];	double Pz = Node->m_printPostion[2];

			GLKPOSITION nextPos = WayPointPatch->GetNodeList().Find(Node)->next;
			QMeshNode* nextNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(nextPos);

			double Px_next = nextNode->m_printPostion[0];	double Py_next = nextNode->m_printPostion[1];	double Pz_next = nextNode->m_printPostion[2];

			// calculate the length of two pnts 
			D = (Px - Px_next) * (Px - Px_next) + (Py - Py_next) * (Py - Py_next) + (Pz - Pz_next) * (Pz - Pz_next);
			D = sqrt(D);
			// increase the extrusion of first point
			if (Node->GetIndexNo() == 0) { D = D + 0.75; }

			if (D > largeLength) {
				D = 0.0;
				Node->m_largeJumpFlag = 1;		// end of prev section
				nextNode->m_largeJumpFlag = -1;	// start of next section
			}
		}
		Node->m_XYZBCE[5] = D;
		// cout << "D: " << D << "flag" << Node->m_largeJumpFlag << endl;
	}
}

void fiveAxisPoint::normalSmooth(QMeshPatch* WayPointPatch, int loop, bool smoothSwitch) {
	if (smoothSwitch == true) {
		double alpha = 0.5;
		int lines = WayPointPatch->GetNodeNumber();
		for (int smoothLoop = 0; smoothLoop < loop; smoothLoop++) {

			for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
				QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

				double Nx_prev, Ny_prev, Nz_prev;
				double Nx, Ny, Nz;
				double Nx_next, Ny_next, Nz_next;

				Nx = Node->m_printNormal[0];	Ny = Node->m_printNormal[1];	Nz = Node->m_printNormal[2];
				// mark the first line or the start point of one segment
				if ((Node->GetIndexNo() == 0) || (Node->m_largeJumpFlag == -1)) {

					Nx_prev = Nx;	Ny_prev = Ny;	Nz_prev = Nz;

					GLKPOSITION nextPos = WayPointPatch->GetNodeList().Find(Node)->next;
					QMeshNode* nextNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(nextPos);

					Nx_next = nextNode->m_printNormal[0];	Ny_next = nextNode->m_printNormal[1];	Nz_next = nextNode->m_printNormal[2];
				}
				// mark the last line or the end point of one segment
				else if ((Node->GetIndexNo() == (lines - 1)) || (Node->m_largeJumpFlag == 1)) {

					GLKPOSITION prevPos = WayPointPatch->GetNodeList().Find(Node)->prev;
					QMeshNode* prevNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(prevPos);

					Nx_prev = prevNode->m_printNormal[0];	Ny_prev = prevNode->m_printNormal[1];	Nz_prev = prevNode->m_printNormal[2];

					Nx_next = Nx;	Ny_next = Ny;	Nz_next = Nz;
				}
				else {
					GLKPOSITION prevPos = WayPointPatch->GetNodeList().Find(Node)->prev;
					QMeshNode* prevNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(prevPos);

					Nx_prev = prevNode->m_printNormal[0];	Ny_prev = prevNode->m_printNormal[1];	Nz_prev = prevNode->m_printNormal[2];

					GLKPOSITION nextPos = WayPointPatch->GetNodeList().Find(Node)->next;
					QMeshNode* nextNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(nextPos);

					Nx_next = nextNode->m_printNormal[0];	Ny_next = nextNode->m_printNormal[1];	Nz_next = nextNode->m_printNormal[2];
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
		double radLambda = DEGREE_TO_ROTATE(lambda);
		int safeFlag = 0;

		if (R < tan(radLambda)) {
			safeFlag = -1;
			Node->isSingularNode = true;
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

void fiveAxisPoint::getBC2(QMeshPatch* WayPointPatch, int layersNoSolve, MatrixXf& B1C1table, MatrixXf& B2C2table) {

	int lines = WayPointPatch->GetNodeNumber();
	//B1C1table.resize(lines, 2);	B2C2table.resize(lines, 2);
	B1C1table = Eigen::MatrixXf::Zero(lines, 2);	B2C2table = Eigen::MatrixXf::Zero(lines, 2);

	for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

		int i = Node->GetIndexNo();
		double Nx = Node->m_printNormal[0];		double Ny = Node->m_printNormal[1];		double Nz = Node->m_printNormal[2];

		// When layersNoSolve = 0, there will be no special actions.
		if (WayPointPatch->GetIndexNo() < layersNoSolve) { Nx = 0; Ny = 0; Nz = 1; }
		// solve 1
		B1C1table(i, 0) = ROTATE_TO_DEGREE(-acos(Nz)); // B1 deg
		B1C1table(i, 1) = ROTATE_TO_DEGREE(-atan2(Ny, Nx)); // C1 deg
		// solve 2
		B2C2table(i, 0) = ROTATE_TO_DEGREE(acos(Nz)); // B2 deg
		B2C2table(i, 1) = ROTATE_TO_DEGREE(-atan2(Ny, Nx)) + 180; // C2 deg

		//cout << Node->m_Bsolve[0] << " " << Node->m_Csolve[0] << " " << Node->m_Bsolve[1] << " " << Node->m_Csolve[1] << endl;
		//cout << ROTATE_TO_DEGREE(atan2(1, 1) )<< " " << ROTATE_TO_DEGREE(atan2(1, -1)) << " " << ROTATE_TO_DEGREE(atan2(-1, -1) )<< " " << ROTATE_TO_DEGREE(atan2(-1, 1) )<< endl;
	}
}

void fiveAxisPoint::motionPlanning(QMeshPatch* WayPointPatch, const MatrixXf& sectionTable, const MatrixXf& B1C1table, const MatrixXf& B2C2table) {

	int lines = WayPointPatch->GetNodeNumber();
	Eigen::MatrixXd BC_Matrix(lines, 3);
	int sectionNumber = 0;
	int sectionAmount = sectionTable.size() / 2;
	int lastSelect = 1;
	int i = 0;

	while (i < lines) {
		//all points of current path are OUT of the sigularity region
		if (sectionAmount == 0) {
			double B1 = B1C1table(i, 0);	double C1 = B1C1table(i, 1);
			double B2 = B2C2table(i, 0);	double C2 = B2C2table(i, 1);
			if (i == 0) {
				if (C2 > 180.0) C2 = C2 - 360.0;

				if ((abs(B1) + abs(C1)) > (abs(B2) + abs(C2))) {
					BC_Matrix.row(i) << B2, C2, 2;
				}
				else {
					BC_Matrix.row(i) << B1, C1, 1;
				}
			}
			else {
				/**********************************************/

				GLKPOSITION jumpPos1 = WayPointPatch->GetNodeList().FindIndex(i);
				QMeshNode* jumpNode1 = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(jumpPos1);
				// large Jump Point
				if (jumpNode1->m_largeJumpFlag == -1) {

					int anotherSelect = (lastSelect % 2) + 1;

					double nowC, absCtemp;
					double prevC = BC_Matrix(i - 1, 1);

					if (lastSelect == 1) { nowC = C1; }
					else if (lastSelect == 2) { nowC = C2; }
					else {}

					if (abs(nowC - prevC) > 180.0) { absCtemp = 360.0 - abs(nowC - prevC); }
					else { absCtemp = abs(nowC - prevC); }

					if (absCtemp > 90.0) {
						if (anotherSelect == 1) { BC_Matrix.row(i) << B1, C1, 1; }
						else if (anotherSelect == 2) { BC_Matrix.row(i) << B2, C2, 2; }
						else { cout << "anotherSelect error " << endl; }
					}
					else {
						if (lastSelect == 1) { BC_Matrix.row(i) << B1, C1, 1; }
						else if (lastSelect == 2) { BC_Matrix.row(i) << B2, C2, 2; }
						else { cout << "lastSelect error " << endl; }
					}
				}
				else {
					if (lastSelect == 1) { BC_Matrix.row(i) << B1, C1, 1; }
					else if (lastSelect == 2) { BC_Matrix.row(i) << B2, C2, 2; }
					else { cout << "lastSelect error " << endl; }
				}

				/**********************************************/
				/*if (lastSelect == 1) { BC_Matrix.row(i) << B1, C1, 1; }
				else if (lastSelect == 2) { BC_Matrix.row(i) << B2, C2, 2; }
				else { cout << "lastSelect error " << endl; }*/
				/**********************************************/
			}
			lastSelect = (int)BC_Matrix(i, 2);
			i = i + 1;
		}
		else {
			int anotherSelect;
			//all points of current path are IN the sigularity region
			if (i == sectionTable(sectionNumber, 0) && i == 0 && sectionTable(sectionNumber, 1) == (lines - 1)) {
				for (int secLineIndex = i; secLineIndex < lines; secLineIndex++) {
					BC_Matrix.row(secLineIndex) << 0.0, 0.0, 0;
				}
				i = lines;
			}
			// start from the singularity region / end in singularity region or not 
			else if (i == sectionTable(sectionNumber, 0) && i == 0 && sectionTable(sectionNumber, 1) != (lines - 1)) {

				int secEndIndex = sectionTable(sectionNumber, 1);
				double endPntB1 = B1C1table(secEndIndex, 0);	double endPntC1 = B1C1table(secEndIndex, 1);
				double endPntB2 = B2C2table(secEndIndex, 0);	double endPntC2 = B2C2table(secEndIndex, 1);

				for (int secLineIndex = i; secLineIndex < secEndIndex; secLineIndex++) {


					if (endPntC2 > 180.0) endPntC2 = endPntC2 - 360.0;

					if ((abs(endPntB1) + abs(endPntC1)) > (abs(endPntB2) + abs(endPntC2))) {
						BC_Matrix.row(secLineIndex) << endPntB2, endPntC2, 2;
					}
					else {
						BC_Matrix.row(secLineIndex) << endPntB1, endPntC1, 1;
					}

					//cout << secLineIndex <<" "<<BC_Matrix(secLineIndex, 0) << " " << BC_Matrix(secLineIndex, 1) << " " << BC_Matrix(secLineIndex, 2) << endl;
				}
				//cout << BC_Matrix << endl;
				lastSelect = (int)BC_Matrix((secEndIndex - 1), 2);
				i = secEndIndex;
				if (sectionNumber != (sectionAmount - 1))	sectionNumber = sectionNumber + 1;
			}
			// end in the singularity region / finish path
			else if (i == sectionTable(sectionNumber, 0) && i != 0 && sectionTable(sectionNumber, 1) == (lines - 1)) {

				int secStartIndex = sectionTable(sectionNumber, 0);
				double startPntB1 = B1C1table(secStartIndex, 0);	double startPntC1 = B1C1table(secStartIndex, 1);
				double startPntB2 = B2C2table(secStartIndex, 0);	double startPntC2 = B2C2table(secStartIndex, 1);

				for (int secLineIndex = i; secLineIndex < lines; secLineIndex++) {
					if (lastSelect == 1) {
						BC_Matrix.row(secLineIndex) << startPntB1, startPntC1, 1;
					}
					else if (lastSelect == 2) {
						BC_Matrix.row(secLineIndex) << startPntB2, startPntC2, 2;
					}
				}
				i = lines;
			}
			// path passes through the sigularity region
			else if (i == sectionTable(sectionNumber, 0) && i != 0 && sectionTable(sectionNumber, 1) != (lines - 1)) {

				//double startPnt_B1_temp, startPnt_C1_temp, startPnt_B2_temp, startPnt_C2_temp;
				//double endPnt_B1_temp, endPnt_C1_temp, endPnt_B2_temp, endPnt_C2_temp;

				// start point message
				int secStartIndex = sectionTable(sectionNumber, 0);
				double startPntB1 = B1C1table(secStartIndex, 0);	double startPntC1 = B1C1table(secStartIndex, 1);
				double startPntB2 = B2C2table(secStartIndex, 0);	double startPntC2 = B2C2table(secStartIndex, 1);
				// end point message
				int secEndIndex = sectionTable(sectionNumber, 1);
				double endPntB1 = B1C1table(secEndIndex, 0);	double endPntC1 = B1C1table(secEndIndex, 1);
				double endPntB2 = B2C2table(secEndIndex, 0);	double endPntC2 = B2C2table(secEndIndex, 1);



				GLKPOSITION jumpPos3 = WayPointPatch->GetNodeList().FindIndex(secEndIndex);
				QMeshNode* jumpNode3 = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(jumpPos3);
				// large Jump Point
				if (jumpNode3->m_largeJumpFlag == -1) {
					endPntB1 = startPntB1; endPntC1 = startPntC1;
					endPntB2 = startPntB2; endPntC2 = startPntC2;
				}

				GLKPOSITION jumpPos4 = WayPointPatch->GetNodeList().FindIndex(secStartIndex);
				QMeshNode* jumpNode4 = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(jumpPos4);
				// large Jump Point
				if (jumpNode4->m_largeJumpFlag == 1) {
					startPntB1 = endPntB1; startPntC1 = endPntC1;
					startPntB2 = endPntB2; startPntC2 = endPntC2;
				}

				double angle = 0.0;
				if (lastSelect == 1) { angle = abs(startPntC1 - endPntC1); }
				else if (lastSelect == 2) { angle = abs(startPntC2 - endPntC2); }
				else { cout << "lastSelect error " << lastSelect << endl; }

				int pointNum = sectionTable(sectionNumber, 1) - sectionTable(sectionNumber, 0);
				anotherSelect = (lastSelect % 2) + 1;
				// decide which arc to choose(< 180)
				if (angle > 180.0)	angle = 360.0 - angle;

				double Cangle = 0.0, Bangle = 0.0;
				// whether flip / yes
				if (angle > 90.0) {
					if (lastSelect == 1) { Cangle = endPntC2 - startPntC1;		Bangle = endPntB2 - startPntB1; }
					else if (lastSelect == 2) { Cangle = endPntC1 - startPntC2;		Bangle = endPntB1 - startPntB2; }
					else { cout << "lastSelect error " << lastSelect << endl; }

					if (abs(Cangle) > 180.0) {
						if (Cangle > 0.0) { Cangle = Cangle - 360; }
						else { Cangle = Cangle + 360; }
					}

					unsigned int times = 0;
					int select_keep = lastSelect;

					for (int secLineIndex = secStartIndex; secLineIndex < secEndIndex; secLineIndex++) {
						if (select_keep == 2) {
							BC_Matrix(secLineIndex, 0) = startPntB2 + times * Bangle / pointNum;
							BC_Matrix(secLineIndex, 1) = startPntC2 + times * Cangle / pointNum;
						}
						else if (select_keep == 1) {
							BC_Matrix(secLineIndex, 0) = startPntB1 + times * Bangle / pointNum;
							BC_Matrix(secLineIndex, 1) = startPntC1 + times * Cangle / pointNum;
						}
						else { cout << "select_keep error " << select_keep << endl; }

						times = times + 1;
						BC_Matrix(secLineIndex, 2) = lastSelect;

						if (secLineIndex == secEndIndex - 1) {
							BC_Matrix(secLineIndex, 2) = anotherSelect;
							lastSelect = anotherSelect;
							anotherSelect = (lastSelect % 2) + 1;
						}
					}

					if (jumpNode4->m_largeJumpFlag == 1) {

						if (select_keep == 2)	BC_Matrix.row(secStartIndex) << B2C2table(secStartIndex, 0), B2C2table(secStartIndex, 1), 2;
						if (select_keep == 1)	BC_Matrix.row(secStartIndex) << B1C1table(secStartIndex, 0), B1C1table(secStartIndex, 1), 1;

					}


				}
				// whether flip / no
				else {

					if (lastSelect == 1) { Cangle = endPntC1 - startPntC1;		Bangle = endPntB1 - startPntB1; }
					else if (lastSelect == 2) { Cangle = endPntC2 - startPntC2;		Bangle = endPntB2 - startPntB2; }
					else { cout << "lastSelect error " << lastSelect << endl; }

					if (abs(Cangle) > 180.0) {
						if (Cangle > 0.0) Cangle = Cangle - 360;
						else Cangle = Cangle + 360;
					}

					unsigned int times = 0;

					for (int secLineIndex = secStartIndex; secLineIndex < secEndIndex; secLineIndex++) {
						if (lastSelect == 2) {
							BC_Matrix(secLineIndex, 0) = startPntB2 + times * Bangle / pointNum;
							BC_Matrix(secLineIndex, 1) = startPntC2 + times * Cangle / pointNum;
							BC_Matrix(secLineIndex, 2) = lastSelect;
						}
						else if (lastSelect == 1) {
							BC_Matrix(secLineIndex, 0) = startPntB1 + times * Bangle / pointNum;
							BC_Matrix(secLineIndex, 1) = startPntC1 + times * Cangle / pointNum;
							BC_Matrix(secLineIndex, 2) = lastSelect;
						}
						else { cout << "lastSelect error " << lastSelect << endl; }

						times = times + 1;
					}

					if (jumpNode4->m_largeJumpFlag == 1) {

						if (lastSelect == 2)	BC_Matrix.row(secStartIndex) << B2C2table(secStartIndex, 0), B2C2table(secStartIndex, 1), 2;
						if (lastSelect == 1)	BC_Matrix.row(secStartIndex) << B1C1table(secStartIndex, 0), B1C1table(secStartIndex, 1), 1;

					}

				}

				lastSelect = (int)BC_Matrix((secEndIndex - 1), 2);
				i = secEndIndex;
				if (sectionNumber != (sectionAmount - 1))	sectionNumber = sectionNumber + 1;

			}
			// other points out of the singularity region
			else {

				double B1 = B1C1table(i, 0);	double C1 = B1C1table(i, 1);
				double B2 = B2C2table(i, 0);	double C2 = B2C2table(i, 1);
				if (i == 0) {

					if (C2 > 180.0) C2 = C2 - 360.0;

					if ((abs(C1) + abs(B1)) > (abs(C2) + abs(B2))) {
						BC_Matrix.row(i) << B2, C2, 2;
						lastSelect = 2;
					}
					else {
						BC_Matrix.row(i) << B1, C1, 1;
						lastSelect = 1;
					}


				}
				else {
					/**************************************/


					GLKPOSITION jumpPos2 = WayPointPatch->GetNodeList().FindIndex(i);
					QMeshNode* jumpNode2 = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(jumpPos2);
					// large Jump Point
					if (jumpNode2->m_largeJumpFlag == -1) {

						anotherSelect = (lastSelect % 2) + 1;

						double nowC, absCtemp;
						double prevC = BC_Matrix(i - 1, 1);

						if (lastSelect == 1) { nowC = C1; }
						else if (lastSelect == 2) { nowC = C2; }
						else { cout << "lastSelect error " << lastSelect << endl; }

						if (abs(nowC - prevC) > 180.0) { absCtemp = 360.0 - abs(nowC - prevC); }
						else { absCtemp = abs(nowC - prevC); }
						// whether flip / yes
						if (absCtemp > 90.0) {
							if (anotherSelect == 1) { BC_Matrix.row(i) << B1, C1, 1; }
							else if (anotherSelect == 2) { BC_Matrix.row(i) << B2, C2, 2; }
							else { cout << "anotherSelect error " << anotherSelect << endl; }
							lastSelect = anotherSelect;
						}
						// whether flip / no
						else {
							if (lastSelect == 1) { BC_Matrix.row(i) << B1, C1, 1; }
							else if (lastSelect == 2) { BC_Matrix.row(i) << B2, C2, 2; }
							else { cout << "lastSelect error " << lastSelect << endl; }
						}
					}
					else {
						if (lastSelect == 1) { BC_Matrix.row(i) << B1, C1, 1; }
						else if (lastSelect == 2) { BC_Matrix.row(i) << B2, C2, 2; }
						else { cout << "lastSelect error " << lastSelect << endl; }
					}

					/*****************************/
					/*if (lastSelect == 1) {BC_Matrix.row(i) << B1C1table(i, 0), B1C1table(i, 1), 1;}
					else if (lastSelect == 2) {BC_Matrix.row(i) << B2C2table(i, 0), B2C2table(i, 1), 2;}
					else{cout << "lastSelect error " << endl;}*/
					/**************************************/
				}

				i = i + 1;
			}
		}
	}

	for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

		int nodeIndex = Node->GetIndexNo();
		Node->m_XYZBCE[3] = BC_Matrix(nodeIndex, 0); //deg
		Node->m_XYZBCE[4] = BC_Matrix(nodeIndex, 1); //deg

	}
}

void fiveAxisPoint::getXYZ(QMeshPatch* WayPointPatch) {
	for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

		double Px = Node->m_printPostion[0];
		double Py = Node->m_printPostion[1];
		double Pz = Node->m_printPostion[2];

		double B = DEGREE_TO_ROTATE(Node->m_XYZBCE[3]);// rad
		double C = DEGREE_TO_ROTATE(Node->m_XYZBCE[4]);// rad

		Node->m_XYZBCE[0] = cos(B) * cos(C) * Px - cos(B) * sin(C) * Py + sin(B) * Pz;
		Node->m_XYZBCE[1] = sin(C) * Px + cos(C) * Py;
		Node->m_XYZBCE[2] = -sin(B) * cos(C) * Px + sin(B) * sin(C) * Py + cos(B) * Pz;

		if (Node->m_XYZBCE[2] < 1.0) { Node->negativeZ = true; }

	}
}

void fiveAxisPoint::optimizationC(QMeshPatch* WayPointPatch) {

	for (int loop = 0; loop < 20; loop++) {

		double threshhold = 180.0;

		for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
			QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

			double C = Node->m_XYZBCE[4]; // deg

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

void fiveAxisPoint::writeGcode(PolygenMesh* polygenMesh_Waypoints, string rltDir, int GcodeGeneRange_From, int GcodeGeneRange_To, double E3_xOff, double E3_yOff) {

	std::cout << "------------------------------------------- " << rltDir << " Gcode Writing ..." << std::endl;

	// Define the basic parameter
	double Z_home = 230;						// The hight of Home point; / mm
	double Z_high = 70;							// The hight of G1 point(for safety); / mm
	double Z_compensateUpDistance = 2;			// The hight of waiting distance of extruder; / mm

	int sExtruder = 3;							// Support extruder num
	int iExtruder = 1;							// Initial extruder num

	int iTemperature = 200;						// The temperature of Extruder Initial
	int sTemperature = 200;						// The temperature of Extruder Support

	int F_G0_XYBC = 5250;						// Speed of G0 move of XYBC
	int F_G0_Z = 4750;							// Speed of G0 move of Z
	int F_G1_support = 1500;					// Speed of G1 support material (normal 2ed~layers)
	int F_G1_original = 1500;					// Speed of G1 original material (normal 2ed~layers)
	int F_G1_1stlayer = 1500;					// Speed of G1(special 1st layer)
	int F_PumpBack = 5000;						// Speed of F_PumpBack
	int F_PumpCompensate = 4750;				// Speed of PumpCompensate

	double E_PumpBack = -12;					// The extruder pump back Xmm
	double E_PumpCompensate = 11;				// The extruder pump compensate Xmm
	double E_PumpCompensateL1 = 15;				// The extruder pump compensate for 1st layer Xmm
	double E_PumpCompensateNewE = 12;			// The extruder pump compensate for new type layer Xmm

	// important parameter for extruder 3
	double xExtuderOffset = E3_xOff;
	double yExtuderOffset = E3_yOff;

	char targetFilename[1024];
	std::sprintf(targetFilename, "%s%s", "../2_GenratedGcode/", rltDir.c_str());
	FILE* fp = fopen(targetFilename, "w");
	if (!fp) {
		perror("Couldn't open the directory");
		return;
	}

	// Record the max Z for security consideration (1st printed layer)
	GLKPOSITION layer1st_Pos = polygenMesh_Waypoints->GetMeshList().FindIndex(GcodeGeneRange_From);
	QMeshPatch* layer1st_WayPointPatch = (QMeshPatch*)polygenMesh_Waypoints->GetMeshList().GetAt(layer1st_Pos);

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
		std::fprintf(fp, "M104 S200 T2\n");
		std::fprintf(fp, "M104 S%d T%d\n", sTemperature, sExtruder);
		std::fprintf(fp, "G28 X0.000 Y0.000 Z%.3f B0.000 C0.000 F%d\n", Z_home, F_G0_XYBC);
		std::fprintf(fp, "M109 S%d T%d\n", sTemperature, sExtruder);
	}
	else {
		std::fprintf(fp, "G90\n");
		std::fprintf(fp, "M104 S200 T2\n");
		std::fprintf(fp, "M104 S%d T%d\n", iTemperature, iExtruder);
		std::fprintf(fp, "G28 X0.000 Y0.000 Z%.3f B0.000 C0.000 F%d\n", Z_home, F_G0_XYBC);
		std::fprintf(fp, "M109 S%d T%d\n", iTemperature, iExtruder);
	}


	for (GLKPOSITION Pos = polygenMesh_Waypoints->GetMeshList().GetHeadPosition(); Pos;) {
		QMeshPatch* WayPointPatch = (QMeshPatch*)polygenMesh_Waypoints->GetMeshList().GetNext(Pos);

		if (WayPointPatch->GetIndexNo() < GcodeGeneRange_From || WayPointPatch->GetIndexNo() > GcodeGeneRange_To) continue;

		for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
			QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);
			double X = Node->m_XYZBCE[0]; double Y = Node->m_XYZBCE[1]; double Z = Node->m_XYZBCE[2];
			double B = Node->m_XYZBCE[3]; double C = Node->m_XYZBCE[4]; double E = Node->m_XYZBCE[5];

			// check the huge change of C angle
			if (Node->GetIndexNo() != 0)
			{
				GLKPOSITION prevPos = WayPointPatch->GetNodeList().Find(Node)->prev;
				QMeshNode* prevNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(prevPos);

				double C_prev = prevNode->m_XYZBCE[4];

				if (abs(C - C_prev) > 300
					&& prevNode->m_largeJumpFlag != 1 && Node->m_largeJumpFlag != -1
					&& Node->isCollisionEnd == false && Node->isCollisionStart == false
					&& Node->isNegZStart == false && Node->isNegZEnd == false) {

					Node->isHighLight = true;
					cerr << "abs(C - C_prev) ERROR! " << abs(C - C_prev) << std::endl;
					cout << prevNode->m_largeJumpFlag << Node->m_largeJumpFlag << Node->isCollisionEnd << prevNode->isCollisionStart << endl;
					cout << "WayPointPatch->GetIndexNo() " << WayPointPatch->GetIndexNo() << endl;
				}

			}
			// check the negative Z motion
			if (Z < 0.0) { cout << "Z sitll < 0.0" << endl; }
			// increase the extrusion of first layer
			if (WayPointPatch->GetIndexNo() == GcodeGeneRange_From) { E = E * 1.1; }//?

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
					std::fprintf(fp, "G0 Z%.3f F%d\n", (Z_max + Z_compensateUpDistance), F_G0_Z);
					// zero extruded length(set E axis to 0)
					std::fprintf(fp, "G92 E0\n");
					std::fprintf(fp, "G1 E%.3f F%d\n", E_PumpCompensateL1, F_PumpCompensate);
					std::fprintf(fp, "G92 E0\n");
					std::fprintf(fp, "G1 F%d\n", F_G1_1stlayer);
				}
				else if (WayPointPatch->isSupportLayer == IsSupportLayer_last) {
					std::fprintf(fp, "G92 E0\n");
					std::fprintf(fp, "G0 E%.3f F%d\n", E_PumpBack, F_PumpBack);
					std::fprintf(fp, "G92 E0\n");
					// return to the safe point Z_max + Z_high
					std::fprintf(fp, "G0 Z%.3f F%d\n", (Z_max + Z_high), F_G0_Z);
					// move to start of printing location
					std::fprintf(fp, "G0 X%.3f Y%.3f B%.3f C%.3f F%d\n", X, Y, B, C, F_G0_XYBC);
					// slowly lower for printing
					std::fprintf(fp, "G0 Z%.3f F%d\n", (Z + Z_compensateUpDistance), F_G0_Z);

					if (WayPointPatch->isSupportLayer == true) {
						std::fprintf(fp, "G1 E%.3f F%d\n", E_PumpCompensate * 1.2, F_PumpCompensate);
						std::fprintf(fp, "G92 E0\n");
						std::fprintf(fp, "G1 F%d\n", F_G1_support);
					}
					else {
						std::fprintf(fp, "G1 E%.3f F%d\n", E_PumpCompensate, F_PumpCompensate);
						std::fprintf(fp, "G92 E0\n");
						std::fprintf(fp, "G1 F%d\n", F_G1_original);
					}

				}
				// the special case: exchange extruder
				else {
					std::fprintf(fp, "G92 E0\n");
					std::fprintf(fp, "G0 E%.3f F%d\n", E_PumpBack, F_PumpBack);
					// return to the home point Z_home
					std::fprintf(fp, "G0 Z%.3f F%d\n", Z_home, F_G0_Z);

					// change extruder
					if (WayPointPatch->isSupportLayer == true)
					{
						std::fprintf(fp, "M104 S%d T%d\n", sTemperature, sExtruder);
					}
					else { std::fprintf(fp, "M104 S%d T%d\n", iTemperature, iExtruder); }
					std::fprintf(fp, "G92 E0\n");

					// move to start of printing location
					std::fprintf(fp, "G0 X%.3f Y%.3f B%.3f C%.3f F%d\n", X, Y, B, C, F_G0_XYBC);
					std::fprintf(fp, "G0 Z%.3f F%d\n", (Z + Z_compensateUpDistance), F_G0_Z);


					if (WayPointPatch->isSupportLayer == true) {
						// slowly lower for printing
						std::fprintf(fp, "G1 E%.3f F%d\n", E_PumpCompensateNewE * 1.2, F_PumpCompensate);
						std::fprintf(fp, "G92 E0\n");
						std::fprintf(fp, "G1 F%d\n", F_G1_support);
					}
					else {
						// slowly lower for printing
						std::fprintf(fp, "G1 E%.3f F%d\n", E_PumpCompensateNewE, F_PumpCompensate);
						std::fprintf(fp, "G92 E0\n");
						std::fprintf(fp, "G1 F%d\n", F_G1_original);
					}
				}
				std::fprintf(fp, "G1 X%.3f Y%.3f Z%.3f B%.3f C%.3f E%.3f\n", X, Y, Z, B, C, E);
			}
			else {
				// Consider the waypoints with too large Length OR large Singularity areas
				if (Node->m_largeJumpFlag == -1 || Node->isSingularSecEndNode || Node->isCollisionStart || Node->isCollisionEnd || Node->isNegZStart || Node->isNegZEnd) {

					//GLKPOSITION prevPos = WayPointPatch->GetNodeList().Find(Node)->prev;
					//QMeshNode* prevNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(prevPos);
					//double oldZ = prevNode->m_XYZBCE[2];
					//double odlE = prevNode->m_XYZBCE[5];

					//std::cout << Node->m_XYZBCE[5] << std::endl;
					//std::cout << "oldE = "<<odlE << std::endl;

					if (WayPointPatch->isSupportLayer == true) {
						//std::fprintf(fp, "G0 E%.3f F%d\n", (odlE + E_PumpBack * 1.95), F_PumpBack);
						//std::fprintf(fp, "G0 Z%.3f F%d\n", (max(Z_max, oldZ) + Z_high), F_G0_Z);
						std::fprintf(fp, "G0 E%.3f F%d\n", (E + E_PumpBack * 1.4), F_PumpBack);
						std::fprintf(fp, "G0 Z%.3f F%d\n", (Z_max + Z_high), F_G0_Z);
						std::fprintf(fp, "G0 X%.3f Y%.3f B%.3f C%.3f F%d\n", X, Y, B, C, F_G0_XYBC);
						std::fprintf(fp, "G0 Z%.3f F%d\n", (Z + Z_compensateUpDistance), F_G0_Z);
						std::fprintf(fp, "G0 E%.3f F%d\n", (E - 0.1), F_PumpCompensate);
						std::fprintf(fp, "G0 Z%.3f E%.3f F%d\n", Z, E, F_G1_support);
					}
					else {
						//std::fprintf(fp, "G0 E%.3f F%d\n", (odlE + E_PumpBack * 0.8), F_PumpBack);
						//std::fprintf(fp, "G0 Z%.3f F%d\n", (max(Z_max, oldZ) + Z_high), F_G0_Z);
						std::fprintf(fp, "G0 E%.3f F%d\n", (E + E_PumpBack * 0.8), F_PumpBack);
						std::fprintf(fp, "G0 Z%.3f F%d\n", (Z_max + Z_high), F_G0_Z);
						std::fprintf(fp, "G0 X%.3f Y%.3f B%.3f C%.3f F%d\n", X, Y, B, C, F_G0_XYBC);
						std::fprintf(fp, "G0 Z%.3f F%d\n", (Z + Z_compensateUpDistance), F_G0_Z);
						std::fprintf(fp, "G0 E%.3f F%d\n", (E - 0.1), F_PumpCompensate);
						std::fprintf(fp, "G0 Z%.3f E%.3f F%d\n", Z, E, F_G1_original);
					}

					//std::cout << "oldE = " << odlE << std::endl;

				}
				std::fprintf(fp, "G1 X%.3f Y%.3f Z%.3f B%.3f C%.3f E%.3f\n", X, Y, Z, B, C, E);
			}
			//std::cout << E << std::endl;
		}

		IsSupportLayer_last = WayPointPatch->isSupportLayer;
	}

	std::fprintf(fp, "G92 E0\n");
	std::fprintf(fp, "G0 E%.3f F%d\n", E_PumpBack, F_G0_XYBC); // PumpBack
	std::fprintf(fp, "G0 Z%.3f F%d\n", Z_home, F_G0_Z); // return to the home point Z_home
	std::fprintf(fp, "G0 X0 Y0 B0 C0 F%d\n", F_G0_XYBC);
	std::fprintf(fp, "M30\n");// Stop all of the motion

	std::fclose(fp);

	std::cout << "------------------------------------------- " << rltDir << " Gcode Write Finish!\n" << std::endl;
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

void fiveAxisPoint::height2E(PolygenMesh* polygenMesh_Waypoints, int GcodeGeneRange_From, int GcodeGeneRange_To, bool func_switch) {

	if (func_switch == true) {
		std::cout << "------------------------------------------- E of Vary-Height Calculation running ..." << std::endl;
	}
	else {
		std::cout << "------------------------------------------- E of Uniform-Height Calculation running ..." << std::endl;
	}

	// E = E + ratio * height * length * width;
	double ratio_initial = 0.55; // Dicided by CNC W.R.T(E:Volume:E = 0.48)
	double ratio_support = 0.5;
	double width = 0.6;

	for (GLKPOSITION Pos = polygenMesh_Waypoints->GetMeshList().GetHeadPosition(); Pos;) {
		QMeshPatch* WayPointPatch = (QMeshPatch*)polygenMesh_Waypoints->GetMeshList().GetNext(Pos);

		if (WayPointPatch->GetIndexNo() < GcodeGeneRange_From || WayPointPatch->GetIndexNo() > GcodeGeneRange_To) continue;

		double E = 0;

		for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
			QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

			double deltaE;
			double height;
			double length = Node->m_XYZBCE[5];

			if (func_switch == true) {
				height = Node->m_layerHeight;
				if (WayPointPatch->isSupportLayer) deltaE = ratio_support * height * length * width;
				else deltaE = ratio_initial * height * length * width;
			}
			else {
				deltaE = 0.25 * length;
			}

			E = E + deltaE;
			Node->m_XYZBCE[5] = E;
		}
		//std::cout << ".";
		//if (((WayPointPatch->GetIndexNo() + 1) % 100 == 0) ||
		//	((WayPointPatch->GetIndexNo() + 1) == polygenMesh_Waypoints->GetMeshList().GetCount())) {
		//	std::cout << std::endl;
		//}
	}

	if (func_switch == true) {
		std::cout << "------------------------------------------- E of Vary-Height Calculation Finish!\n" << std::endl;
	}
	else {
		std::cout << "------------------------------------------- E of Uniform-Height Calculation Finish!\n" << std::endl;
	}
}

void fiveAxisPoint::detectCollision(PolygenMesh* polygenMesh_Waypoints, PolygenMesh* polygenMesh_extruderHead, bool func_switch) {

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

void fiveAxisPoint::writeABBGcode(PolygenMesh* polygenMesh_Waypoints, string rltDir, int GcodeGeneRange_From, int GcodeGeneRange_To, double E3_xOff, double E3_yOff)
{
	std::cout << "------------------------------------------- " << rltDir << " Gcode Writing ..." << std::endl;

	// Define the basic parameter
	double Z_home = 230;						// The hight of Home point; / mm
	double Z_high = 70;							// The hight of G1 point(for safety); / mm
	double Z_compensateUpDistance = 2;			// The hight of waiting distance of extruder; / mm

	int sExtruder = 1;							// Support extruder num
	int iExtruder = 0;							// Initial extruder num

	int iTemperature = 200;						// The temperature of Extruder Initial
	int sTemperature = 200;						// The temperature of Extruder Support


	int V_Print = 20;                           //Speed of end-effector while printing initial model
	int V_Support = 15;
	double deltaDistance;
	double deltaE;
	double F_Print;



	int F_G0_XYBC = 1000;						// Speed of G0 move of XYBC
	int F_G0_Z = 1000;							// Speed of G0 move of Z
	int F_G1_support = 150;					// Speed of G1 support material (normal 2ed~layers)
	int F_G1_original = 150;					// Speed of G1 original material (normal 2ed~layers)
	int F_G1_1stlayer = 150;					// Speed of G1(special 1st layer)
	int F_PumpBack = 1000;						// Speed of F_PumpBack
	int F_PumpCompensate = 1000;				// Speed of PumpCompensate

	double E_PumpBack = -8;					// The extruder pump back Xmm
	double E_PumpCompensate = 7;				// The extruder pump compensate Xmm
	double E_PumpCompensateL1 = 11;				// The extruder pump compensate for 1st layer Xmm
	double E_PumpCompensateNewE = 8;			// The extruder pump compensate for new type layer Xmm

	Eigen::Matrix3f R1; R1 << 0, -1, 0, 1, 0, 0, 0, 0, 1;
	Eigen::Matrix3f RX0; RX0 << 1, 0, 0, 0, 1, 0, 0, 0, 1;
	Eigen::Matrix3f RY0; RY0 << 1, 0, 0, 0, 1, 0, 0, 0, 1;
	Eigen::Matrix3f RZ0; RZ0 << -1, 0, 0, 0, -1, 0, 0, 0, 1;
	Eigen::Matrix3f RX1; RX1 << 1, 0, 0, 0, cos(0.073), -sin(DEGREE_TO_ROTATE(0.073)), 0, sin(DEGREE_TO_ROTATE(0.073)), cos(DEGREE_TO_ROTATE(0.073));
	Eigen::Matrix3f RY1; RY1 << cos(DEGREE_TO_ROTATE(-0.1)), 0, sin(DEGREE_TO_ROTATE(0.1)), 0, 1, 0, -sin(DEGREE_TO_ROTATE(0.1)), 0, cos(DEGREE_TO_ROTATE(0.1));
	Eigen::Matrix3f RZ1; RZ1 << cos(DEGREE_TO_ROTATE(89.87)), -sin(DEGREE_TO_ROTATE(89.87)), 0, sin(DEGREE_TO_ROTATE(89.87)), cos(DEGREE_TO_ROTATE(89.87)), 0, 0, 0, 1;
	Eigen::Vector3f T1; T1 << 1459.306, -26.9, 2.591 + 2.07;


	// important parameter for extruder 3
	/*double xExtuderOffset = E3_xOff;
	double yExtuderOffset = E3_yOff;*/

	char targetFilename[1024];
	std::sprintf(targetFilename, "%s%s", "../4_ABBGcode/", rltDir.c_str());
	FILE* fp = fopen(targetFilename, "w");
	if (!fp) {
		perror("Couldn't open the directory");
		return;
	}

	// Record the max Z for security consideration (1st printed layer)
	GLKPOSITION layer1st_Pos = polygenMesh_Waypoints->GetMeshList().FindIndex(GcodeGeneRange_From);
	QMeshPatch* layer1st_WayPointPatch = (QMeshPatch*)polygenMesh_Waypoints->GetMeshList().GetAt(layer1st_Pos);

	double Z_max = -99999.9;
	for (GLKPOSITION Pos = layer1st_WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* Node = (QMeshNode*)layer1st_WayPointPatch->GetNodeList().GetNext(Pos);

		double Pz = Node->m_XYZBCE[2];
		if (Pz > Z_max) { Z_max = Pz; }
	}
	// Record the layer type of last Layer
	bool IsSupportLayer_last = layer1st_WayPointPatch->isSupportLayer;

	// Give the start message of G_code
	if (layer1st_WayPointPatch->isSupportLayer == true) {
		std::fprintf(fp, "T1 \n");
		std::fprintf(fp, "G92 E0 \n");
	}
	else {
		std::fprintf(fp, "T0 \n");
		std::fprintf(fp, "G92 E0 \n");
	}


	for (GLKPOSITION Pos = polygenMesh_Waypoints->GetMeshList().GetHeadPosition(); Pos;) {
		QMeshPatch* WayPointPatch = (QMeshPatch*)polygenMesh_Waypoints->GetMeshList().GetNext(Pos);

		if (WayPointPatch->GetIndexNo() < GcodeGeneRange_From || WayPointPatch->GetIndexNo() > GcodeGeneRange_To) continue;
		

		for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition() ; Pos;) {
			GLKPOSITION PosPrev = Pos;
			QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);
			double X = Node->m_XYZBCE[0]; double Y = Node->m_XYZBCE[1]; double Z = Node->m_XYZBCE[2];
			double B = Node->m_XYZBCE[3]; double C = Node->m_XYZBCE[4]; double E = Node->m_XYZBCE[5];
			//Node->GetIndexNo;
			//translate from CNC to ABB for IRB4600
			X = X - 100 * sin(DEGREE_TO_ROTATE(B)); Z = Z + 100 * (1 - cos(DEGREE_TO_ROTATE(B)));
			Eigen::Vector3f P1; P1 << X, Y, Z;
			//Eigen::Matrix3f R1; R1 << 0, -1, 0, 1, 0, 0, 0, 0, 1;
			Eigen::Vector3f P01; P01 = R1 * P1;
			/*Eigen::Matrix3f RX0; RX0 << 1, 0, 0, 0, 1, 0, 0, 0, 1;
			Eigen::Matrix3f RY0; RY0 << 1, 0, 0, 0, 1, 0, 0, 0, 1;
			Eigen::Matrix3f RZ0; RZ0 << -1, 0, 0, 0, -1, 0, 0, 0, 1;*/
			Eigen::Vector3f P02; P02=RZ0*RY0*RX0*P01;
			P02[2] = P02[2] + 920;
			/*Eigen::Matrix3f RX1; RX1 << 1, 0, 0, 0, cos(0.073), -sin(DEGREE_TO_ROTATE(0.073)), 0, sin(DEGREE_TO_ROTATE(0.073)), cos(DEGREE_TO_ROTATE(0.073));
			Eigen::Matrix3f RY1; RY1 << cos(DEGREE_TO_ROTATE(-0.1)), 0, sin(DEGREE_TO_ROTATE(0.1)), 0, 1, 0, -sin(DEGREE_TO_ROTATE(0.1)), 0, cos(DEGREE_TO_ROTATE(0.1));
			Eigen::Matrix3f RZ1; RZ1 << cos(DEGREE_TO_ROTATE(89.87)), -sin(DEGREE_TO_ROTATE(89.87)), 0, sin(DEGREE_TO_ROTATE(89.87)), cos(DEGREE_TO_ROTATE(89.87)), 0, 0, 0, 1;
			Eigen::Vector3f T1; T1 << 1459.306, -26.9, 2.591 + 2.07;*/
			P02 = RX1 * RY1 * RZ1 * P02 + T1;
			X = P02[0]; Y = P02[1]; Z = P02[2];
			B = -B;
			if(PosPrev != WayPointPatch->GetNodeList().GetHeadPosition())
			{
				QMeshNode* NodePrev = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(PosPrev->prev);
				double XPrev = NodePrev->m_XYZBCE[0]; double YPrev = NodePrev->m_XYZBCE[1]; double ZPrev = NodePrev->m_XYZBCE[2];
				double BPrev = NodePrev->m_XYZBCE[3]; double CPrev = NodePrev->m_XYZBCE[4]; double EPrev = NodePrev->m_XYZBCE[5];

				//translate from CNC to ABB for IRB4600
				XPrev = XPrev - 100 * sin(DEGREE_TO_ROTATE(BPrev)); ZPrev = ZPrev + 100 * (1 - cos(DEGREE_TO_ROTATE(BPrev)));
				Eigen::Vector3f P1Prev; P1Prev << XPrev, YPrev, ZPrev;
				//Eigen::Matrix3f R1; R1 << 0, -1, 0, 1, 0, 0, 0, 0, 1;
				Eigen::Vector3f P01Prev; P01Prev = R1 * P1Prev;
				/*Eigen::Matrix3f RX0; RX0 << 1, 0, 0, 0, 1, 0, 0, 0, 1;
				Eigen::Matrix3f RY0; RY0 << 1, 0, 0, 0, 1, 0, 0, 0, 1;
				Eigen::Matrix3f RZ0; RZ0 << -1, 0, 0, 0, -1, 0, 0, 0, 1;*/
				Eigen::Vector3f P02Prev; P02Prev = RZ0 * RY0 * RX0 * P01Prev;
				P02Prev[2] = P02Prev[2] + 920;
				/*Eigen::Matrix3f RX1; RX1 << 1, 0, 0, 0, cos(0.073), -sin(DEGREE_TO_ROTATE(0.073)), 0, sin(DEGREE_TO_ROTATE(0.073)), cos(DEGREE_TO_ROTATE(0.073));
				Eigen::Matrix3f RY1; RY1 << cos(DEGREE_TO_ROTATE(-0.1)), 0, sin(DEGREE_TO_ROTATE(0.1)), 0, 1, 0, -sin(DEGREE_TO_ROTATE(0.1)), 0, cos(DEGREE_TO_ROTATE(0.1));
				Eigen::Matrix3f RZ1; RZ1 << cos(DEGREE_TO_ROTATE(89.87)), -sin(DEGREE_TO_ROTATE(89.87)), 0, sin(DEGREE_TO_ROTATE(89.87)), cos(DEGREE_TO_ROTATE(89.87)), 0, 0, 0, 1;
				Eigen::Vector3f T1; T1 << 1459.306, -26.9, 2.591 + 2.07;*/
				P02Prev = RX1 * RY1 * RZ1 * P02Prev + T1;
				XPrev = P02Prev[0]; YPrev = P02Prev[1]; ZPrev = P02Prev[2];


				deltaDistance = sqrt(pow((XPrev - X), 2) + pow((YPrev - Y), 2) + pow((ZPrev - Z), 2));
				deltaE = EPrev - E;
				if (WayPointPatch->isSupportLayer)
					F_Print = abs(V_Support * deltaE * 60 / deltaDistance);
				else
					F_Print = abs(V_Print * deltaE * 60 / deltaDistance);
			
			}
			else
			{
				F_Print = 150; 
			}


			// check the large change of C angle
			/*if (Node->GetIndexNo() != 0)
			{
				GLKPOSITION prevPos = WayPointPatch->GetNodeList().Find(Node)->prev;
				QMeshNode* prevNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(prevPos);

				double C_prev = prevNode->m_XYZBCE[4];

				if (abs(C - C_prev) > 300
					&& prevNode->m_largeJumpFlag != 1 && Node->m_largeJumpFlag != -1
					&& Node->isCollisionEnd == false && Node->isCollisionStart == false
					&& Node->isNegZStart == false && Node->isNegZEnd == false) {

					Node->isHighLight = true;
					cerr << "abs(C - C_prev) ERROR! " << abs(C - C_prev) << std::endl;
					cout << prevNode->m_largeJumpFlag << Node->m_largeJumpFlag << Node->isCollisionEnd << prevNode->isCollisionStart << endl;
					cout << "WayPointPatch->GetIndexNo() " << WayPointPatch->GetIndexNo() << endl;
				}

			}*/
			// check the negative Z motion
			//if (Z < 0.0) { cout << "Z sitll < 0.0" << endl; }
			// increase the extrusion of first layer
			if (WayPointPatch->GetIndexNo() == GcodeGeneRange_From) { E = E * 1.1; }//?

			//modify the xy-offset according to the extruder
			/*if (WayPointPatch->isSupportLayer == true) {
				X = X + xExtuderOffset;
				Y = Y + yExtuderOffset;
			}*/
			// Record the max Z for security consideration (rest layers)

			//this part should be calculated in workspace and turn back to the world space again.
			if (Z > Z_max) { Z_max = Z; }

			// Add some auxiliary G code
			if (Node->GetIndexNo() == 0) {

				if (WayPointPatch->GetIndexNo() == 0) {
					// move to start of printing location
					std::fprintf(fp, "G0 F%d X%.3f Y%.3f B%.3f C%.3f \n", F_G0_XYBC, X, Y, B, C);
					// slowly lower for printing
					std::fprintf(fp, "G0 F%d Z%.3f \n", F_G0_Z, (Z_max + Z_compensateUpDistance));
					// zero extruded length(set E axis to 0)
					std::fprintf(fp, "G92 E0 \n");
					std::fprintf(fp, "G1 F%d E%.3f \n", F_PumpCompensate, E_PumpCompensateL1);
					std::fprintf(fp, "G92 E0 \n");
					//std::fprintf(fp, "G1 F%d \n", F_G1_1stlayer);
				}
				else if (WayPointPatch->isSupportLayer == IsSupportLayer_last) {
					std::fprintf(fp, "G92 E0 \n");
					std::fprintf(fp, "G0 F%d E%.3f \n", F_PumpBack, E_PumpBack);
					std::fprintf(fp, "G92 E0 \n");
					// return to the safe point Z_max + Z_high
					std::fprintf(fp, "G0 F%d Z%.3f \n", F_G0_Z, (Z_max + Z_high));
					// move to start of printing location
					std::fprintf(fp, "G0 F%d X%.3f Y%.3f B%.3f C%.3f \n", F_G0_XYBC, X, Y, B, C);
					// slowly lower for printing
					std::fprintf(fp, "G0 F%d Z%.3f \n", F_G0_Z, (Z + Z_compensateUpDistance));

					if (WayPointPatch->isSupportLayer == true) {
						std::fprintf(fp, "G1 F%d E%.3f \n", F_PumpCompensate, E_PumpCompensate * 1.2);
						std::fprintf(fp, "G92 E0 \n");
						//std::fprintf(fp, "G1 F%d \n", F_G1_support);
					}
					else {
						std::fprintf(fp, "G1 F%d E%.3f \n", F_PumpCompensate, E_PumpCompensate);
						std::fprintf(fp, "G92 E0 \n");
						//std::fprintf(fp, "G1 F%d \n", F_G1_original);
					}

				}
				// the special case: exchange extruder
				else {
					std::fprintf(fp, "G92 E0 \n");
					std::fprintf(fp, "G0 F%d E%.3f \n", F_PumpBack, E_PumpBack);
					// return to the home point Z_home
					std::fprintf(fp, "G0 F%d Z1200 \n", F_G0_Z);

					// change extruder
					if (WayPointPatch->isSupportLayer == true)
					{
						std::fprintf(fp, "T%d \n", sExtruder);
					}
					else { std::fprintf(fp, "T%d \n", iExtruder); }
					std::fprintf(fp, "G92 E0 \n");

					// move to start of printing location
					std::fprintf(fp, "G0 F%d X%.3f Y%.3f B%.3f C%.3f \n", F_G0_XYBC, X, Y, B, C);
					std::fprintf(fp, "G0 F%d Z%.3f \n", F_G0_Z, (Z + Z_compensateUpDistance));


					if (WayPointPatch->isSupportLayer == true) {
						// slowly lower for printing
						std::fprintf(fp, "G1 F%d E%.3f \n", F_PumpCompensate, E_PumpCompensateNewE * 1.2);
						std::fprintf(fp, "G92 E0 \n");
						//std::fprintf(fp, "G1 F%d\n", F_G1_support);
					}
					else {
						// slowly lower for printing
						std::fprintf(fp, "G1 F%d E%.3f \n", F_PumpCompensate, E_PumpCompensateNewE);
						std::fprintf(fp, "G92 E0 \n");
						//std::fprintf(fp, "G1 F%d\n", F_G1_original);
					}
				}
				std::fprintf(fp, "G1 F%.3f X%.3f Y%.3f Z%.3f B%.3f C%.3f E%.3f \n", F_Print, X, Y, Z, B, C, E);
			}
			else {
				// Consider the waypoints with too large Length OR large Singularity areas
				if (Node->m_largeJumpFlag == -1 || Node->isSingularSecEndNode || Node->isCollisionStart || Node->isCollisionEnd || Node->isNegZStart || Node->isNegZEnd) {

					//GLKPOSITION prevPos = WayPointPatch->GetNodeList().Find(Node)->prev;
					//QMeshNode* prevNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(prevPos);
					//double oldZ = prevNode->m_XYZBCE[2];
					//double odlE = prevNode->m_XYZBCE[5];

					//std::cout << Node->m_XYZBCE[5] << std::endl;
					//std::cout << "oldE = "<<odlE << std::endl;

					if (WayPointPatch->isSupportLayer == true) {
						//std::fprintf(fp, "G0 E%.3f F%d\n", (odlE + E_PumpBack * 1.95), F_PumpBack);
						//std::fprintf(fp, "G0 Z%.3f F%d\n", (max(Z_max, oldZ) + Z_high), F_G0_Z);
						std::fprintf(fp, "G0 F%d E%.3f \n", F_PumpBack, (E + E_PumpBack * 1.4));
						std::fprintf(fp, "G0 F%d Z%.3f \n", F_G0_Z, (Z_max + Z_high));
						std::fprintf(fp, "G0 F%d X%.3f Y%.3f B%.3f C%.3f \n", F_G0_XYBC, X, Y, B, C);
						std::fprintf(fp, "G0 F%d Z%.3f \n", F_G0_Z, (Z + Z_compensateUpDistance));
						std::fprintf(fp, "G0 F%d E%.3f \n", F_PumpCompensate, E);
						//std::fprintf(fp, "G0 F%d Z%.3f E%.3f \n", F_G1_support, Z, E);
						std::fprintf(fp, "G0 F%d Z%.3f \n", F_G0_Z, Z);
					}
					else {
						//std::fprintf(fp, "G0 E%.3f F%d\n", (odlE + E_PumpBack * 0.8), F_PumpBack);
						//std::fprintf(fp, "G0 Z%.3f F%d\n", (max(Z_max, oldZ) + Z_high), F_G0_Z);

						std::fprintf(fp, "G0 F%d E%.3f \n", F_PumpBack, (E + E_PumpBack * 0.8));
						std::fprintf(fp, "G0 F%d Z%.3f \n", F_G0_Z, (Z_max + Z_high));
						std::fprintf(fp, "G0 F%d X%.3f Y%.3f B%.3f C%.3f \n", F_G0_XYBC, X, Y, B, C);
						std::fprintf(fp, "G0 F%d Z%.3f \n", F_G0_Z, (Z + Z_compensateUpDistance));
						std::fprintf(fp, "G0 F%d E%.3f \n", F_PumpCompensate, E);
						//std::fprintf(fp, "G0 F%d Z%.3f E%.3f \n", F_G1_support, Z, E);
					}

					//std::cout << "oldE = " << odlE << std::endl;

				}
				std::fprintf(fp, "G1 F%.3f X%.3f Y%.3f Z%.3f B%.3f C%.3f E%.3f \n", F_Print, X, Y, Z, B, C, E);
			}
			//std::cout << E << std::endl;
		}

		IsSupportLayer_last = WayPointPatch->isSupportLayer;
	}

	std::fprintf(fp, "G92 E0 \n");
	std::fprintf(fp, "G0 F%d E%.3f \n", F_G0_XYBC, E_PumpBack); // PumpBack
	std::fprintf(fp, "G0 F%d Z1200 \n", F_G0_Z);; // return to the home point Z_home
	//std::fprintf(fp, "G0 F%d X0 Y0 B0 C0 \n", F_G0_XYBC);
	//std::fprintf(fp, "M30\n");// Stop all of the motion

	std::fclose(fp);

	std::cout << "------------------------------------------- " << rltDir << " Gcode Write Finish!\n" << std::endl;
}

void fiveAxisPoint::_drawWaypointData(GLKObList* polygenMeshList)
{
	
}