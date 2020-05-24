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



class waypoints {

public:
	waypoints() {};
	~waypoints() {};
	void translation();
	//void output();
	Eigen::MatrixXd initWayPoint;
};