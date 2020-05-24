#include "stdafx.h"
#include "DeformTet.h"
#include <QCoreApplication>

#include "..\GLKLib\GLKObList.h"
#include "..\GLKLib\GLKGeometry.h"
#include "..\GLKLib\GLKMatrixLib.h"
#include "..\QMeshLib\QMeshPatch.h"
#include "..\QMeshLib\QMeshNode.h"
#include "..\QMeshLib\QMeshTetra.h"

#include <omp.h>

#include <iostream>
#include <fstream>

using namespace std;
using namespace Eigen;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define VELE 4
#define FACTOR 3

DeformTet::DeformTet()
{

}

DeformTet::~DeformTet()
{
	ClearAll();
}

void DeformTet::ClearAll()
{
	/*if (FactorizedA) { LinearSolver::DeleteF(FactorizedA); FactorizedA = 0; }
	if (VectorX) { delete VectorX; VectorX = 0; }
	if (VectorB) { delete VectorB; VectorB = 0; }
	if (matA) delete matA;
	if (LocalCoord) delete[] LocalCoord;
	if (LocalGoal) delete[] LocalGoal;
	if (InverseP) delete[] InverseP;*/
}

void DeformTet::PreProcess()
{
	Initialization();
	ComputeLocalGoalAndInverse();

	FillMatrixA();
	FactorizeMatrixA();
	cout << "finish preprocess the system" << endl;
}

bool DeformTet::Run(int loop)
{
	for (int i = 0; i < loop; i++) {
		LocalProjection();
		FillVectorB();
		Solve();
		if (!UpdateVertex()) return false;
		//ShifttoInitialPos();
	}
	return true;
}

void DeformTet::DoTopologyAnalysis(bool render)
{
	ComputeARAPenergy();
	if (render) 
	{ 
		SensitivityAnalysis_DirectlyDenseChange(); 
		DrawARAPEnergyField();
	}
	else DrawARAPEnergyField();
}

void DeformTet::Initialization()
{
	ClearAll();

	//set node and face number
	vertNum = 0;
	for (GLKPOSITION pos = TetMesh->GetNodeList().GetHeadPosition(); pos != nullptr;) {
		QMeshNode *node = (QMeshNode*)TetMesh->GetNodeList().GetNext(pos);
		if(node->isFixed == false && node->isHandle == false)
			node->SetIndexNo(vertNum++);
		else node->SetIndexNo(-1);

		double pp[3] = { 0 };
		node->GetCoord3D_last(pp[0], pp[1], pp[2]);
		node->SetCoord3D(pp[0], pp[1], pp[2]);
	}
	eleNum = 0;
	for (GLKPOSITION Pos = TetMesh->GetTetraList().GetHeadPosition(); Pos;) {
		QMeshTetra *Tet = (QMeshTetra*)TetMesh->GetTetraList().GetNext(Pos);
		if (Tet->IsFixed()) Tet->SetIndexNo(-1);
		else Tet->SetIndexNo(eleNum++);
	}

	//initialize all computing matrix
	matA.resize(VELE * eleNum, vertNum);
	matAT.resize(vertNum, VELE * eleNum);

	LocalCoord.resize(eleNum);
	InverseP.resize(eleNum);
	LocalGoal.resize(eleNum);
	VectorXPosition.resize(3);
	VectorXPosition_Last.resize(3);
	VectorBSide.resize(3);

	for (int i = 0; i < 3; i++) {
		VectorXPosition[i] = Eigen::VectorXd::Zero(vertNum);
		VectorXPosition_Last[i] = Eigen::VectorXd::Zero(vertNum);
		VectorBSide[i] = Eigen::VectorXd::Zero(eleNum * VELE);
	}

	//precomputing
	//ComputeLocalGoalAndInverse();
}

