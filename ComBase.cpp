// ComBase.cpp: implementation of the CComBase class
//

#include "stdafx.h"
#include "resource.h"
#include "ComBase.h"

#include <string>
#include <cstringt.h>
using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CComBase::CComBase()
{

}

CComBase::~CComBase()
{

}

//���Ӱ��ĺ�׺��
CString CComBase::GetImgFormat(LPCTSTR lpstrFileName)
{
	string saveFileName = string((char*)lpstrFileName);

	string suffix = "";

	if (saveFileName.length() != 0)
	{
		const char* charName;
		charName = saveFileName.data();
		charName = strrchr(charName, '.');
		if (charName)
			suffix = charName + 1;
	}

	CString result(suffix.c_str());

	return result;
}

//ͨ��Ӱ��ĺ�׺���õ���ӦGDAL�ܴ����������
CString CComBase::GetGDALFormat(CString imgFormat)
{
	CString strFormat;

	if (imgFormat.GetLength() == 0)
	{
		return strFormat;
	}

	if (imgFormat == "bmp")
		strFormat = "BMP";
	else if (imgFormat == "bt")
		strFormat = "BT";
	else if (imgFormat == "gif")
		strFormat = "GIF";
	else if (imgFormat == "img")
		strFormat = "HFA";
	else if (imgFormat == "jpg")
		strFormat = "JPEG";
	else if (imgFormat == "png")
		strFormat = "PNG";
	else if (imgFormat == "tif")
		strFormat = "GTiff";
	else if (imgFormat == "vrt")
		strFormat = "VRT";

	return strFormat;
}

//�ͷ�GDALDataset���ݼ�
void  CComBase::RELEASE(GDALDataset* pData)
{
	if (pData != NULL)
	{
		GDALClose((GDALDatasetH)pData);
		pData = NULL;
	}
}

//�ж������strImgName�Ƿ���Ч���˴���imgName֧��*.tif & *.img��ʽ
BOOL  CComBase::bImgNameVerf(CString strImgName, CString& imgFormat, CString& strFormat)
{
	if (strImgName.GetLength()<1)	return FALSE;

	imgFormat = GetImgFormat(strImgName);
	strFormat = GetGDALFormat(imgFormat);

	return TRUE;
}


//����һ���µ�Ӱ��(��ͶӰ��Ϣ)������True˵�������ɹ������򴴽�ʧ��
//ImgName:����Ӱ���·��
//imgWidth��imgHeight:����Ӱ��Ŀ�͸�
//Xmin��Ymax:����Ӱ����X��Y�ķ�Χ
//dx��dy:����Ӱ��ĸ����ֱ���
//invalidVal:����Ӱ���е���Чֵ
//demZ:����Ӱ����ÿ�����ص�ֵ
BOOL  CComBase::CreateNewImg(CString strImgName, int imgWidth, int imgHeight, double Xmin, double Ymax, double dx, double dy, double invalidVal, CString projRef, float *demZ)
{
	GDALAllRegister();

	//������Ӱ���ʽ�Ƿ���ȷ
	CString imgFormat, strFormat;
	if (bImgNameVerf(strImgName, imgFormat, strFormat) == FALSE)
	{
		printf("Ӱ���ʽ����\n");
		return FALSE;
	}

	GDALDriverH hDriver = NULL;
	USES_CONVERSION;
	LPSTR charFormat = T2A(strFormat);
	LPCSTR charImgName = T2A(strImgName);

	hDriver = GetGDALDriverManager()->GetDriverByName("HFA");
	if (hDriver == NULL) printf("hDriver == NULL \n");
	if (hDriver == NULL || GDALGetMetadataItem(hDriver, GDAL_DCAP_CREATE, NULL) == NULL)
	{
		printf("hDriver == NULL || GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATE, NULL ) == NULL\n");
		return FALSE;
	}

	char **papszOptions = NULL;
	GDALDataset *pDataset = (GDALDataset *)GDALCreate(hDriver, charImgName, imgWidth, imgHeight, 1, GDT_Float32, papszOptions);

	double adfGeoTransform[6] = { Xmin, dx, 0, Ymax, 0, -dy };
	pDataset->SetGeoTransform(adfGeoTransform);
	LPCSTR charprojRef = T2A(projRef);

	pDataset->SetProjection(charprojRef);
	GDALRasterBand *pBand = pDataset->GetRasterBand(1);

	pBand->SetNoDataValue((double)invalidVal);  //���ò��ε���Чֵ����������Ӱ��=invalidValΪ��Чֵ
	pBand->RasterIO(GF_Write, 0, 0, imgWidth, imgHeight, demZ, imgWidth, imgHeight, GDT_Float32, 0, 0);
	double min = 0, max = 0, mean = 0, dev = 0; //���õ�Ӱ������ֵ����Сֵ
	pBand->ComputeStatistics(FALSE, &min, &max, &mean, &dev, NULL, NULL);

	GDALDeleteDataset(hDriver, charImgName);
	RELEASE(pDataset);

	return TRUE;
}


//��һӰ�����ݣ���ø�Ӱ�����Ϣ������True˵�������ɹ������򴴽�ʧ��
//ImgName:����Ӱ���·��
//imgWidth,imgHeight:����Ӱ��Ŀ���
BOOL  CComBase::OpenImg(BSTR ImgName, int& imgWidth, int& imgHeight)
{
	CString strImgName(ImgName);
	if (strImgName.GetLength()<1)
	{
		cout << "length < 1,return FALSE!" << endl;
		return FALSE;
	}

	//��DEMӰ��    
	GDALDataset *pImgDataset = NULL;
	USES_CONVERSION;
	LPCSTR charImgName = T2A(strImgName);

	int len = strlen(charImgName);

	pImgDataset = (GDALDataset *)GDALOpen(charImgName, GA_ReadOnly);
	if (pImgDataset == NULL)
	{
		cout << "Img cannot open here!" << endl;
		return FALSE;
	}

	imgHeight = pImgDataset->GetRasterYSize();
	imgWidth = pImgDataset->GetRasterXSize();

	cout << "DEM�Ѿ���," << "��Ϊ" << imgHeight << ",��Ϊ" << imgWidth << "!" << endl;

	RELEASE(pImgDataset);

	if (imgHeight <= 0 || imgWidth <= 0)
		return FALSE;

	return TRUE;
}

