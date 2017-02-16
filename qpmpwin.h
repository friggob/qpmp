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
	void setnDel(bool);
	void setNoSound(bool);
	void setMoveFile(bool);
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
	void updateStatus(QString);
	void on_actionFullscreen_triggered();
	void on_actionForce_4_3_triggered();
	void on_actionForce_16_9_triggered();
	void on_actionShuffle_triggered();
	void on_actionMove_file_triggered();
	void on_actionNo_delete_triggered();
	void on_actionSave_triggered();
	void updatecRow(int);
	void on_actionDelete_triggered();

  private:
	Ui::qpmpWin *ui;
	int cRow;
	QStringList mFiles;
	QStringList fList;
	QStringList pArgs; // Movie player arguments
	QString startFile; // Start playlist with this file
	QString startDir; // Current dir when program starts
	QString sFile; // Savelist file name
	void resizeEvent(QResizeEvent *event);
	void setupTable();
	void startPlayer();
	void toggleArgs(QAction*,QString);
	void deleteFile();
	void saveFileList(QString);
	void moveFile(QString, QString sDir="sett");
	struct mAction{
		bool ndel;
		bool mv;
		bool rm;
	} mAction;

  signals:
	void mFilesUpdated();
	void statusChanged(QString);
	void rowChanged(int);

  protected:
	bool eventFilter(QObject *watched, QEvent *event);
};

#endif // QPMPWIN_H
