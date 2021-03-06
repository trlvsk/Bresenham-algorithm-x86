// graf_io.c : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

typedef struct
{
    int width, height;
    unsigned char* pImg;
    int cX, cY;
    int col;
} imgInfo;

typedef struct
{
    unsigned short bfType;  //-12
    unsigned long  bfSize;  //16
    unsigned short bfReserved1; //-20
    unsigned short bfReserved2; //24
    unsigned long  bfOffBits;   //28
    unsigned long  biSize;  //32
    long  biWidth;  //36
    long  biHeight; //40
    short biPlanes; //44
    short biBitCount; //48
    unsigned long  biCompression; //52
    unsigned long  biSizeImage; //56
    long biXPelsPerMeter; //60
    long biYPelsPerMeter; //64
    unsigned long  biClrUsed; //68
    unsigned long  biClrImportant; //72
    unsigned long  RGBQuad_0; //76
    unsigned long  RGBQuad_1; //80
} bmpHdr;

void* freeResources(FILE* pFile, void* pFirst, void* pSnd)
{
    if (pFile != 0)
        fclose(pFile);
    if (pFirst != 0)
        free(pFirst);
    if (pSnd !=0)
        free(pSnd);
    return 0;
}

imgInfo* readBMP(const char* fname)
{
    imgInfo* pInfo = 0;
    FILE* fbmp = 0;
    bmpHdr bmpHead;
    int lineBytes, y;
    unsigned long imageSize = 0;
    unsigned char* ptr;

    pInfo = 0;
    fbmp = fopen(fname, "rb");
    if (fbmp == 0)
        return 0;

    fread((void *) &bmpHead, sizeof(bmpHead), 1, fbmp);
    // parę sprawdzeń
    if (bmpHead.bfType != 0x4D42 || bmpHead.biPlanes != 1 ||
        bmpHead.biBitCount != 1 || bmpHead.biClrUsed != 2 ||
        (pInfo = (imgInfo *) malloc(sizeof(imgInfo))) == 0)
        return (imgInfo*) freeResources(fbmp, pInfo->pImg, pInfo);

    pInfo->width = bmpHead.biWidth;
    pInfo->height = bmpHead.biHeight;
    imageSize = (((pInfo->width + 31) >> 5) << 2) * pInfo->height;

    if ((pInfo->pImg = (unsigned char*) malloc(imageSize)) == 0)
        return (imgInfo*) freeResources(fbmp, pInfo->pImg, pInfo);

    // process height (it can be negative)
    ptr = pInfo->pImg;
    lineBytes = ((pInfo->width + 31) >> 5) << 2; // line size in bytes
    if (pInfo->height > 0)
    {
        // "upside down", bottom of the image first
        ptr += lineBytes * (pInfo->height - 1);
        lineBytes = -lineBytes;
    }
    else
        pInfo->height = -pInfo->height;

    // reading image
    // moving to the proper position in the file
    if (fseek(fbmp, bmpHead.bfOffBits, SEEK_SET) != 0)
        return (imgInfo*) freeResources(fbmp, pInfo->pImg, pInfo);

    for (y=0; y<pInfo->height; ++y)
    {
        fread(ptr, 1, abs(lineBytes), fbmp);
        ptr += lineBytes;
    }
    fclose(fbmp);
    return pInfo;
}

int saveBMP(const imgInfo* pInfo, const char* fname)
{
    int lineBytes = ((pInfo->width + 31) >> 5)<<2;
    bmpHdr bmpHead =
            {
                    0x4D42,				// unsigned short bfType;
                    sizeof(bmpHdr),		// unsigned long  bfSize;
                    0, 0,				// unsigned short bfReserved1, bfReserved2;
                    sizeof(bmpHdr),		// unsigned long  bfOffBits;
                    40,					// unsigned long  biSize;
                    pInfo->width,		// long  biWidth;
                    pInfo->height,		// long  biHeight;
                    1,					// short biPlanes;
                    1,					// short biBitCount;
                    0,					// unsigned long  biCompression;
                    lineBytes * pInfo->height,	// unsigned long  biSizeImage;
                    11811,				// long biXPelsPerMeter; = 300 dpi
                    11811,				// long biYPelsPerMeter;
                    2,					// unsigned long  biClrUsed;
                    0,					// unsigned long  biClrImportant;
                    0x00000000,			// unsigned long  RGBQuad_0;
                    0x00FFFFFF			// unsigned long  RGBQuad_1;
            };

    FILE * fbmp;
    unsigned char *ptr;
    int y;

    if ((fbmp = fopen(fname, "wb")) == 0)
        return -1;
    if (fwrite(&bmpHead, sizeof(bmpHdr), 1, fbmp) != 1)
    {
        fclose(fbmp);
        return -2;
    }

    ptr = pInfo->pImg + lineBytes * (pInfo->height - 1);
    for (y=pInfo->height; y > 0; --y, ptr -= lineBytes)
        if (fwrite(ptr, sizeof(unsigned char), lineBytes, fbmp) != lineBytes)
        {
            fclose(fbmp);
            return -3;
        }
    fclose(fbmp);
    return 0;
}

