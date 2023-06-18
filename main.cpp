#include "HouseWidget.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    //MyMainWindow w;
    HouseWidget w;
    w.show();
    return a.exec();
}
