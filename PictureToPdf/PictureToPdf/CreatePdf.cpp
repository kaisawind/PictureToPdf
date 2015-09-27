#include "stdafx.h"
#include "CreatePdf.h"


CCreatePdf::CCreatePdf()
{
    m_PicInfo.clear();
    m_PdfPath = "a.pdf";
}


CCreatePdf::~CCreatePdf()
{
}

CCreatePdf* CCreatePdf::GetInstance()
{
    static CCreatePdf obj;
    return &obj;
}

bool CCreatePdf::CombineJPGToPDF()
{
    bool ret = true;
    vector<PicInfo>::iterator picPath = m_PicInfo.begin();
    
    WritePdfHeader();

    return ret;
}

void CCreatePdf::AddPictures(string picPath)
{
    RETURN_TYPE ret = RETURN_OK;
    PicInfo temp = { 0 };
    temp.picPath = picPath;

    // 打开图片文件
    FILE *picFile = fopen(temp.picPath.c_str(), "rb");
    if (picFile == NULL)
    {
        MessageBox(NULL, TEXT("无法打开PDF路径"), TEXT("ERROR"), MB_OK);
        ret = RETURN_NG;
    }
    else
    {
        // 计算文件BYTE数
        fseek(picFile, 0, SEEK_END);
        temp.length = ftell(picFile);
        fseek(picFile, 0, SEEK_SET);

        // 图片的Buffer
        UINT8 *picBuffer = new UINT8[temp.length];
        fread(picBuffer, temp.length, 1, picFile);
        fclose(picFile);

        // 获取图片大小
        for (int index = 0; index < temp.length; index++)
        {
            if (0xC0 == picBuffer[index])
            {
                if (0xFF == picBuffer[index - 1])
                {
                    temp.height = picBuffer[index + 4] << 8;
                    temp.height += picBuffer[index + 5];
                    temp.width = picBuffer[index + 6] << 8;
                    temp.width += picBuffer[index + 7];
                    break;
                }
            }
        }

        m_PicInfo.push_back(temp);

        delete[]picBuffer;
    }
    return;
}

void CCreatePdf::AddPdfPath(string pdfPath)
{
    m_PdfPath = pdfPath;
    return;
}