void DeformTet::ComputeLocalGoalAndInverse()
{
	//For the deformation usage, every tetra should remain its initial shape
	for (GLKPOSITION Pos = TetMesh->GetTetraList().GetHeadPosition(); Pos;) {
		QMeshTetra *Tet = (QMeshTetra*)TetMesh->GetTetraList().GetNext(Pos);
		int fdx = Tet->GetIndexNo();
		if (fdx < 0) continue;
		Tet->selected = false;
		for (int i = 0; i < VELE; i++) {
			QMeshNode* Tet_Node = Tet->GetNodeRecordPtr(i + 1);
			if (Tet_Node->selected == true) {
				Tet->selected = true;
				break;
			}
		}

		QMeshNode *nodes[VELE];
		Eigen::MatrixXd P = Eigen::MatrixXd::Zero(3, VELE);
		double center[3] = { 0 };

		for (int i = 0; i < VELE; i++) {
			nodes[i] = Tet->GetNodeRecordPtr(i + 1);
			nodes[i]->GetCoord3D_last(P(0, i), P(1, i), P(2, i));
			for (int j = 0; j < 3; j++) center[j] += P(j, i);
		} for (int j = 0; j < 3; j++) center[j] /= VELE;
		for (int i = 0; i < VELE; i++) for (int j = 0; j < 3; j++) P(j, i) -= center[j];

		//This is for selected reigon size
		if (Tet->selected == true) {
			for (int i = 0; i < VELE; i++) {
				for (int j = 0; j < 3; j++) { P(j, i) *= pow(1.5, 1.0 / 3); }
			}
		}

		//solving pseudoInverse with GLKMATRIX Lib, Eigen may occur error here
		LocalGoal[fdx] = P;

		GLKMatrix GLKP(3, VELE), GLKInverseP(3, VELE);
		InverseP[fdx] = Eigen::MatrixXd::Zero(3, 4);

		for (int i = 0; i < 3; i++) { for (int j = 0; j < 4; j++)  GLKP(i, j) = P(i, j); }

		GLKMatrix TP(VELE, 3), GLKATA(3, 3);
		GLKMatrixLib::Transpose(GLKP, 3, VELE, TP);
		GLKMatrixLib::Mul(GLKP, TP, 3, VELE, 3, GLKATA);

		if (!GLKMatrixLib::Inverse(GLKATA, 3)) { 
			printf("ERROR in finding Inverse!\n"); 
			getchar(); 
		}
		GLKMatrixLib::Mul(GLKATA, GLKP, 3, 3, VELE, GLKInverseP);

		for (int i = 0; i < 3; i++) { for (int j = 0; j < 4; j++) InverseP[fdx](i, j) = GLKInverseP(i, j); }
	}
	//printf("Finish Compute local inverse!\n");
}

void DeformTet::FillMatrixA()
{ 
	//give memory to sparse matrix, to accerate the insert speed
	matA.reserve(VectorXi::Constant(VELE * eleNum, 1000));

	float c1 = -1.0 / VELE, c2 = 1 + c1;

	for (GLKPOSITION Pos = TetMesh->GetTetraList().GetHeadPosition(); Pos;) {
		QMeshTetra *Tetra = (QMeshTetra*)TetMesh->GetTetraList().GetNext(Pos);
		if (Tetra->GetIndexNo() < 0) continue;

		int fdx = Tetra->GetIndexNo() * VELE;

		int vdxArr[VELE];
		for (int i = 0; i < VELE; i++) vdxArr[i] = Tetra->GetNodeRecordPtr(i + 1)->GetIndexNo();

		for (int i = 0; i < VELE; i++) {
			if (vdxArr[i] < 0) continue;
			for (int j = 0; j < VELE; j++) {
				if (vdxArr[j] < 0) continue;
				if (i == j) matA.insert(fdx + j, vdxArr[i]) = c2*pow(Tetra->dense, FACTOR);
				else matA.insert(fdx + j, vdxArr[i]) = c1*pow(Tetra->dense, FACTOR);
			}
		}
	}

	matA.makeCompressed();
}

