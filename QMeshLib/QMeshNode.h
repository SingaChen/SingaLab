// QMeshNode.h: interface for the QMeshNode class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _QMESHNODE
#define _QMESHNODE

#include "../GLKLib/GLKObList.h"
#include <../eigen3/Eigen/Dense> 

class QMeshPatch;
class QMeshFace;
class QMeshEdge;
class QMeshTetra;

class QMeshNode : public GLKObject
{
public:
	QMeshNode();
	virtual ~QMeshNode();

public:
	int GetIndexNo();		//from 1 to n
	void SetIndexNo(const int _index = 1);

	bool GetAttribFlag(const int whichBit);
	void SetAttribFlag(const int whichBit, const bool toBe = true);

	void GetCoord2D(double& x, double& y);
	void SetCoord2D(double x, double y);

	void GetCoord3D(double& x, double& y, double& z);
	void SetCoord3D(double x, double y, double z);

	void GetCoord3D_last(double& x, double& y, double& z);
	void SetCoord3D_last(double x, double y, double z);

	void SetMeanCurvatureNormalVector(double kHx, double kHy, double kHz);
	void GetMeanCurvatureNormalVector(double& kHx, double& kHy, double& kHz);

	void SetGaussianCurvature(double kG);
	double GetGaussianCurvature();

	void SetPMaxCurvature(double k1);
	double GetPMaxCurvature();

	void SetPMinCurvature(double k2);
	double GetPMinCurvature();

	void CalNormal();
	void CalNormal(double normal[]);
	void GetNormal(double& nx, double& ny, double& nz) { nx = m_normal[0]; ny = m_normal[1]; nz = m_normal[2]; };
	void SetNormal(double nx, double ny, double nz) { m_normal[0] = nx; m_normal[1] = ny; m_normal[2] = nz; };

	void SetBoundaryDis(double dist);
	double GetBoundaryDis();

	void SetDensityFuncValue(double density) { m_densityFuncValue = density; };
	double GetDensityFuncValue() { return m_densityFuncValue; };

	void SetMeshPatchPtr(QMeshPatch* _mesh);
	QMeshPatch* GetMeshPatchPtr();

	void AddTetra(QMeshTetra* trglTetra);
	int GetTetraNumber();
	QMeshTetra* GetTetraRecordPtr(int No);	//from 1 to n
	GLKObList& GetTetraList();

	void AddFace(QMeshFace* _face);
	int GetFaceNumber();
	QMeshFace* GetFaceRecordPtr(int No);	//from 1 to n
	GLKObList& GetFaceList();

	void AddEdge(QMeshEdge* _edge);
	int GetEdgeNumber();
	QMeshEdge* GetEdgeRecordPtr(int No);	//from 1 to n
	GLKObList& GetEdgeList();

	void AddNode(QMeshNode* _node);
	int GetNodeNumber();
	QMeshNode* GetNodeRecordPtr(int No);	//from 1 to n
	GLKObList& GetNodeList();
	bool IsNodeInNodeList(QMeshNode* _node);

	void SetMinCurvatureVector(double vx, double vy, double vz);
	void GetMinCurvatureVector(double& vx, double& vy, double& vz);

	void SetMaxCurvatureVector(double vx, double vy, double vz);
	void GetMaxCurvatureVector(double& vx, double& vy, double& vz);

	void SetWeight(double weight) { m_weight = weight; };
	double GetWeight() { return m_weight; };

	void SetColor(float r, float g, float b) { m_rgb[0] = r; m_rgb[1] = g; m_rgb[2] = b; };
	void GetColor(float& r, float& g, float& b) { r = m_rgb[0]; g = m_rgb[1]; b = m_rgb[2]; };

	GLKObList attachedList;

	double m_trackingPos[3];	int m_collideConstraintIndex;
	QMeshFace* m_trackingFace;

	void* attachedPointer, * attachedPointer1;

	int m_nIdentifiedPatchIndex = -1;
	bool selected = false;
	bool selectedforEdgeSelection;
	bool selectedforFaceSelection;
	bool boundary;
	bool boundary1, boundary2;
	bool active;
	bool visited;
	int featurePt;
	bool m_sepFlag;
	bool isFixed = false;
	bool isHandle = false;