//��һӰ�����ݣ���ø�Ӱ�����Ϣ(����ͶӰ��Ϣ)������True˵�������ɹ������򴴽�ʧ��
//ImgName:����Ӱ���·����
//imgWidth,imgHeight:Ӱ��Ŀ��ߣ�
//dx,dy:����Ӱ��ķֱ��ʣ�
//Xmin,Ymax:����Ӱ������귶Χ��
//pBuffer:����Ӱ��ĸ�������ֵ��
BOOL  CComBase::OpenImg(BSTR ImgName, int imgWidth, int imgHeight, double& dx, double& dy, double& Xmin, double& Ymax, CString& projRef, float *pBuffer)
{
	CString strImgName(ImgName);
	if (strImgName.GetLength()<1 || pBuffer == NULL)
	{
		cout << "strImgName.GetLength()<1||pBuffer==NULL,return FALSE!" << endl;
		return FALSE;
	}

	//��DEMӰ��    
	GDALDataset *pImgDataset = NULL;

	USES_CONVERSION;
	LPCSTR charImgName = T2A(strImgName);

	pImgDataset = (GDALDataset *)GDALOpen(charImgName, GA_ReadOnly);
	if (pImgDataset == NULL)
	{
		cout << "img cannot open!" << endl;
		return FALSE;
	}

	double geoTransform[6];
	pImgDataset->GetGeoTransform(geoTransform);  //���DEM�����귶Χ���Լ����طֱ���
	dx = geoTransform[1], dy = fabs(geoTransform[5]);
	Xmin = geoTransform[0], Ymax = geoTransform[3];

	projRef = pImgDataset->GetProjectionRef();

	int BandNum = pImgDataset->GetRasterCount();
	GDALRasterBand *pBand = pImgDataset->GetRasterBand(BandNum);
	if (pBand == NULL)
	{
		RELEASE(pImgDataset);
		return FALSE;
	}
	pBand->RasterIO(GF_Read, 0, 0, imgWidth, imgHeight, pBuffer, imgWidth, imgHeight, GDT_Float32, 0, 0);

	RELEASE(pImgDataset);

	if (pBuffer == NULL || imgWidth <= 0 || imgHeight <= 0 || dx <= 0 || dy <= 0)
		return FALSE;

	return TRUE;
}

//��֪�����������������ε����
double CComBase::AreaTrig(double x1, double y1, double x2, double y2, double x3, double y3)
{
	double a = sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
	double b = sqrt((x1 - x3)*(x1 - x3) + (y1 - y3)*(y1 - y3));
	double c = sqrt((x3 - x2)*(x3 - x2) + (y3 - y2)*(y3 - y2));

	double p = (a + b + c) / 2;

	double area = sqrt(p*(p - a)*(p - b)*(p - c));
	return area;
}

//�жϵ�(cenX,cenY,cenZ)�Ƿ���������(xt,yt,zt)����(�����߽�)���ж�һ���㵽�����θ��������������Ƿ�������������
BOOL CComBase::IsInTriangle(double cenX, double cenY, double* xt, double* yt)
{
	double c01 = AreaTrig(cenX, cenY, xt[0], yt[0], xt[1], yt[1]);
	double c02 = AreaTrig(cenX, cenY, xt[0], yt[0], xt[2], yt[2]);
	double c21 = AreaTrig(cenX, cenY, xt[2], yt[2], xt[1], yt[1]);

	double trig = AreaTrig(xt[0], yt[0], xt[1], yt[1], xt[2], yt[2]);

	if (fabs(trig - (c01 + c02 + c21))<0.00001)
		return true;

	return false;
}

//�жϵ�(cenX,cenY,cenZ)�Ƿ���������(xt,yt,zt)�ı߽��ϡ��ж�һ���㵽�������˵������Ƿ�����ߵľ���
BOOL CComBase::IsInEdge(double cenX, double cenY, double* xt, double* yt)
{
	double dis01 = sqrt((xt[0] - xt[1])*(xt[0] - xt[1]) + (yt[0] - yt[1])*(yt[0] - yt[1]));
	double dis02 = sqrt((xt[0] - xt[2])*(xt[0] - xt[2]) + (yt[0] - yt[2])*(yt[0] - yt[2]));
	double dis12 = sqrt((xt[2] - xt[1])*(xt[2] - xt[1]) + (yt[2] - yt[1])*(yt[2] - yt[1]));

	double disc0 = sqrt((cenX - xt[0])*(cenX - xt[0]) + (cenY - yt[0])*(cenY - yt[0]));
	double disc1 = sqrt((cenX - xt[1])*(cenX - xt[1]) + (cenY - yt[1])*(cenY - yt[1]));
	double disc2 = sqrt((cenX - xt[2])*(cenX - xt[2]) + (cenY - yt[2])*(cenY - yt[2]));

	if (fabs((disc0 + disc1) - dis01)<0.00001 || fabs((disc0 + disc2) - dis02)<0.00001 || fabs((disc2 + disc1) - dis12)<0.00001)
		return true;

	return false;
}