void DeformTet::FactorizeMatrixA()
{
	Eigen::SparseMatrix<double> matATA(vertNum, vertNum);

	matAT = matA.transpose();
	matATA = matAT*matA;

	Solver.compute(matATA);
	//printf("end factorize materix A\n");
}

void DeformTet::FillVectorB()
{
	double c1 = -1.0 / VELE, c2 = 1 + c1;

	int Core = 12;
	int EachCore = eleNum / Core + 1;
	//int n = 0;

#pragma omp parallel   
	{
#pragma omp for  
		for (int omptime = 0; omptime < Core; omptime++) {
			//int BeginTetraIndex = EachCore * omptime + 1;
			//				if (TetraIndex > TetraNumSum) break;
			for (GLKPOSITION Pos = TetMesh->GetTetraList().GetHeadPosition(); Pos;) {
				QMeshTetra *Tetra = (QMeshTetra*)TetMesh->GetTetraList().GetNext(Pos);

				if (Tetra->GetIndexNo() < omptime*EachCore) continue;
				else if (Tetra->GetIndexNo() >(1 + omptime)*EachCore) break;

				if (Tetra->GetIndexNo() != -1) {
					int fdx = Tetra->GetIndexNo();
					//double center[3]; face->CalCenterPos(center[0], center[1], center[2]);
					double center[3] = { 0 };
					for (int i = 0; i < VELE; i++) for (int j = 0; j < 3; j++) center[j] += LocalCoord[fdx](j, i);
					for (int i = 0; i < 3; i++) center[i] /= VELE;

					for (int i = 0; i < VELE; i++) {
						for (int j = 0; j < 3; j++) {
							VectorBSide[j](fdx*VELE + i) = (LocalCoord[fdx](j, i) - center[j])*pow(Tetra->dense, FACTOR);
							//VectorBSide[j](fdx*VELE + i) = LocalCoord[fdx](j, i)* cc2 / c2;

							//if (abs(VectorBSide[j](fdx*VELE + i)) < 0.0001) VectorBSide[j](fdx*VELE + i) = 0.0;
							//cout << VectorBSide[j](fdx*VELE + i) <<endl;
						}
						for (int k = 0; k < VELE; k++) {
							QMeshNode *node = Tetra->GetNodeRecordPtr(k + 1);
							if (node->GetIndexNo() < 0) {
								//printf("This happen once\n\n");
								double p[3]; node->GetCoord3D(p[0], p[1], p[2]);
								if (i == k) for (int l = 0; l < 3; l++) VectorBSide[l](fdx *VELE + i) -= p[l] * c2*pow(Tetra->dense, FACTOR);
								else for (int l = 0; l < 3; l++) VectorBSide[l](fdx *VELE + i) -= p[l] * c1*pow(Tetra->dense, FACTOR);
							}
						}
					}
				}
			}
		}
	}
}

bool DeformTet::Solve()
{
#pragma omp parallel
	{
#pragma omp for
		for (int i = 0; i < 3; i++) {
			Eigen::VectorXd ATB = matAT*VectorBSide[i];
			VectorXPosition[i] = Solver.solve(ATB);
		}
	}
	return true;
}

