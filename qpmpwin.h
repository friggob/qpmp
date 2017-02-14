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
	QString mP; //movie player
	const QVersionNumber ver = QVersionNumber::fromString(APP_VERSION);
	~qpmpWin();

  private slots:
	void on_actionQuit_triggered();
	void on_actionOpen_triggered();
	void on_actionSaveAs_triggered();
	void processFileList(QStringList);
	void updateTable();
	void on_actionRefresh_triggered();
	void on_actionAbout_triggered();

	void on_actionNo_Sound_triggered();

  private:
	Ui::qpmpWin *ui;
	QStringList mFiles;
	QStringList fList;
	QStringList pArgs; // Movie player arguments
	void resizeEvent(QResizeEvent *event);
	void setupTable();
	void startPlayer();

  signals:
	void mFilesUpdated();

  protected:
	bool eventFilter(QObject *watched, QEvent *event);
};

#endif // QPMPWIN_H
