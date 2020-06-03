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
#include <fstream>

using namespace std;
using namespace Eigen;

class QMeshPatch;
class PolygenMesh;
typedef struct qHullSet 
{
	int faceNum;	double* normalVec;	double* offset;
	int vertNum;	double* vertPos;
	unsigned int* faceTable;	//	Note that: the index starts from '1'
}QHULLSET;

class fiveAxisPoint
{

public:
	fiveAxisPoint() {};
	~fiveAxisPoint() {};

	Eigen::MatrixXd initWayPoint;
	
	/*natSort will output a filenamecell in sequence something like 100 101 102 200...(as string)
	  and the input of natSort is the folder of pointdata named PosNorFileDir*/
	void natSort(QString PosNorFileDir, vector<string>& fileNameCell);

	/*readWayPointData will build up a Polygenmesh in polygenmeshilist,
	  and this polygenmesh contain the data of waypoint something like xyznxnynz,
	  also, it will display the model in the view window and model tree*/
	void readWayPointData(QString packName, bool Yup2Zup_switch, GLKObList *polygenMeshList, vector<string> wayPointFileCell, GLKLib* pGLK);
	
	/*readSliceData will build up a Polygenmesh in polygenmeshilist,
	  and this polygenmesh contain the data of each layer,
	  also, it will display the model in the view window and model tree*/
	void readSliceData(QString sliceSetName, bool Yup2Zup_switch, double Zoff, double Xoff, double Yoff, GLKObList *polygenMeshList, vector<string> sliceSetFileCell, GLKLib* pGLK);
	
	/*readExtruderHeadfile will build up a Polygenmesh in polygenmeshilist,
	  and this polygenmesh contain the data of ExtruderHead,
	  also, it will display the model in the view window and model tree*/
	void readExtruderHeadfile(string extruderHeadName, GLKObList *polygenMeshList, GLKLib* pGLK);
	
	/*readPlatformfile will build up a Polygenmesh in polygenmeshilist,
	  and this polygenmesh contain the data of platform,
	  also, it will display the model in the view window and model tree*/
	void readPlatformfile(string platformName, GLKObList *polygenMeshList, GLKLib* pGLK);
	
	/*getLayerHeight will build up a Polygenmesh in polygenmeshilist,
	  and this polygenmesh contain the data of platform,
	  also, it will display the model in the view window and model tree*/
	void getLayerHeight(PolygenMesh* polygenMesh_Slices, PolygenMesh* polygenMesh_Waypoints, PolygenMesh* polygenMesh_PrintPlatform,
		bool varyThickness_switch, int GcodeGeneRange_From, int GcodeGeneRange_To, bool upPlate2Height_switch, double upZdist, double Xmove = 0.0, double Ymove = 0.0);
	void getUpZwayPnts(PolygenMesh* polygenMesh_Waypoints);
	void singularityOpt(PolygenMesh* polygenMesh_Waypoints, int GcodeGeneRange_From, int GcodeGeneRange_To);
	void detectCollision(PolygenMesh* polygenMesh_Waypoints, PolygenMesh* polygenMesh_extruderHead, bool func_switch);
	void height2E(PolygenMesh* polygenMesh_Waypoints, int GcodeGeneRange_From, int GcodeGeneRange_To, bool func_switch);
	void writeGcode(PolygenMesh* polygenMesh_Waypoints, string rltDir, int GcodeGeneRange_From, int GcodeGeneRange_To, double E3_xOff, double E3_yOff);
	void isLargeLength(QMeshPatch* WayPointPatch);
	void getD(QMeshPatch* WayPointPatch);
	void normalSmooth(QMeshPatch* WayPointPatch, int loop, bool smoothSwitch);
	void getRawCdata(QMeshPatch* WayPointPatch, double lambda);
	void getDangerSec(QMeshPatch* WayPointPatch, MatrixXf& sectionTable);
	void getBC2(QMeshPatch* WayPointPatch, int layersNoSolve, MatrixXf& B1C1table, MatrixXf& B2C2table);
	void motionPlanning(QMeshPatch* WayPointPatch, const MatrixXf& sectionTable, const MatrixXf& B1C1table, const MatrixXf& B2C2table);
	void getXYZ(QMeshPatch* WayPointPatch);
	void optimizationC(QMeshPatch* WayPointPatch);
	void writeABBGcode(PolygenMesh* polygenMesh_Waypoints, string rltDir, int GcodeGeneRange_From, int GcodeGeneRange_To, double E3_xOff, double E3_yOff);

	void _drawWaypointData(GLKObList* polygenMeshList);


	void testXYZBCE(QMeshPatch* WayPointPatch, string Dir, bool testSwitch);
	void testLayerHeight(QMeshPatch* WayPointPatch, string Dir, bool testSwitch);
	QHULLSET* buildConvexHull_extruderHead(QMeshPatch* eHead);
	QHULLSET* _mallocMemoryConvexHull(int faceNum, int vertNum);
	bool _isPntInsideConvexHull(QHULLSET* pConvexHull, double pnt[]);
	void _freeMemoryConvexHull(QHULLSET*& pConvexHull);
};