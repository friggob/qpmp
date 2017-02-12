#ifndef QPMPWIN_H
#define QPMPWIN_H

#include <QMainWindow>

namespace Ui {
  class qpmpWin;
}

class qpmpWin : public QMainWindow
{
	Q_OBJECT

  public:
	explicit qpmpWin(QWidget *parent = 0);
	~qpmpWin();

  private:
	Ui::qpmpWin *ui;
};

#endif // QPMPWIN_H
