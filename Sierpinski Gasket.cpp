#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include <Windows.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <random>
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

static const size_t IMAGE_WIDTH  = 500;
static const size_t IMAGE_HEIGHT = 500;
static const int    STEPS        = 100000;

template <typename TYPE>
inline TYPE CreateRandomIntegerValue(TYPE minValue, TYPE maxValue)
{
    std::random_device                  randomDevice;
    std::mt19937                        mt19937RandomEngine(randomDevice());
    std::uniform_int_distribution<TYPE> distribution(minValue, maxValue);

    return distribution(mt19937RandomEngine);
}

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

byte_t* DrawSierpinskiGasket(byte_t* image, SIZE imageSize, POINT point1, POINT point2, POINT point3, int steps, COLORREF color)
{
    std::vector<POINT> points;
    POINT              centerPoint;
    int                index;

    points.push_back(point1);
    points.push_back(point2);
    points.push_back(point3);

    SetPixel(image, imageSize, points[0], color);
    SetPixel(image, imageSize, points[1], color);
    SetPixel(image, imageSize, points[2], color);

    index       = CreateRandomIntegerValue<int>(0, 2);
    centerPoint = points[index];

    for (int step = 0; step < steps; ++step)
    {
        index         = CreateRandomIntegerValue<int>(0, 2);
        centerPoint.x = (LONG)((centerPoint.x + points[index].x) / 2.0 + 0.5);
        centerPoint.y = (LONG)((centerPoint.y + points[index].y) / 2.0 + 0.5);

        SetPixel(image, imageSize, centerPoint, color);
    }

    return image;
}

int main(void)
{
    byte_t* image = new byte_t[IMAGE_WIDTH * IMAGE_HEIGHT * 3];

    memset(image, 255, sizeof(byte_t) * IMAGE_WIDTH * IMAGE_HEIGHT * 3);

    DrawSierpinskiGasket(image, { IMAGE_WIDTH, IMAGE_HEIGHT }, { 250, 100 }, { 100, 400 }, { 400, 400 }, STEPS, RGB(0, 0, 0));

    WritePXM("Sierpinski Gasket.ppm", { "P6", IMAGE_WIDTH, IMAGE_HEIGHT, 255 }, image, true, true);
    SAFE_DELETE(image);

    return 0;
}