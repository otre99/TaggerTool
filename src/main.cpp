#include <QApplication>
#include <QDebug>

#include "mainwindow.h"
#include "utils.h"

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  a.setApplicationName(Helper::appName);
  a.setOrganizationName(Helper::organizationName);
  a.setOrganizationDomain(Helper::organizationDomain);

  MainWindow w;
  w.showMaximized();

  return a.exec();
}
