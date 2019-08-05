#include "mainwindow.h"
#include <QApplication>
#include "annimgmanager.h"
#include "annotationfilter.h"
#include <QDebug>


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    qRegisterMetaType<AnnImgManager>("AnnImgManager");
    qRegisterMetaType<AnnotationFilter>("AnnotationFilter");
    MainWindow w;
    w.showMaximized();

    return a.exec();
}
