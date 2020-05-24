	#pragma once

#include <vector>
#include <../eigen3/Eigen/Dense> 
#include <dirent.h>
#include <iostream>
#include <iomanip>

#include "../QMeshLib/QMeshPatch.h"
#include "../QMeshLib/PolygenMesh.h"
#include "../QMeshLib/QMeshFace.h"
#include "../QMeshLib/QMeshNode.h"
#include "../PQPLib/PQP.h"
#include "QHullLib/qhull_a.h"
#include "PMBody.h"


using namespace std;
using namespace Eigen;

class QMeshPatch;
class PolygenMesh;
typedef struct qHullSet {
	int faceNum;	double* normalVec;	double* offset;
	int vertNum;	double* vertPos;
	unsigned int* faceTable;	//	Note that: the index starts from '1'
}QHULLSET;

class GcodeGeneration {

public:
	GcodeGeneration() {};
	~GcodeGeneration() {};

	// bool build = false;

	void getLayerHeight(PolygenMesh* polygenMesh_Slices, PolygenMesh* polygenMesh_Waypoints, bool varyThickness_switch);
	void getUpZwayPnts(PolygenMesh* polygenMesh_Waypoints);
	void singularityOpt(PolygenMesh* polygenMesh_Waypoints);
	void detectCollision(PolygenMesh* polygenMesh_Waypoints, PolygenMesh* polygenMesh_extruderHead, bool func_switch);
	void height2E(PolygenMesh* polygenMesh_Waypoints, bool func_switch);
	void writeGcode(PolygenMesh* polygenMesh_Waypoints, string rltDir);

private:

	void isLargeLength(QMeshPatch* WayPointPatch);
	void normalSmooth(QMeshPatch* WayPointPatch, int loop, bool smoothSwitch);
	void getRawCdata(QMeshPatch* WayPointPatch, double lambda);
	void getDangerSec(QMeshPatch* WayPointPatch, MatrixXf& sectionTable);

	void getNewPntNor(QMeshPatch* WayPointPatch, const MatrixXf& sectionTable, double lambda);
		void getCspacePnt(vector<double>& CspacePnt, int PntIndex, QMeshPatch* WayPointPatch);
		double getRotRadius(vector<double>& srtPnt, vector<double>& endPnt, double minR, 
			vector<double>& srtPnt_temp, vector<double>& endPnt_temp);
		int getQuadrant(vector<double>& quadPnt);
		int getRotSign(int quadSrt, int quadEnd, vector<double>& srtPnt, vector<double>& endPnt);
		double getVec2Angle(vector<double>& srtPnt, vector<double>& endPnt);

	void getXYZBCE(QMeshPatch* WayPointPatch, int layersNoSolve);
	void optimizationC(QMeshPatch* WayPointPatch);
	void testXYZBCE(QMeshPatch* WayPointPatch, string Dir, bool testSwitch);
	void testLayerHeight(QMeshPatch* WayPointPatch, string Dir, bool testSwitch);

	QHULLSET* buildConvexHull_extruderHead(QMeshPatch* eHead);
	QHULLSET* _mallocMemoryConvexHull(int faceNum, int vertNum);
	bool _isPntInsideConvexHull(QHULLSET* pConvexHull, double pnt[]);
	void _freeMemoryConvexHull(QHULLSET*& pConvexHull);

};


