#include "qpmpwin.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  qpmpWin w;
  w.show();

  return a.exec();
}