void DeformTet::LocalProjection()
{
	int Core = 12;
	int EachCore = eleNum / Core + 1;

#pragma omp parallel   
	{
#pragma omp for  
		for (int omptime = 0; omptime < Core; omptime++) {
			//int BeginTetraIndex = EachCore * omptime + 1;
			//				if (TetraIndex > TetraNumSum) break;
			for (GLKPOSITION Pos = TetMesh->GetTetraList().GetHeadPosition(); Pos;) {
				QMeshTetra *Tetra = (QMeshTetra*)TetMesh->GetTetraList().GetNext(Pos);

				if (Tetra->GetIndexNo() < omptime*EachCore) continue;
				else if (Tetra->GetIndexNo() > (1 + omptime)*EachCore) break;

				int fdx = Tetra->GetIndexNo(); if (fdx < 0) continue;


				double center[3] = {0}; 
				Tetra->CalCenterPos(center[0], center[1], center[2]);

				//This tP should be put ahead to see if it works for the energy function.
				Eigen::MatrixXd tP = Eigen::MatrixXd::Zero(VELE, 3); QMeshNode *nodes[VELE];
				for (int i = 0; i < VELE; i++) {
					nodes[i] = Tetra->GetNodeRecordPtr(i + 1);
					nodes[i]->GetCoord3D(tP(i, 0), tP(i, 1), tP(i, 2));
				}
				for (int i = 0; i < VELE; i++) for (int j = 0; j < 3; j++) tP(i, j) -= center[j];

				Eigen::Matrix3d T = Matrix3d::Zero(3, 3),
					T_transpose = Matrix3d::Zero(3, 3), R = Matrix3d::Zero(3, 3);
				T = InverseP[fdx] * tP;
				T_transpose = T.transpose();

				/////////////////Eigen SVD decomposition/////////////////////////

				JacobiSVD<Eigen::MatrixXd> svd(T_transpose, ComputeThinU | ComputeThinV);
				Matrix3d V = svd.matrixV(), U = svd.matrixU();
				R = U * V.transpose();

				LocalCoord[fdx] = R * LocalGoal[fdx];
			}
		}
	}
}

