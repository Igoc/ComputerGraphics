#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include <Windows.h>

#include <cinttypes>
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

byte_t* DrawCircle(byte_t* image, SIZE imageSize, POINT centerPoint, LONG radius, COLORREF color)
{
    POINT symmetryPoint = { 0, radius };
    int   discriminant  = 1 - radius;

    while (symmetryPoint.x <= symmetryPoint.y)
    {
        SetPixel(image, imageSize, { centerPoint.x + symmetryPoint.x, centerPoint.y + symmetryPoint.y }, color);
        SetPixel(image, imageSize, { centerPoint.x + symmetryPoint.x, centerPoint.y - symmetryPoint.y }, color);
        SetPixel(image, imageSize, { centerPoint.x - symmetryPoint.x, centerPoint.y + symmetryPoint.y }, color);
        SetPixel(image, imageSize, { centerPoint.x - symmetryPoint.x, centerPoint.y - symmetryPoint.y }, color);
        SetPixel(image, imageSize, { centerPoint.x + symmetryPoint.y, centerPoint.y + symmetryPoint.x }, color);
        SetPixel(image, imageSize, { centerPoint.x + symmetryPoint.y, centerPoint.y - symmetryPoint.x }, color);
        SetPixel(image, imageSize, { centerPoint.x - symmetryPoint.y, centerPoint.y + symmetryPoint.x }, color);
        SetPixel(image, imageSize, { centerPoint.x - symmetryPoint.y, centerPoint.y - symmetryPoint.x }, color);

        symmetryPoint.x += 1;

        if (discriminant < 0)
            discriminant += 2 * symmetryPoint.x + 1;
        else
        {
            symmetryPoint.y -= 1;
            discriminant    += 2 * (symmetryPoint.x - symmetryPoint.y) + 1;
        }
    }

    return image;
}

int main(void)
{
    byte_t* image = new byte_t[IMAGE_WIDTH * IMAGE_HEIGHT * 3];

    memset(image, 255, sizeof(byte_t) * IMAGE_WIDTH * IMAGE_HEIGHT * 3);

    DrawCircle(image, { IMAGE_WIDTH, IMAGE_HEIGHT }, { 150, 150 }, 100, RGB(255, 0, 0));
    DrawCircle(image, { IMAGE_WIDTH, IMAGE_HEIGHT }, { 300, 250 }, 200, RGB(0, 255, 0));
    DrawCircle(image, { IMAGE_WIDTH, IMAGE_HEIGHT }, { 125, 225 }, 150, RGB(0, 0, 255));

    WritePXM("Circle.ppm", { "P6", IMAGE_WIDTH, IMAGE_HEIGHT, 255 }, image, true, true);
    SAFE_DELETE(image);

    return 0;
}