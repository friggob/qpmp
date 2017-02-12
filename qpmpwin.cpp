#include "qpmpwin.h"
#include "ui_qpmpwin.h"

qpmpWin::qpmpWin(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::qpmpWin)
{
  ui->setupUi(this);
}

qpmpWin::~qpmpWin()
{
  delete ui;
}