void DeformTet::ComputeARAPenergy()
{
	//Using ARAP energy for sensitiviy analysis

//	int Core = 12;
//	int EachCore = eleNum / Core + 1;
//
//	//First compute the Sensitivity analysis for every element
//
//#pragma omp parallel   
//	{
//#pragma omp for  
//		for (int omptime = 0; omptime < Core; omptime++) {
//			//int BeginTetraIndex = EachCore * omptime + 1;
//			//				if (TetraIndex > TetraNumSum) break;
//			for (GLKPOSITION Pos = TetMesh->GetTetraList().GetHeadPosition(); Pos;) {
//				QMeshTetra *Tetra = (QMeshTetra*)TetMesh->GetTetraList().GetNext(Pos);
//
//				if (Tetra->GetIndexNo() < omptime*EachCore) continue;
//				else if (Tetra->GetIndexNo() > (1 + omptime)*EachCore) break;
//
//				int fdx = Tetra->GetIndexNo(); 
//				
//				if (fdx < 0) continue; //this tetra belongs to rigid or handle region
//
//				double center[3] = { 0 };
//				Tetra->CalCenterPos(center[0], center[1], center[2]);
//
//				//This tP should be put ahead to see if it works for the energy function.
//				Eigen::MatrixXd tP = Eigen::MatrixXd::Zero(VELE, 3); QMeshNode *nodes[VELE];
//				for (int i = 0; i < VELE; i++) {
//					nodes[i] = Tetra->GetNodeRecordPtr(i + 1);
//					nodes[i]->GetCoord3D(tP(i, 0), tP(i, 1), tP(i, 2));
//				}
//				for (int i = 0; i < VELE; i++) for (int j = 0; j < 3; j++) tP(i, j) -= center[j];
//
//				Eigen::Matrix3d T = Matrix3d::Zero(3, 3),
//					T_transpose = Matrix3d::Zero(3, 3), R = Matrix3d::Zero(3, 3);
//				T = InverseP[fdx] * tP;
//				T_transpose = T.transpose();
//
//				/////////////////Eigen SVD decomposition/////////////////////////
//
//				JacobiSVD<Eigen::MatrixXd> svd(T_transpose, ComputeThinU | ComputeThinV);
//				Matrix3d V = svd.matrixV(), U = svd.matrixU();
//				R = U * V.transpose();
//
//				LocalCoord[fdx] = R * LocalGoal[fdx];
//				/*cout << "LC = " << LocalCoord[fdx] << endl << endl;
//				cout << "TP = " << tP << endl << endl;*/
//
//				//Compute the ARAP energy
//				for (int i = 0; i < VELE; i++) {
//					for (int j = 0; j < 3; j++) {
//						Tetra->ARAPenergy += (LocalCoord[fdx](j, i) - tP(i, j))
//							*(LocalCoord[fdx](j, i) - tP(i, j)) * fabs(Tetra->CalVolume())*1000;
//					}
//				}
//			}
//		}
//	}

	//Using UTKU for sensitivity anlysis
	double c1 = -1.0 / VELE, c2 = 1 + c1;
	MatrixXd A = MatrixXd::Zero(4, 4), U = MatrixXd::Zero(4, 3), AU = MatrixXd::Zero(4, 3);
	Matrix3d UTATAU = Matrix3d::Zero(3, 3);
	double pp[3] = { 0 }, ppl[3] = { 0 };
	for (int i = 0; i < VELE; i++) {
		for (int j = 0; j < VELE; j++) {
			if (i == j) A(i, j) = c2;
			else A(i, j) = c1;
		}
	}

	for (GLKPOSITION Pos = TetMesh->GetTetraList().GetHeadPosition(); Pos;) {
		QMeshTetra *Tetra = (QMeshTetra*)TetMesh->GetTetraList().GetNext(Pos);

		if (Tetra->GetIndexNo() < 0) continue;
		Tetra->ARAPenergy = 0;
		for (int i = 0; i < 4; i++) {
			QMeshNode * Node = Tetra->GetNodeRecordPtr(i + 1);
			Node->GetCoord3D(pp[0], pp[1], pp[2]);
			Node->GetCoord3D_last(ppl[0], ppl[1], ppl[2]);
			for (int j = 0; j < 3; j++) U(i, j) = pp[j] - ppl[j];
		}

		//Make every U reduce its average displacement
		double U_average[3] = { 0 };
		for (int j = 0; j < 3; j++) {
			for (int i = 0; i < 4; i++) U_average[j] += U(i, j);
		}
		for (int j = 0; j < 3; j++) {
			for (int i = 0; i < 4; i++) U(i, j) = U(i, j) - U_average[j] / 4;
		}

		AU = A*U;
		UTATAU = U.transpose()*AU;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				Tetra->ARAPenergy += UTATAU(i, j) * UTATAU(i, j) * fabs(Tetra->CalVolume_Last()) *100000;
			}
		}
	}

	//Adding filiter method, first compute the Alpha_j for each node.
	for (GLKPOSITION Pos = TetMesh->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode *Node = (QMeshNode*)TetMesh->GetNodeList().GetNext(Pos);
		int neighborNum = 0;
		double ARAPSum = 0;
		for (GLKPOSITION Pos = Node->GetTetraList().GetHeadPosition(); Pos;) {
			QMeshTetra *ConnectedTet = (QMeshTetra*)Node->GetTetraList().GetNext(Pos);
			int fdx = ConnectedTet->GetIndexNo();
			if (fdx < 0) continue;
			ARAPSum += ConnectedTet->ARAPenergy;
			neighborNum++;
		}
		Node->Alpha = ARAPSum / neighborNum;
		//cout << Node->Alpha << endl;
	}

	// After that, compute the updated ARAP energy for tetrahedral.
	double field_distance = 5.0; //for 2-norm neighborhood method
	for (GLKPOSITION Pos = TetMesh->GetTetraList().GetHeadPosition(); Pos;) {
		QMeshTetra *Tetra = (QMeshTetra*)TetMesh->GetTetraList().GetNext(Pos);
		int fdx = Tetra->GetIndexNo();
		if (fdx < 0) continue;
		double ARAP_updated = 0; int field_Num = 0; double distanceSum = 0;
		//1-norm neighborhood
		for (int i = 0; i < 4; i++) {
			ARAP_updated += Tetra->GetNodeRecordPtr(i+1)->Alpha;
		}
		Tetra->ARAPenergy = ARAP_updated;

		//2-norm neighborhood
	/*	for (int i = 0; i < 4; i++) {
			QMeshNode *Node = Tetra->GetNodeRecordPtr(i + 1);
			double TetC[3] = { 0 };
			Tetra->CalCenterPos(TetC[0], TetC[1], TetC[2]);
			for (GLKPOSITION Pos = Node->GetTetraList().GetHeadPosition(); Pos;) {
				QMeshTetra *ConnectedTet = (QMeshTetra*)Node->GetTetraList().GetNext(Pos);
				int fdx2 = ConnectedTet->GetIndexNo(); if (fdx2 < 0) continue;
				for (int j = 0; j < 4; j++) {
					QMeshNode *ConnectedNode = Tetra->GetNodeRecordPtr(i + 1);
					double NodeC[3] = { 0 };
					ConnectedNode->GetCoord3D(NodeC[0], NodeC[1], NodeC[2]);

					double dis = (TetC[0] - NodeC[0])*(TetC[0] - NodeC[0])
						+ (TetC[1] - NodeC[1])*(TetC[1] - NodeC[1])
						+ (TetC[2] - NodeC[2])*(TetC[2] - NodeC[2]);
					double distance = sqrt(dis);
					if (distance > field_distance) continue;
					else {
						ARAP_updated += (field_distance - distance)*ConnectedNode->Alpha;
						distanceSum += field_distance - distance;
						field_Num++;
					}
				}
			}
		}
		cout << field_Num << endl;*/
	}
}

