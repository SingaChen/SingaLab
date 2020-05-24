#pragma once
#include "..\eigen3\Eigen\Eigen"

class QMeshPatch;
class GLKMatrix;

/////////////////////////Guoxin Fang/////////////////////////
//The DeformTet is writen for topology optimization project//
/////////////////////////////////////////////////////////////

class DeformTet
{
public:
	DeformTet();
	~DeformTet();

	void PreProcess();
	bool Run(int loop = 1);
	void DoTopologyAnalysis(bool render);

	//Read mesh from platform
	void SetMesh(QMeshPatch *mesh) { TetMesh = mesh; }

private:

	void ClearAll();
	void Initialization();
	void FillMatrixA();
	void FactorizeMatrixA();
	void FillVectorB();
	bool Solve();
	void LocalProjection();
	bool UpdateVertex();
	void ComputeLocalGoalAndInverse();
	void ShifttoInitialPos();
	void ComputeARAPenergy();
	void SensitivityAnalysis_DynamicDenseMethod();
	void SensitivityAnalysis_DirectlyDenseChange();
	void DrawARAPEnergyField();

	int vertNum, eleNum;
	double ReduceConstrainRatio = 0.6;
	double ReduceRatio_perIteration = 0.03;
	double zeroDensity = 0.01;
	//Define all matrix used in computings
	Eigen::SparseMatrix<double> matA;
	Eigen::SparseMatrix<double> matAT;
	Eigen::SimplicialLDLT <Eigen::SparseMatrix<double>> Solver;
	//SparseQR <Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int>> Solver;
	//SparseLU<SparseMatrix<double>> Solver;
	std::vector<Eigen::MatrixXd> LocalCoord, InverseP, LocalGoal;
	std::vector<Eigen::VectorXd> VectorXPosition, VectorBSide, VectorXPosition_Last;

private:
	QMeshPatch *TetMesh;
};

