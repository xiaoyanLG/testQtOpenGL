/*
 * Demo 展示在同一个Qt窗口上同时进行opengl绘制和painter绘制的可行性
 *
*/
#include <QApplication>
#include "xytestwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    XYTestWindow w;
    w.resize(500, 500);
    w.show();

    return a.exec();
}
