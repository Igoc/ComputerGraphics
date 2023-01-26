#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include <Windows.h>

#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#ifndef SAFE_DELETE
    #define SAFE_DELETE(pointer) { if (pointer != nullptr) delete[] pointer; pointer = nullptr; }
#endif

#ifndef CHECK_COORD_VALIDITY
    #define CHECK_COORD_VALIDITY(x, y, width, height) (x >= 0 && y >= 0 && x < width && y < height)
#endif

typedef uint8_t byte_t;

struct PXMINFOHEADER
{
    std::string magicNumber;
    size_t      width;
    size_t      height;
    byte_t      maxLevel;
};

static const size_t IMAGE_WIDTH   = 500;
static const size_t IMAGE_HEIGHT  = 500;
static const int    STEPS         = 3;
static const int    THETA         = 60;
static const float  RADIAN        = THETA * 3.141592F / 180.0F;

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

byte_t* DrawDDALine(byte_t* image, SIZE imageSize, POINT startPoint, POINT endPoint, COLORREF color)
{
    SIZE  variation = { endPoint.x - startPoint.x, endPoint.y - startPoint.y };
    long  step      = (abs(variation.cx) > abs(variation.cy)) ? (abs(variation.cx)) : (abs(variation.cy));
    float markingX  = (float)startPoint.x;
    float markingY  = (float)startPoint.y;
    float increaseX = (float)variation.cx / (float)step;
    float increaseY = (float)variation.cy / (float)step;

    for (int index = 0; index <= step; ++index)
    {
        SetPixel(image, imageSize, { (int)(markingX + 0.5F), (int)(markingY + 0.5F) }, color);

        markingX = markingX + increaseX;
        markingY = markingY + increaseY;
    }

    return image;
}

byte_t* DrawKochCurve(byte_t* image, SIZE imageSize, POINT point1, POINT point2, POINT point3, int steps, COLORREF color)
{
    std::vector<POINT> points;
    size_t             previousSize;

    points.push_back(point1);
    points.push_back(point2);
    points.push_back(point3);
    points.push_back(point1);

    for (int step = 0; step < steps; ++step)
    {
        previousSize = points.size();
        points.resize(points.size() + (points.size() - 1) * 3);

        for (int index = (int)previousSize - 1; index >= 0; --index)
            points[index + index * 3] = points[index];

        for (int index = 0; index < points.size() - 1; index += 4)
        {
            points[index + 1].x = points[index].x + (LONG)((points[index + 4].x - points[index].x) / 3.0 + 0.5);
            points[index + 1].y = points[index].y + (LONG)((points[index + 4].y - points[index].y) / 3.0 + 0.5);

            points[index + 3].x = points[index].x + (LONG)((points[index + 4].x - points[index].x) * 2.0 / 3.0 + 0.5);
            points[index + 3].y = points[index].y + (LONG)((points[index + 4].y - points[index].y) * 2.0 / 3.0 + 0.5);

            points[index + 2].x = (LONG)(points[index + 1].x * cos(RADIAN) - points[index + 1].y * sin(RADIAN) - points[index + 3].x * cos(RADIAN) + points[index + 3].y * sin(RADIAN) + points[index + 3].x + 0.5);
            points[index + 2].y = (LONG)(points[index + 1].x * sin(RADIAN) + points[index + 1].y * cos(RADIAN) - points[index + 3].x * sin(RADIAN) - points[index + 3].y * cos(RADIAN) + points[index + 3].y + 0.5);
        }
    }

    for (int index = 1; index < points.size(); ++index)
        DrawDDALine(image, imageSize, points[index - 1], points[index], color);

    return image;
}

int main(void)
{
    byte_t* image = new byte_t[IMAGE_WIDTH * IMAGE_HEIGHT * 3];

    memset(image, 255, sizeof(byte_t) * IMAGE_WIDTH * IMAGE_HEIGHT * 3);

    DrawKochCurve(image, { IMAGE_WIDTH, IMAGE_HEIGHT }, { 100, 100 }, { 400, 100 }, { 250, 400 }, STEPS, RGB(0, 0, 0));

    WritePXM("Koch Curve.ppm", { "P6", IMAGE_WIDTH, IMAGE_HEIGHT, 255 }, image, true, true);
    SAFE_DELETE(image);

    return 0;
}