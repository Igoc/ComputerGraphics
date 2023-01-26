#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include <Windows.h>

#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

#ifndef SAFE_DELETE
    #define SAFE_DELETE(pointer) { if (pointer != nullptr) delete[] pointer; pointer = nullptr; }
#endif

#ifndef CHECK_COORD_VALIDITY
    #define CHECK_COORD_VALIDITY(x, y, width, height) (x >= 0 && y >= 0 && x < width && y < height)
#endif

typedef uint8_t byte_t;

enum class LINETYPE
{
    SOLID  = 0,
    DASHED = 1,
    DOTTED = 2
};

struct PXMINFOHEADER
{
    std::string magicNumber;
    size_t      width;
    size_t      height;
    byte_t      maxLevel;
};

static const size_t IMAGE_WIDTH  = 500;
static const size_t IMAGE_HEIGHT = 500;

static const bool  SOLID_LINE[8]  = { true, true, true, true, true, true, true, true };
static const bool  DASHED_LINE[8] = { true, true, true, true, false, false, false, false };
static const bool  DOTTED_LINE[8] = { true, false, true, false, true, false, true, false };
static const bool* LINE_STYLES[3] = { SOLID_LINE, DASHED_LINE, DOTTED_LINE };

void WritePXM(const char* filePath, PXMINFOHEADER pxmInfoHeader, byte_t* image, bool isColor, bool isBinary)
{
    const char* fileMode    = (isBinary == true) ? ("w+b") : ("w+t");
    FILE*       fileStream  = fopen(filePath, fileMode);
    size_t      bitPerPixel = (isColor == false) ? (1) : (3);

    fprintf(fileStream, "%s\n",      pxmInfoHeader.magicNumber.data());
    fprintf(fileStream, "%zd %zd\n", pxmInfoHeader.width, pxmInfoHeader.height);
    fprintf(fileStream, "%d\n",      pxmInfoHeader.maxLevel);
    fwrite(image, sizeof(byte_t) * bitPerPixel, pxmInfoHeader.width * pxmInfoHeader.height, fileStream);

    fclose(fileStream);
}

bool SetPixel(byte_t* image, SIZE imageSize, POINT point, COLORREF color)
{
    if (CHECK_COORD_VALIDITY(point.x, point.y, imageSize.cx, imageSize.cy) == false)
        return false;

    image[point.y * imageSize.cx * 3 + point.x * 3 + 0] = GetRValue(color);
    image[point.y * imageSize.cx * 3 + point.x * 3 + 1] = GetGValue(color);
    image[point.y * imageSize.cx * 3 + point.x * 3 + 2] = GetBValue(color);

    return true;
}

byte_t* DrawDDALine(byte_t* image, SIZE imageSize, POINT startPoint, POINT endPoint, COLORREF color, LINETYPE lineType)
{
    SIZE  variation = { endPoint.x - startPoint.x, endPoint.y - startPoint.y };
    long  step      = (abs(variation.cx) > abs(variation.cy)) ? (abs(variation.cx)) : (abs(variation.cy));
    float markingX  = (float)startPoint.x;
    float markingY  = (float)startPoint.y;
    float increaseX = (float)variation.cx / (float)step;
    float increaseY = (float)variation.cy / (float)step;

    for (int index = 0; index <= step; ++index)
    {
        if (LINE_STYLES[(int)lineType][index % 8] == true)
            SetPixel(image, imageSize, { (int)(markingX + 0.5F), (int)(markingY + 0.5F) }, color);

        markingX = markingX + increaseX;
        markingY = markingY + increaseY;
    }

    return image;
}

int main(void)
{
    byte_t* image = new byte_t[IMAGE_WIDTH * IMAGE_HEIGHT * 3];

    memset(image, 255, sizeof(byte_t) * IMAGE_WIDTH * IMAGE_HEIGHT * 3);

    DrawDDALine(image, { IMAGE_WIDTH, IMAGE_HEIGHT }, { 50, 50 },   { 250, 250 }, RGB(0, 0, 0),   LINETYPE::SOLID);
    DrawDDALine(image, { IMAGE_WIDTH, IMAGE_HEIGHT }, { 400, 300 }, { 0, 0 },     RGB(255, 0, 0), LINETYPE::SOLID);
    DrawDDALine(image, { IMAGE_WIDTH, IMAGE_HEIGHT }, { 200, 100 }, { 250, 250 }, RGB(0, 255, 0), LINETYPE::DASHED);
    DrawDDALine(image, { IMAGE_WIDTH, IMAGE_HEIGHT }, { 475, 475 }, { 125, 250 }, RGB(0, 0, 255), LINETYPE::DOTTED);

    WritePXM("DDA Line.ppm", { "P6", IMAGE_WIDTH, IMAGE_HEIGHT, 255 }, image, true, true);
    SAFE_DELETE(image);

    return 0;
}