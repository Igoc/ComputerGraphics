#include <Windows.h>

#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <tuple>
#include <vector>

#include <glut.h>

#ifndef GLOBAL_VARIABLE
    #define GLOBAL_VARIABLE(variable) (variable)
#endif

#ifndef SAFE_DELETE
    #define SAFE_DELETE(pointer) { if (pointer != nullptr) delete[] pointer; pointer = nullptr; }
#endif

#ifndef CHECK_COORD_VALIDITY
    #define CHECK_COORD_VALIDITY(x, y, width, height) (x >= 0 && y >= 0 && x < width && y < height)
#endif

typedef uint8_t byte_t;

enum class MOUSEBUTTON
{
    LEFT_BUTTON   = 0,
    MIDDLE_BUTTON = 1,
    RIGHT_BUTTON  = 2
};

struct ComplexNumber
{
    double realNumber;
    double imaginaryNumber;
};

static const COORD WINDOW_COORD  = { 0,   0   };
static const SIZE  WINDOW_SIZE   = { 500, 500 };
static const int   MAX_ITERATION = 100;

byte_t*                    GLOBAL_VARIABLE(image);
RECT                       GLOBAL_VARIABLE(zoomArea);
MOUSEBUTTON                GLOBAL_VARIABLE(mouseButton);

std::tuple<double, double> GLOBAL_VARIABLE(mandelbrotViewport);
std::tuple<double, double> GLOBAL_VARIABLE(mandelbrotCenter);

byte_t* DrawMandelbrot(byte_t* image, SIZE imageSize, std::tuple<double, double> center, std::tuple<double, double> viewport)
{
    ComplexNumber    complexNumber;
    ComplexNumber    recurrenceRelation[2];

    int              iteration;
    int              correctedMaxIteration;

    std::vector<int> iterationImage;
    double           iterationMin;
    double           iterationMax;

    correctedMaxIteration  = MAX_ITERATION;
    correctedMaxIteration += 10 * (int)((1.0 - log10(std::get<0>(viewport))) / log10(2.0));

    for (int iy = 0; iy < imageSize.cy; ++iy)
        for (int ix = 0; ix < imageSize.cx; ++ix)
        {
            if (CHECK_COORD_VALIDITY(ix, iy, imageSize.cx, imageSize.cy) == false)
                continue;

            complexNumber.realNumber      = ix * std::get<0>(viewport) / (imageSize.cx - 1) - std::get<0>(viewport) / 2.0 + std::get<0>(center);
            complexNumber.imaginaryNumber = iy * std::get<1>(viewport) / (imageSize.cy - 1) - std::get<1>(viewport) / 2.0 + std::get<1>(center);

            recurrenceRelation[0].realNumber      = 0.0;
            recurrenceRelation[0].imaginaryNumber = 0.0;

            recurrenceRelation[1].realNumber      = 0.0;
            recurrenceRelation[1].imaginaryNumber = 0.0;

            for (iteration = 0; iteration < correctedMaxIteration; ++iteration)
            {
                recurrenceRelation[0].realNumber      = recurrenceRelation[1].realNumber;
                recurrenceRelation[0].imaginaryNumber = recurrenceRelation[1].imaginaryNumber;

                recurrenceRelation[1].realNumber      = recurrenceRelation[0].realNumber * recurrenceRelation[0].realNumber - recurrenceRelation[0].imaginaryNumber * recurrenceRelation[0].imaginaryNumber + complexNumber.realNumber;
                recurrenceRelation[1].imaginaryNumber = 2.0 * recurrenceRelation[0].realNumber * recurrenceRelation[0].imaginaryNumber + complexNumber.imaginaryNumber;

                if (recurrenceRelation[1].realNumber * recurrenceRelation[1].realNumber + recurrenceRelation[1].imaginaryNumber * recurrenceRelation[1].imaginaryNumber > 4.0)
                    break;
            }

            iterationImage.push_back(iteration);
        }

    iterationMin = (double)iterationImage[std::min_element(iterationImage.begin(), iterationImage.end()) - iterationImage.begin()];
    iterationMax = (double)iterationImage[std::max_element(iterationImage.begin(), iterationImage.end()) - iterationImage.begin()];

    for (int index = 0; index < (int)iterationImage.size(); ++index)
        image[index] = (int)((iterationMax - iterationImage[index]) * 255.0 / (iterationMax - iterationMin) + 0.5);

    return image;
}

void InitializeGlobalVariables()
{
    GLOBAL_VARIABLE(image) = new byte_t[WINDOW_SIZE.cx * WINDOW_SIZE.cy];
    memset(GLOBAL_VARIABLE(image), 255, sizeof(byte_t) * WINDOW_SIZE.cx * WINDOW_SIZE.cy);

    GLOBAL_VARIABLE(mandelbrotViewport) = std::make_tuple(2.0F,  2.0F);
    GLOBAL_VARIABLE(mandelbrotCenter)   = std::make_tuple(-0.5F, 0.0F);

    DrawMandelbrot(GLOBAL_VARIABLE(image), WINDOW_SIZE, GLOBAL_VARIABLE(mandelbrotCenter), GLOBAL_VARIABLE(mandelbrotViewport));
}

void InitializeGlut()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
}

