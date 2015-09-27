#pragma once
#include "PTD_define.h"
#include <vector>
#include <fstream>

struct PicInfo
{
    int width;
    int height;
    int length;
    string picPath;
};

class CCreatePdf
{
public:
    ~CCreatePdf();
    static CCreatePdf* GetInstance();
    bool CombineJPGToPDF();
    void AddPictures(string picPath);
    void WritePdfHeader();
    void AddPdfPath(string pdfPath);

private:
    CCreatePdf();
    //GetImageInfo(FILE* picFile, );

private:
    vector<PicInfo> m_PicInfo;
    string m_PdfPath;
    ofstream m_PdfFile;
};

