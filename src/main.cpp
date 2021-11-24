#include <QApplication>
#include <QDebug>

#include "annimgmanager.h"
#include "mainwindow.h"

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  MainWindow w;
  w.showMaximized();

  return a.exec();
}
