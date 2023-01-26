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

byte_t* DrawEllipse(byte_t* image, SIZE imageSize, POINT centerPoint, SIZE radius, LONG theta, COLORREF color)
{
    SIZE  squaredRadius     = { radius.cx * radius.cx, radius.cy * radius.cy };
    POINT symmetryPoint     = { 0, radius.cy };
    POINT discriminantPoint = { 0, 2 * squaredRadius.cx * radius.cy };
    float radian            = theta * 3.141592F / 180.0F;
    int   discriminant;

    discriminant = (int)(squaredRadius.cy - squaredRadius.cx * radius.cy + 0.25 * squaredRadius.cx + 0.5);

    while (discriminantPoint.x <= discriminantPoint.y)
    {
        SetPixel(image, imageSize, { (int)(centerPoint.x + symmetryPoint.x * cos(radian) - symmetryPoint.y * sin(radian)),  (int)(centerPoint.y + symmetryPoint.x * sin(radian)  + symmetryPoint.y * cos(radian)) }, color);
        SetPixel(image, imageSize, { (int)(centerPoint.x + symmetryPoint.x * cos(radian) - symmetryPoint.y * sin(-radian)), (int)(centerPoint.y - symmetryPoint.x * sin(-radian) - symmetryPoint.y * cos(radian)) }, color);
        SetPixel(image, imageSize, { (int)(centerPoint.x - symmetryPoint.x * cos(radian) + symmetryPoint.y * sin(-radian)), (int)(centerPoint.y + symmetryPoint.x * sin(-radian) + symmetryPoint.y * cos(radian)) }, color);
        SetPixel(image, imageSize, { (int)(centerPoint.x - symmetryPoint.x * cos(radian) + symmetryPoint.y * sin(radian)),  (int)(centerPoint.y - symmetryPoint.x * sin(radian)  - symmetryPoint.y * cos(radian)) }, color);

        symmetryPoint.x     += 1;
        discriminantPoint.x += 2 * squaredRadius.cy;

        if (discriminant < 0)
            discriminant += squaredRadius.cy + discriminantPoint.x;
        else
        {
            symmetryPoint.y     -= 1;
            discriminantPoint.y -= 2 * squaredRadius.cx;
            discriminant        += squaredRadius.cy + discriminantPoint.x - discriminantPoint.y;
        }
    }

    discriminant = (int)(squaredRadius.cy * (symmetryPoint.x + 0.5) * (symmetryPoint.x + 0.5) + squaredRadius.cx * (symmetryPoint.y - 1) * (symmetryPoint.y - 1) - squaredRadius.cx * squaredRadius.cy);

    while (symmetryPoint.y > 0)
    {
        symmetryPoint.y     -= 1;
        discriminantPoint.y -= 2 * squaredRadius.cx;

        if (discriminant > 0)
            discriminant += squaredRadius.cx - discriminantPoint.y;
        else
        {
            symmetryPoint.x     += 1;
            discriminantPoint.x += 2 * squaredRadius.cy;
            discriminant        += squaredRadius.cx + discriminantPoint.x - discriminantPoint.y;
        }

        SetPixel(image, imageSize, { (int)(centerPoint.x + symmetryPoint.x * cos(radian) - symmetryPoint.y * sin(radian)),  (int)(centerPoint.y + symmetryPoint.x * sin(radian)  + symmetryPoint.y * cos(radian)) }, color);
        SetPixel(image, imageSize, { (int)(centerPoint.x + symmetryPoint.x * cos(radian) - symmetryPoint.y * sin(-radian)), (int)(centerPoint.y - symmetryPoint.x * sin(-radian) - symmetryPoint.y * cos(radian)) }, color);
        SetPixel(image, imageSize, { (int)(centerPoint.x - symmetryPoint.x * cos(radian) + symmetryPoint.y * sin(-radian)), (int)(centerPoint.y + symmetryPoint.x * sin(-radian) + symmetryPoint.y * cos(radian)) }, color);
        SetPixel(image, imageSize, { (int)(centerPoint.x - symmetryPoint.x * cos(radian) + symmetryPoint.y * sin(radian)),  (int)(centerPoint.y - symmetryPoint.x * sin(radian)  - symmetryPoint.y * cos(radian)) }, color);
    }

    return image;
}

int main(void)
{
    byte_t* image = new byte_t[IMAGE_WIDTH * IMAGE_HEIGHT * 3];

    memset(image, 255, sizeof(byte_t) * IMAGE_WIDTH * IMAGE_HEIGHT * 3);

    DrawEllipse(image, { IMAGE_WIDTH, IMAGE_HEIGHT }, { 150, 150 }, { 100, 200 }, 0,   RGB(255, 0, 0));
    DrawEllipse(image, { IMAGE_WIDTH, IMAGE_HEIGHT }, { 300, 250 }, { 50, 150 },  75,  RGB(0, 255, 0));
    DrawEllipse(image, { IMAGE_WIDTH, IMAGE_HEIGHT }, { 125, 225 }, { 175, 150 }, 120, RGB(0, 0, 255));

    WritePXM("Ellipse.ppm", { "P6", IMAGE_WIDTH, IMAGE_HEIGHT, 255 }, image, true, true);
    SAFE_DELETE(image);

    return 0;
}