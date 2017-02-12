#ifndef QPMPWIN_H
#define QPMPWIN_H

#include <QMainWindow>
#include <QVersionNumber>

namespace Ui {
  class qpmpWin;
}

class qpmpWin : public QMainWindow
{
	Q_OBJECT

  public:
	explicit qpmpWin(QWidget *parent = 0);
	void setFileList(QStringList);
	QString sizeFormat(quint64 size);
	const QVersionNumber ver = QVersionNumber(0,2,0);
	~qpmpWin();

  private slots:
	void on_actionQuit_triggered();
	void on_actionOpen_triggered();
	void on_actionSaveAs_triggered();
	void processFileList(QStringList);
	void updateTable();

	void on_actionRefresh_triggered();

	void on_actionAbout_triggered();

  private:
	Ui::qpmpWin *ui;
	QStringList mFiles;
	QStringList fList;
	void resizeEvent(QResizeEvent *event);

	void setupTable();

  signals:
	void mFilesUpdated();
};

#endif // QPMPWIN_H