	double value1;
	double value2;

	double Alpha = 0; //For TopOpt Filter Variable

	double U, V, W;

	void GetCoord3D_FLP(double& x, double& y, double& z);
	void SetCoord3D_FLP(double x, double y, double z);

	void GetCoord3D_Laplacian(double& x, double& y, double& z) { x = coord3D_Laplacian[0]; y = coord3D_Laplacian[1]; z = coord3D_Laplacian[2]; };
	void SetCoord3D_Laplacian(double x, double y, double z) { coord3D_Laplacian[0] = x; coord3D_Laplacian[1] = y; coord3D_Laplacian[2] = z; };

	void SetMixedArea(double area) { m_mixedArea = area; };

	int TempIndex; // for remeshing
	int identifiedIndex;
	bool inner, i_inner;

	// for Gcode simulation
	Eigen::Vector3d QMeshNode::calPlatformGuesture(double P_x, double P_y, double P_z, double X, double Y, double B, double C);

private:
	int indexno;
	bool flags[8];
	// bit 0 -- True for boundary points
	// bit 1 -- True for points on coarse mesh
	// bit 2 -- True for points on interpolation curve 
	// bit 3 -- True for points on hand (temp use) (or points adjacent to boundary face)
	// bit 4 -- True for points on arm (temp use) (or branch vertices)
	// bit 5 -- True for sharp-feature vertex (or vertex cannot be moved)
	// bit 6 -- True for sharp-feature-region vertex
	// bit 7 -- True for points moved (temp use or newly created)
	double coord2D[2];
	// 2D space coordinate
	double coord3D[3];
	// 3D space coordinate
	double coord3D_last[3];  // just for reset sewing operation for one step
				// 3D space coordinate in the last sewing step
	double coord3D_Laplacian[3];

	double m_meanCurvatureNormalVector[3], m_gaussianCurvature, m_pMaxCurvature, m_pMinCurvature;
	double m_minCurvatureVector[3], m_maxCurvatureVector[3];
	double m_boundaryDist, m_densityFuncValue;
	double m_mixedArea;

	QMeshPatch* meshSurface;		// QMesh contains this node

	GLKObList faceList;	// a list of faces contained this node (QMeshFace)
	GLKObList edgeList;	// a list of edges contained this node (QMeshEdge)
	GLKObList nodeList;	// a list of nodes coincident with this node (QMeshNode)
	GLKObList tetraList;	// a list of nodes coincident with this node (QMeshNode)

	double m_normal[3];
	double m_weight;
	double coord3D_FLP[3]; //For Keep the FLProcessData

	float m_rgb[3];


	/* Added by ZTY 2020-02-28 */
	// variables for G code Generation.
public:
	double m_orginalNormal[3];
	double m_orginalPostion[3];
	double m_YZ_rotateNormal[3];
	double m_YZ_rotatePostion[3];
	double m_flipNormal[3];
	double m_printNormal[3];
	double m_printPostion[3];
	double m_finalNormal[3];
	double m_finalPostion[3];
	double m_XYZBCE[7];//X/Y/Z/B/C/E/S
	double m_ABB_XYZBCEF[7];
	double m_ABB_Velocty[3];
	double m_layerHeight;
	double m_layerWidth;

	double m_Bsolve[2];
	double m_Csolve[2];
	unsigned int m_BCsolveChoice;

	double m_norCspace[2];
	int    m_safeFlag = 0;
	int    m_largeJumpFlag = 0;
	bool   isCollision = false;
	bool   iscollided = false;
	bool   isGcodeNode = false;
	bool   isSupportNode = false;
	bool   isSingularNode = false;
	bool   isSingularSecEndNode = false;
	//bool   isfirstNode = false;
	bool   isCollisionStart = false;
	bool   isCollisionEnd = false;
	bool   negativeZ = false;
	bool   isNegZStart = false;
	bool   isNegZEnd = false;
	bool   negativeZz = false;
	bool   overV_C = false;
	bool   isHighLight = false;
	bool   isPrintPnt = false;

};

#endif
