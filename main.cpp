#include "widget.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Widget w;

    w.setWindowTitle("图像处理平台");

    w.show();
    return a.exec();
}
