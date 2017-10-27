// main.cpp : �������̨Ӧ�ó������ڵ�
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <time.h>
#include "stdafx.h"
#include "stdio.h" 
#include "ComBase.h"
#include "TinClass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CWinApp theApp;
using namespace std;

extern "C" BOOL ztolerancePointSelect(float *psrc, int imgWidth, int imgHeight, double bboxMinX, double bboxMaxY, double dx, double dy, double ztolerValue, float *pdes, float *pIdx);

void GDALPointSelect(BSTR inName, double thresholdVal, BSTR outName)
{
	CString strOutName(outName);
	CComBase comBase;
	int imgWidth = 0, imgHeight = 0;

	cout << "�Ѿ�����PointSelect,������DEMӰ��" << endl;

	if (!comBase.OpenImg(inName, imgWidth, imgHeight))
		return;

	cout << "��һ�δ�DEM��ɣ�" << endl;

	float *pBufferIn = new float[imgWidth*imgHeight];
	double dx = 0, dy = 0, Xmin = 0, Ymax = 0;
	CString prjRef;
	if (!comBase.OpenImg(inName, imgWidth, imgHeight, dx, dy, Xmin, Ymax, prjRef, pBufferIn))
		return;

	cout << "�ڶ��δ�DEM��ɣ�" << endl;

	//����һ���µ�����������DEMӰ��
	float *DEMPtSel = new float[imgWidth*imgHeight];
	memset(DEMPtSel, 0, imgWidth*imgHeight*sizeof(float));

	//���ü�������˳���index��ֵ��Ϣ
	float *DEMPtIdx = NULL;
	DEMPtIdx = new float[imgWidth*imgHeight];
	memset(DEMPtIdx, 0, imgWidth*imgHeight*sizeof(float));

	cout << "��������DEM���������ȡ������" << endl;

	//һ������ÿ���������о������̲߳�ֵ�ĵ㣨ǰ����ÿ���������е����̲߳�ֵ������ֵ������ӵ����������㼯���У�
	ztolerancePointSelect(pBufferIn, imgWidth, imgHeight, Xmin, Ymax, dx, dy, thresholdVal, DEMPtSel, DEMPtIdx);

	cout << "����createNewImg!" << endl;
	for (int i = 0; i < 10; i++)
	{
		cout << DEMPtSel[i] << endl;
	}

	BOOL b = comBase.CreateNewImg(outName, imgWidth, imgHeight, Xmin, Ymax, dx, dy, -9999, prjRef, DEMPtSel);
	if (!b)
	{
		cout << "CreateNewImg failed!" << endl;
	}

	if (DEMPtIdx != NULL)
	{
		cout << "DEMPtIdx != NULL!" << endl;
		BSTR name1 = L"_idx.img";
		CString OutName = strOutName + name1;
		comBase.CreateNewImg(OutName, imgWidth, imgHeight, Xmin, Ymax, dx, dy, -9999, prjRef, DEMPtIdx);
		delete[]DEMPtIdx; DEMPtIdx = NULL;
	}

	delete[]pBufferIn; pBufferIn = NULL;
	delete[]DEMPtIdx; DEMPtIdx = NULL;
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	GDALAllRegister();
	clock_t begin, end;
	double time;
	cout << "��ʼ��ȡ�����㣺" << endl;
	BSTR inName = L"D:\\workstation\\DEM_2980_3515.img";
	BSTR outName = L"D:\\workstation\\DEM_2980_3515_cp.img";
	ofstream timeTxt;
	timeTxt.open("D:\\DEM_2980_3515_18.txt");
	begin = clock();
	GDALPointSelect(inName, 18, outName);
	end = clock();
	time = (double)(end - begin) / CLOCKS_PER_SEC;
	timeTxt << "DEM_2980_3515_18_time:" << time << endl;
	timeTxt << flush;
	timeTxt.close();
	system("pause");
	return 0;
}