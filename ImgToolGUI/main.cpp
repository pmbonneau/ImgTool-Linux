#include "imgtoolgui.h"

#include <QtGui>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ImgToolGUI w;
    w.show();
    return a.exec();
}
