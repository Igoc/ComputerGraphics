#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include <Windows.h>

#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
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

class Math
{
public:
    static int Factorial(int value)
    {
        int result = 1;

        for (int index = 1; index < value + 1; ++index)
            result = result * index;

        return result;
    }

    static double Combination(int elementNumber, int selectionNumber)
    {
        if (selectionNumber > elementNumber)
            return 0.0;

        return (double)Factorial(elementNumber) / (double)(Factorial(selectionNumber) * Factorial(elementNumber - selectionNumber));
    }
};

static const size_t IMAGE_WIDTH  = 500;
static const size_t IMAGE_HEIGHT = 500;
static const int    STEPS        = 1000;

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

byte_t* DrawBezierSpline(byte_t* image, SIZE imageSize, std::vector<POINT> points, int steps, COLORREF color)
{
    std::vector<POINT> sectionPoints;
    double             stepX, stepY;

    for (double step = 0.0; step <= 1.0; step += 1.0 / steps)
    {
        stepX = 0.0;
        stepY = 0.0;

        for (int index = 0; index < (int)points.size(); ++index)
        {
            stepX += Math::Combination((int)points.size() - 1, index) * pow(step, index) * pow(1.0 - step, (int)points.size() - index - 1) * points[index].x;
            stepY += Math::Combination((int)points.size() - 1, index) * pow(step, index) * pow(1.0 - step, (int)points.size() - index - 1) * points[index].y;
        }

        sectionPoints.push_back({ (LONG)(stepX + 0.5), (LONG)(stepY + 0.5) });
    }

    for (int index = 1; index < sectionPoints.size(); ++index)
        DrawDDALine(image, imageSize, sectionPoints[index - 1], sectionPoints[index], color);

    return image;
}

int main(void)
{
    byte_t*            image = new byte_t[IMAGE_WIDTH * IMAGE_HEIGHT * 3];
    POINT              point;
    std::vector<POINT> points;
    int                pointNumber;

    do
    {
        std::cout << "Number of Points (4 to 9): ";
        std::cin >> pointNumber;
    } while (pointNumber < 4 || pointNumber > 9);

    for (int index = 0; index < pointNumber; ++index)
    {
        std::cout << "Coordinates of X" << index + 1 << " (0 to " << IMAGE_WIDTH  - 1 << "): ";
        std::cin  >> point.x;

        std::cout << "Coordinates of Y" << index + 1 << " (0 to " << IMAGE_HEIGHT - 1 << "): ";
        std::cin  >> point.y;

        if (CHECK_COORD_VALIDITY(point.x, point.y, IMAGE_WIDTH, IMAGE_HEIGHT) == false)
            index = index - 1;
        else
            points.push_back(point);
    }

    memset(image, 255, sizeof(byte_t) * IMAGE_WIDTH * IMAGE_HEIGHT * 3);
    DrawBezierSpline(image, { IMAGE_WIDTH, IMAGE_HEIGHT }, points, STEPS, RGB(0, 0, 0));

    WritePXM("Bezier Spline.ppm", { "P6", IMAGE_WIDTH, IMAGE_HEIGHT, 255 }, image, true, true);
    SAFE_DELETE(image);

    return 0;
}