void DeformTet::SensitivityAnalysis_DirectlyDenseChange()
{
	std::vector<double> rExpand;
	rExpand.resize(eleNum);
	int iter_eleNum = 0;

	for (GLKPOSITION Pos = TetMesh->GetTetraList().GetHeadPosition(); Pos;) {
		QMeshTetra *Tetra = (QMeshTetra*)TetMesh->GetTetraList().GetNext(Pos);

		int fdx = Tetra->GetIndexNo();
		if (fdx < 0) continue; //this tetra belongs to rigid or handle region
		if (Tetra->isPeel == true) continue;
		rExpand[iter_eleNum] = Tetra->ARAPenergy;
		Tetra->topopt_index = iter_eleNum; //begin from 0
		iter_eleNum++;
	}
	rExpand.resize(iter_eleNum);
	cout << iter_eleNum << endl;

	//sort the element by ARAP energy
	vector<size_t> idx(rExpand.size());
	iota(idx.begin(), idx.end(), 0);
	//for (int i = 0; i < rExpand.size(); i++) cout << idx[i] << " , value = " << rExpand[idx[i]] << endl;

	sort(idx.begin(), idx.end(),
		[&rExpand](size_t i1, size_t i2) {return rExpand[i1] < rExpand[i2]; });
	//for (int i = 0; i < rExpand.size(); i++) cout << idx[i] << " , value = " << rExpand[idx[i]] << endl;

	//Update the density
	int EleZeroDensityNum = eleNum * ReduceRatio_perIteration; //each iteration peel same amout of tetra.

	for (GLKPOSITION Pos = TetMesh->GetTetraList().GetHeadPosition(); Pos;) {
		QMeshTetra *Tetra = (QMeshTetra*)TetMesh->GetTetraList().GetNext(Pos);
		int fdx = Tetra->GetIndexNo();
		if (fdx < 0) continue; //this tetra belongs to rigid or handle region
		if (Tetra->isPeel == true) continue;
		for (int i = 0; i < EleZeroDensityNum; i++) {
			if (Tetra->topopt_index == idx[i]) {
				Tetra->dense = zeroDensity;
				Tetra->isPeel = true;
			}
		}
		Tetra->topopt_index = -1;
	}
	TetMesh->runTopOpt = true;
}

