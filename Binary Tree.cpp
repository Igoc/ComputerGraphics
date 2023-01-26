#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include <Windows.h>

#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>

#ifndef SAFE_DELETE
    #define SAFE_DELETE(pointer) { if (pointer != nullptr) delete[] pointer; pointer = nullptr; }
#endif

#ifndef CHECK_COORD_VALIDITY
    #define CHECK_COORD_VALIDITY(x, y, width, height) (x >= 0 && y >= 0 && x < width && y < height)
#endif

#ifndef EXECUTION_CONDITION
    #define EXECUTION_CONDITION(expression, status) { if ((expression) == false) return (status); }
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
static const float  DECREASE_RATE = 0.6F;
static const int    THETA         = 45;
static const int    STEPS         = 10;

template <typename TYPE>
inline TYPE CreateRandomIntegerValue(TYPE minValue, TYPE maxValue)
{
    std::random_device                  randomDevice;
    std::mt19937                        mt19937RandomEngine(randomDevice());
    std::uniform_int_distribution<TYPE> distribution(minValue, maxValue);

    return distribution(mt19937RandomEngine);
}

template <typename TYPE>
inline TYPE CreateRandomRealValue(TYPE minValue, TYPE maxValue)
{
    std::random_device                   randomDevice;
    std::mt19937                         mt19937RandomEngine(randomDevice());
    std::uniform_real_distribution<TYPE> distribution(minValue, maxValue);

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

byte_t* DrawNormalTree(byte_t* image, SIZE imageSize, POINT startPoint, POINT endPoint, float decreaseRate, int theta, int steps, COLORREF color)
{
    EXECUTION_CONDITION(steps > 0, image);

    float radian;
    POINT rotationPoint;
    POINT decreasePoint;

    DrawDDALine(image, imageSize, startPoint, endPoint, color);

    decreasePoint.x = (LONG)(startPoint.x + (endPoint.x - startPoint.x) * (1.0 - decreaseRate));
    decreasePoint.y = (LONG)(startPoint.y + (endPoint.y - startPoint.y) * (1.0 - decreaseRate));

    radian          = (180 + theta) * 3.141592F / 180.0F;
    rotationPoint.x = (LONG)(decreasePoint.x * cos(radian) - decreasePoint.y * sin(radian) - endPoint.x * cos(radian) + endPoint.y * sin(radian) + endPoint.x + 0.5);
    rotationPoint.y = (LONG)(decreasePoint.x * sin(radian) + decreasePoint.y * cos(radian) - endPoint.x * sin(radian) - endPoint.y * cos(radian) + endPoint.y + 0.5);
    DrawNormalTree(image, imageSize, endPoint, rotationPoint, decreaseRate, theta, steps - 1, color);

    radian          = (180 - theta) * 3.141592F / 180.0F;
    rotationPoint.x = (LONG)(decreasePoint.x * cos(radian) - decreasePoint.y * sin(radian) - endPoint.x * cos(radian) + endPoint.y * sin(radian) + endPoint.x + 0.5);
    rotationPoint.y = (LONG)(decreasePoint.x * sin(radian) + decreasePoint.y * cos(radian) - endPoint.x * sin(radian) - endPoint.y * cos(radian) + endPoint.y + 0.5);
    DrawNormalTree(image, imageSize, endPoint, rotationPoint, decreaseRate, theta, steps - 1, color);

    return image;
}

byte_t* DrawRandomTree(byte_t* image, SIZE imageSize, POINT startPoint, POINT endPoint, int steps, COLORREF color)
{
    EXECUTION_CONDITION(steps > 0, image);

    float radian;
    POINT rotationPoint;
    POINT decreasePoint;

    DrawDDALine(image, imageSize, startPoint, endPoint, color);

    decreasePoint.x = (LONG)(startPoint.x + (endPoint.x - startPoint.x) * (1.0 - CreateRandomRealValue<float>(0.45, 0.85)));
    decreasePoint.y = (LONG)(startPoint.y + (endPoint.y - startPoint.y) * (1.0 - CreateRandomRealValue<float>(0.45, 0.85)));

    radian          = (180 + CreateRandomIntegerValue<int>(-10, 60)) * 3.141592F / 180.0F;
    rotationPoint.x = (LONG)(decreasePoint.x * cos(radian) - decreasePoint.y * sin(radian) - endPoint.x * cos(radian) + endPoint.y * sin(radian) + endPoint.x + 0.5);
    rotationPoint.y = (LONG)(decreasePoint.x * sin(radian) + decreasePoint.y * cos(radian) - endPoint.x * sin(radian) - endPoint.y * cos(radian) + endPoint.y + 0.5);
    DrawRandomTree(image, imageSize, endPoint, rotationPoint, steps - 1, color);

    radian          = (180 - CreateRandomIntegerValue<int>(-10, 60)) * 3.141592F / 180.0F;
    rotationPoint.x = (LONG)(decreasePoint.x * cos(radian) - decreasePoint.y * sin(radian) - endPoint.x * cos(radian) + endPoint.y * sin(radian) + endPoint.x + 0.5);
    rotationPoint.y = (LONG)(decreasePoint.x * sin(radian) + decreasePoint.y * cos(radian) - endPoint.x * sin(radian) - endPoint.y * cos(radian) + endPoint.y + 0.5);
    DrawRandomTree(image, imageSize, endPoint, rotationPoint, steps - 1, color);

    return image;
}

int main(void)
{
    byte_t* normalTreeImage = new byte_t[IMAGE_WIDTH * IMAGE_HEIGHT * 3];
    byte_t* randomTreeImage = new byte_t[IMAGE_WIDTH * IMAGE_HEIGHT * 3];

    memset(normalTreeImage, 255, sizeof(byte_t) * IMAGE_WIDTH * IMAGE_HEIGHT * 3);
    memset(randomTreeImage, 255, sizeof(byte_t) * IMAGE_WIDTH * IMAGE_HEIGHT * 3);

    DrawNormalTree(normalTreeImage, { IMAGE_WIDTH, IMAGE_HEIGHT }, { 250, 400 }, { 250, 250 }, DECREASE_RATE, THETA, STEPS, RGB(0, 0, 0));
    DrawRandomTree(randomTreeImage, { IMAGE_WIDTH, IMAGE_HEIGHT }, { 250, 400 }, { 250, 250 }, STEPS, RGB(0, 0, 0));

    WritePXM("Normal Binary Tree.ppm", { "P6", IMAGE_WIDTH, IMAGE_HEIGHT, 255 }, normalTreeImage, true, true);
    SAFE_DELETE(normalTreeImage);

    WritePXM("Random Binary Tree.ppm", { "P6", IMAGE_WIDTH, IMAGE_HEIGHT, 255 }, randomTreeImage, true, true);
    SAFE_DELETE(randomTreeImage);

    return 0;
}