void CCreatePdf::WritePdfHeader()
{
    FILE *outFile = NULL;
    FILE *jpgFile = NULL;
    RETURN_TYPE ret = RETURN_OK;
    unsigned int i = 0;
    int j = 0;
    unsigned fileLen = 0;
    char imgID[64];
    char buffer[32 * 1024];
    unsigned int objectOffset[1024];
    memset(objectOffset, 0, sizeof(objectOffset));
    unsigned int temp, totalLen = 0, objectCount = 1;
    int width, height, offset;
    char *buf = NULL;
    double scale = 1.0;

    // 图片的个数
    int pdfCount = m_PicInfo.size();    

    outFile = fopen(m_PdfPath.c_str(), "wb");
    if (outFile == NULL)
    {
        MessageBox(NULL, TEXT("无法打开PDF路径"), TEXT("ERROR"), MB_OK);
        ret = RETURN_NG;
    }
    else
    {
        //write pdf header
        //PDF_HEADER      "%%PDF-%1.1f\n"
        sprintf(buffer, PDF_HEADER, PDF_VERESION);
        totalLen += strlen(buffer);
        fputs(buffer, outFile);

        //write pdf catalog
        //PDF_CATALOG     "%d 0 obj\n<<\n/Type /Catalog\n/Version /%1.1f\n/Pages %d 0 R\n>>\nendobj\n"
        sprintf(buffer, PDF_CATALOG, objectCount, PDF_VERESION, objectCount + 2);
        objectOffset[objectCount++] = totalLen;
        totalLen += strlen(buffer);
        fputs(buffer, outFile);

        //write pdf info
        //PDF_INFO        "%d 0 obj\n<<\n/Author (%s)\n/Creator (%s)\n/Producer (%s)\n>>\nendobj\n"
        sprintf(buffer, PDF_INFO, objectCount, PDF_AUTHOR, PDF_CREATER, PDF_PRODUCER);
        objectOffset[objectCount++] = totalLen;
        totalLen += strlen(buffer);
        fputs(buffer, outFile);

        //write pdf page
        buf = (char *)malloc(10 * pdfCount);
        for (j = 0, offset = 0; j < pdfCount; j++)
        {
            sprintf(buf + offset, "%d 0 R ", j * 4 + 4);
            offset += strlen(buf + offset);
        }
        //PDF_PAGES       "%d 0 obj\n<<\n/Type /Pages\n/Kids [%s]\n/Count %d\n>>\nendobj\n"
        sprintf(buffer, PDF_PAGES, objectCount, buf, pdfCount);
        objectOffset[objectCount++] = totalLen;
        totalLen += strlen(buffer);
        fputs(buffer, outFile);
        free(buf);
        buf = NULL;

        for (j = 0; j < pdfCount; j++)
        {

            jpgFile = fopen(m_PicInfo[j].picPath.c_str(), "rb");
            if (jpgFile == NULL)
            {
                MessageBox(NULL, TEXT("打开图片失败"), TEXT("ERROR"), MB_OK);
                ret = RETURN_NG;
            }
            else
            {
                width = m_PicInfo[j].width;
                height = m_PicInfo[j].height;
                sprintf(imgID, "Im%d", j + 1);

                //write pdf kids
                //PDF_PAGES_KID   "%d 0 obj\n<<\n/Type /Page\n/Parent %d 0 R\n/MediaBox [0 0 %3.4f %3.4f]\n/Contents %d 0 R\n/Resources <<\n/XObject <<\n/%s %d 0 R >>\n/ProcSet [/ImageC]\n>>\n>>\nendobj\n"
                sprintf(buffer, PDF_PAGES_KID, objectCount, 3, width*1.0, height*1.0, objectCount + 1, imgID, objectCount + 3);
                objectOffset[objectCount++] = totalLen;
                totalLen += strlen(buffer);
                fputs(buffer, outFile);

                // 图片大小比例变换 暂时没有使用
                if (width > PDF_X_PIXELS || height > PDF_Y_PIXELS)
                {
                    if (PDF_X_PIXELS*1.0 / width < PDF_Y_PIXELS*1.0 / height)
                    {
                        scale = PDF_X_PIXELS*1.0 / width;
                    }
                    else
                    {
                        scale = PDF_Y_PIXELS*1.0 / height;
                    }
                }
                else
                {
                    scale = 1.0;
                }

                //write pdf contents
                //PDF_CONTENTS    "%d 0 obj\n<<\n/Length %d 0 R\n>>\nstream\nq  %3.4f 0.0000 0.0000 %3.4f 0.0000 0.0000 cm /%s Do Q\nendstream\nendobj\n"
                sprintf(buffer, PDF_CONTENTS, objectCount, objectCount + 1, width*1.0, height*1.0, imgID);
                objectOffset[objectCount++] = totalLen;
                totalLen += strlen(buffer);
                fputs(buffer, outFile);

                //write pdf object length
                //PDF_LENGTH      "%d 0 obj\n%d\nendobj\n"
                sprintf(buffer, PDF_LENGTH, objectCount, 60);
                objectOffset[objectCount++] = totalLen;
                totalLen += strlen(buffer);
                fputs(buffer, outFile);

                fseek(jpgFile, 0, SEEK_END);
                fileLen = ftell(jpgFile);
                fseek(jpgFile, 0, SEEK_SET);

                //write data header
                //PDF_JPG_DATA_HEADER "%d 0 obj\n<<\n/Length %d\n/Type /XObject\n/Subtype /Image\n/Name /%s\n/Width %d\n/Height %d\n/BitsPerComponent %d\n/ColorSpace /DeviceRGB\n/Filter /DCTDecode >>\nstream\n"
                sprintf(buffer, PDF_JPG_DATA_HEADER, objectCount, fileLen, imgID, width, height, 8);
                objectOffset[objectCount++] = totalLen;
                totalLen += strlen(buffer);
                fputs(buffer, outFile);

                while (fileLen > 0)
                {
                    temp = fileLen > sizeof(buffer) ? sizeof(buffer) : fileLen;
                    if (temp != fread(buffer, 1, temp, jpgFile))
                    {
                        printf("read file error\n");
                        ret = RETURN_NG;
                        break;
                    }

                    if (temp != fwrite(buffer, 1, temp, outFile))
                    {
                        printf("write file error\n");
                        ret = RETURN_NG;
                        break;
                    }

                    fileLen -= temp;
                    totalLen += temp;
                }

                //write data ender
                //PDF_JPG_DATA_ENDER  "\nendstream\nendobj\n"
                sprintf(buffer, PDF_JPG_DATA_ENDER);
                totalLen += strlen(buffer);
                fputs(buffer, outFile);

                fclose(jpgFile);
            }
        }

        buf = (char *)malloc(25 * objectCount);

        sprintf(buf, "%010d 65535 f\r\n", objectOffset[0], 0);

        for (i = 1; i < objectCount; i++)
        {
            sprintf(buf + i * 20, "%010d %05d n\r\n", objectOffset[i], 0);
        }

        //write pdf xref
        // PDF_XREF        "xref\r\n0 %d\r\n%s\r\ntrailer\r\n<<\r\n/Size %d\r\n/Root %d 0 R\r\n/Info %d 0 R\r\n/ID[<2900000023480000FF180000FF670000><2900000023480000FF180000FF670000>]\r\n>>\r\nstartxref\r\n%d\r\n%%%%EOF"
        sprintf(buffer, PDF_XREF, objectCount, buf, objectCount, 1, 2, totalLen);
        objectOffset[objectCount++] = totalLen;
        totalLen += strlen(buffer);
        fputs(buffer, outFile);

        free(buf);
        buf = NULL;

        fclose(outFile);
    }

    return;
}