void GLUTCALLBACK DisplayCallback()
{
    glViewport(0, 0, WINDOW_SIZE.cx, WINDOW_SIZE.cy);
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_POINTS);
        for (int index = 0; index < WINDOW_SIZE.cx * WINDOW_SIZE.cy; ++index)
        {
            glColor3ub(GLOBAL_VARIABLE(image)[index], GLOBAL_VARIABLE(image)[index], GLOBAL_VARIABLE(image)[index]);
            glVertex3f((index % WINDOW_SIZE.cx) / (float)WINDOW_SIZE.cx, (index / WINDOW_SIZE.cx) / (float)WINDOW_SIZE.cy, 0.0F);
        }
    glEnd();

    glBegin(GL_LINE_LOOP);
        glColor3f(1.0F, 0.0F, 0.0F);
        glVertex3f(GLOBAL_VARIABLE(zoomArea).left  / (float)WINDOW_SIZE.cx, (WINDOW_SIZE.cy - GLOBAL_VARIABLE(zoomArea.top))    / (float)WINDOW_SIZE.cy, 0.0F);
        glVertex3f(GLOBAL_VARIABLE(zoomArea).left  / (float)WINDOW_SIZE.cx, (WINDOW_SIZE.cy - GLOBAL_VARIABLE(zoomArea.bottom)) / (float)WINDOW_SIZE.cy, 0.0F);
        glVertex3f(GLOBAL_VARIABLE(zoomArea).right / (float)WINDOW_SIZE.cx, (WINDOW_SIZE.cy - GLOBAL_VARIABLE(zoomArea.bottom)) / (float)WINDOW_SIZE.cy, 0.0F);
        glVertex3f(GLOBAL_VARIABLE(zoomArea).right / (float)WINDOW_SIZE.cx, (WINDOW_SIZE.cy - GLOBAL_VARIABLE(zoomArea.top))    / (float)WINDOW_SIZE.cy, 0.0F);
    glEnd();

    glutSwapBuffers();
}

void GLUTCALLBACK ReshapeCallback(GLint width, GLint height)
{
    glutReshapeWindow(WINDOW_SIZE.cx, WINDOW_SIZE.cy);
}

void GLUTCALLBACK MouseCallback(GLint button, GLint state, GLint x, GLint y)
{
    double majorAxisLength;

    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        if (state == GLUT_DOWN)
        {
            GLOBAL_VARIABLE(zoomArea).left = x;
            GLOBAL_VARIABLE(zoomArea).top  = y;
        }
        else if (state == GLUT_UP)
        {
            if (abs(GLOBAL_VARIABLE(zoomArea).right - GLOBAL_VARIABLE(zoomArea).left) > abs(GLOBAL_VARIABLE(zoomArea).bottom - GLOBAL_VARIABLE(zoomArea).top))
                majorAxisLength = abs(GLOBAL_VARIABLE(zoomArea).right - GLOBAL_VARIABLE(zoomArea).left);
            else
                majorAxisLength = abs(GLOBAL_VARIABLE(zoomArea).bottom - GLOBAL_VARIABLE(zoomArea).top);

            std::get<0>(GLOBAL_VARIABLE(mandelbrotCenter)) = std::get<0>(GLOBAL_VARIABLE(mandelbrotCenter)) - (WINDOW_SIZE.cx / 2.0F - (GLOBAL_VARIABLE(zoomArea).left + GLOBAL_VARIABLE(zoomArea).right)  / 2.0F) * std::get<0>(GLOBAL_VARIABLE(mandelbrotViewport)) / WINDOW_SIZE.cx;
            std::get<1>(GLOBAL_VARIABLE(mandelbrotCenter)) = std::get<1>(GLOBAL_VARIABLE(mandelbrotCenter)) + (WINDOW_SIZE.cy / 2.0F - (GLOBAL_VARIABLE(zoomArea).top  + GLOBAL_VARIABLE(zoomArea).bottom) / 2.0F) * std::get<1>(GLOBAL_VARIABLE(mandelbrotViewport)) / WINDOW_SIZE.cy;

            std::get<0>(GLOBAL_VARIABLE(mandelbrotViewport)) = majorAxisLength * std::get<0>(GLOBAL_VARIABLE(mandelbrotViewport)) / WINDOW_SIZE.cx;
            std::get<1>(GLOBAL_VARIABLE(mandelbrotViewport)) = majorAxisLength * std::get<1>(GLOBAL_VARIABLE(mandelbrotViewport)) / WINDOW_SIZE.cy;

            DrawMandelbrot(GLOBAL_VARIABLE(image), WINDOW_SIZE, GLOBAL_VARIABLE(mandelbrotCenter), GLOBAL_VARIABLE(mandelbrotViewport));
            memset(&GLOBAL_VARIABLE(zoomArea), 0, sizeof(RECT));
            glutPostRedisplay();
        }

        GLOBAL_VARIABLE(mouseButton) = MOUSEBUTTON::LEFT_BUTTON;
        break;

    case GLUT_MIDDLE_BUTTON:
        GLOBAL_VARIABLE(mouseButton) = MOUSEBUTTON::MIDDLE_BUTTON;
        break;

    case GLUT_RIGHT_BUTTON:
        GLOBAL_VARIABLE(mouseButton) = MOUSEBUTTON::RIGHT_BUTTON;
        break;
    }
}

void GLUTCALLBACK MotionCallback(GLint x, GLint y)
{
    if (GLOBAL_VARIABLE(mouseButton) == MOUSEBUTTON::LEFT_BUTTON)
    {
        GLOBAL_VARIABLE(zoomArea).right  = x;
        GLOBAL_VARIABLE(zoomArea).bottom = y;
    }

    glutPostRedisplay();
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowPosition(WINDOW_COORD.X, WINDOW_COORD.Y);
    glutInitWindowSize(WINDOW_SIZE.cx, WINDOW_SIZE.cy);
    glutCreateWindow("Mandelbrot");

    InitializeGlobalVariables();
    InitializeGlut();

    glutDisplayFunc(DisplayCallback);
    glutReshapeFunc(ReshapeCallback);
    glutMouseFunc(MouseCallback);
    glutMotionFunc(MotionCallback);

    glutMainLoop();
    SAFE_DELETE(GLOBAL_VARIABLE(image));

    return 0;
}