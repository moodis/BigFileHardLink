#include "BigFileHardLink.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BigFileHardLink w;
    w.show();
    return a.exec();
}