void DeformTet::DrawARAPEnergyField()
{
	std::vector<double> rExpand;
	rExpand.resize(eleNum);

	for (GLKPOSITION Pos = TetMesh->GetTetraList().GetHeadPosition(); Pos;) {
		QMeshTetra *Tetra = (QMeshTetra*)TetMesh->GetTetraList().GetNext(Pos);
		int fdx = Tetra->GetIndexNo();
		if (fdx < 0) continue; //this tetra belongs to rigid or handle region
		rExpand[fdx] = Tetra->ARAPenergy;
	}

	std::vector<double>::iterator biggest =
		std::max_element(std::begin(rExpand), std::end(rExpand));
	auto smallest = std::min_element(std::begin(rExpand), std::end(rExpand));
	TetMesh->deformDrawValue[0] = *biggest;
	TetMesh->deformDrawValue[1] = *smallest;
	//cout << "the maximum ARAP = " << *biggest << " ,the minimum ARAP = " << *smallest << endl;
	TetMesh->drawDeformationField = true;
}

bool DeformTet::UpdateVertex()
{
	for (GLKPOSITION Pos = TetMesh->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode *node = (QMeshNode*)TetMesh->GetNodeList().GetNext(Pos);
		int idx = node->GetIndexNo();
		if (idx > -1)
		node->SetCoord3D(VectorXPosition[0](idx), VectorXPosition[1](idx), VectorXPosition[2](idx));
	}
	
	return true;
}

void DeformTet::ShifttoInitialPos() {
	double CoordLast[3] = { 0 }, CoordCurrent[3] = { 0 }, shift[3] = {0};
	int nodeNum = 0;
	for (GLKPOSITION Pos = TetMesh->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode *node = (QMeshNode*)TetMesh->GetNodeList().GetNext(Pos);
		if (node->selected == true) continue;
		node->GetCoord3D_last(CoordLast[0], CoordLast[1], CoordLast[2]);
		node->GetCoord3D(CoordCurrent[0], CoordCurrent[1], CoordCurrent[2]);
		for (int i = 0; i < 3; i++) shift[i] += CoordCurrent[i] - CoordLast[i];
		nodeNum++;
	}
	for (int i = 0; i < 3; i++) shift[i] /= (double)nodeNum;
	//cout << nodeNum << " , " << shift[0] <<" , "<< shift[1] << " , " << shift[2] << endl;
	for (GLKPOSITION Pos = TetMesh->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode *node = (QMeshNode*)TetMesh->GetNodeList().GetNext(Pos);
		node->GetCoord3D(CoordCurrent[0], CoordCurrent[1], CoordCurrent[2]);
		node->SetCoord3D(CoordCurrent[0] - shift[0], CoordCurrent[1] - shift[1], CoordCurrent[2] - shift[2]);
		node->CalNormal();
	}
}

void DeformTet::SensitivityAnalysis_DynamicDenseMethod()
{
	//This is the density change method
	double lmin = 0.00001, lmax = 10;

	while (lmax - lmin > 0.0001) {
		double lmid = 0.5*(lmin + lmax);
		double volumeUpdated = 0.0, VolumeConstrain = 0.0;

		for (GLKPOSITION Pos = TetMesh->GetTetraList().GetHeadPosition(); Pos;) {
			QMeshTetra *tetra = (QMeshTetra*)TetMesh->GetTetraList().GetNext(Pos);
			if (tetra->GetIndexNo() < 0) continue;
			double Dense = tetra->dense;
			Dense = max(0.001, max(Dense - 0.2, min(1.0, min(Dense + 0.2, Dense*sqrt(tetra->ARAPenergy / lmid)))));
			tetra->dense = Dense;

			double vol = tetra->CalVolume_Last();
			VolumeConstrain += vol*ReduceConstrainRatio;
			volumeUpdated += vol*tetra->dense;
		}

		cout << "volumeUpdated = " << volumeUpdated << " ,VolumeConstrain = " << VolumeConstrain << endl;

		if (volumeUpdated > VolumeConstrain) lmin = lmid;
		else lmax = lmid;
		cout << "lmin = " << lmin << " ,lmax = " << lmax << endl << endl << endl;
	}
}