/****************************************************************************************/
imgInfo* InitScreen (int w, int h)
{
    imgInfo *pImg;
    if ( (pImg = (imgInfo *) malloc(sizeof(imgInfo))) == 0)
        return 0;
    pImg->height = h;
    pImg->width = w;
    pImg->pImg = (unsigned char*) malloc((((w + 31) >> 5) << 2) * h);
    if (pImg->pImg == 0)
    {
        free(pImg);
        return 0;
    }
    memset(pImg->pImg, 0xFF, (((w + 31) >> 5) << 2) * h);
    pImg->cX = 0;
    pImg->cY = 0;
    pImg->col = 0;
    return pImg;
}

void FreeScreen(imgInfo* pInfo)
{
    if (pInfo && pInfo->pImg)
        free(pInfo->pImg);
    if (pInfo)
        free(pInfo);
}

imgInfo* SetColor_ref(imgInfo* pImg, int col)
{
    pImg->col = col != 0;
    return pImg;
}

imgInfo* MoveTo(imgInfo* pImg, int x, int y)
{
    if (x >= 0 && x < pImg->width)
        pImg->cX = x;
    if (y >= 0 && y < pImg->height)
        pImg->cY = y;
    return pImg;
}

void SetPixel_ref(imgInfo* pImg, int x, int y)
{
    unsigned char *pPix = pImg->pImg + (((pImg->width + 31) >> 5) << 2) * y + (x >> 3);
    unsigned char mask = 0x80 >> (x & 0x07);

    if (x < 0 || x >= pImg->width || y < 0 || y >= pImg->height)
        return;

    if (pImg->col)
        *pPix |= mask;
    else
        *pPix &= ~mask;
}

/// Prototype functions implemented in assembly
imgInfo* DrawCircle_a(imgInfo* pImg, int radius);

imgInfo* SetColor_a(imgInfo* pImg, int col);

imgInfo* MoveTo_a(imgInfo* pImg, int x, int y);


/*imgInfo* DrawCircle_c(imgInfo* pImg, int radius)
{
    // draws circle with center in currnet position and given radius
    int cx = pImg->cX, cy = pImg->cY;
    int d = 5 - 4 * radius, x = 0, y = radius;
    int dltA = (-2*radius+5)*4;
    int dltB = 3*4;

    while (x <= y)
    {
        // 8 symmetric pixels
        SetPixel(pImg, cx-x, cy-y);
        SetPixel(pImg, cx-x, cy+y);
        SetPixel(pImg, cx+x, cy-y);
        SetPixel(pImg, cx+x, cy+y);
        SetPixel(pImg, cx-y, cy-x);
        SetPixel(pImg, cx-y, cy+x);
        SetPixel(pImg, cx+y, cy-x);
        SetPixel(pImg, cx+y, cy+x);
        if (d > 0)
        {
            d += dltA;
            y--;
            x++;
            dltA += 4*4;
            dltB += 2*4;
        }
        else
        {
            d += dltB;
            x++;
            dltA += 2*4;
            dltB += 2*4;
        }
    }
    return pImg;
}
*/
/****************************************************************************************/

int main(int argc, char* argv[])
{
    imgInfo* pInfo;
    int i, j;
    pInfo = readBMP("blank.bmp");
    MoveTo_a(pInfo, 256, 256);
    for (i=3; i < 256; i+=3)
    {
        SetColor_a(pInfo, i & 1);
        DrawCircle_a(pInfo, i);
    }
    saveBMP(pInfo, "result.bmp");
    FreeScreen(pInfo);
    return 0;
}
