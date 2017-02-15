#include "qpmpwin.h"
#include "ui_qpmpwin.h"
#include <QFileDialog>
#include <QDebug>
#include <QMimeDatabase>
#include <QMessageBox>
#include <QProcess>
#include <QKeyEvent>

qpmpWin::~qpmpWin()
{
  delete ui;
}

qpmpWin::qpmpWin(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::qpmpWin)
{
#ifndef Q_OS_WIN
  mP = "mpv";
#else
  mP = "mpv.exe";
#endif

  mAction.ndel = false;
  mAction.mv = false;

  pArgs << "--fs";

  connect(this,SIGNAL(mFilesUpdated()),this,SLOT(updateTable()));
  ui->setupUi(this);
  setupTable();
  ui->tableWidget->installEventFilter(this);
}

bool qpmpWin::eventFilter(QObject *watched, QEvent *event){

  if(event->type() == QEvent::KeyPress){
	QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

	if(keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return){
	  startPlayer();
	  return true;
	}
  }
  return QObject::eventFilter(watched,event);
}

void qpmpWin::startPlayer(){
  QString mF;
  int row;
  QTableWidget *tw = ui->tableWidget;
  QProcess *proc = new QProcess();
  QStringList args;

  if((row = tw->currentRow()) == -1){
	qDebug() << "No row selected!";
	return;
  }

  mF = tw->item(row,1)->toolTip();

  args = pArgs;
  args << mF;

  proc->start(mP,args);

  QString msg = "mpv args: ";
  msg += pArgs.join(", ");

  ui->statusBar->showMessage(msg);

  if(!proc->waitForFinished(-1)){
	qDebug() << "Program had an error";
  }

  tw->selectRow(row+1);
}

void qpmpWin::setupTable(){
  QTableWidget *tw = ui->tableWidget;
  QHeaderView *hv = tw->verticalHeader();

  hv->setSectionResizeMode(QHeaderView::Fixed);
  hv->setDefaultSectionSize(18);
  tw->setSelectionBehavior(QTableWidget::SelectRows);
  tw->setRowCount(mFiles.count());
  tw->setSelectionMode(QTableWidget::SingleSelection);
}

void qpmpWin::updateTable(){
  QTableWidget *tw = ui->tableWidget;
  QHeaderView *hh = tw->horizontalHeader();

  tw->setRowCount(mFiles.count());

  foreach(QString s,mFiles){
	int i = mFiles.indexOf(s);
	QFileInfo fi(s);

	QTableWidgetItem *nCell = tw->item(i,1);
	QTableWidgetItem *sCell = tw->item(i,0);

	if(!nCell){
	  nCell = new QTableWidgetItem();
	  tw->setItem(i,1,nCell);
	}
	nCell->setText(fi.fileName());
	nCell->setToolTip(fi.absoluteFilePath());

	if(!sCell){
	  sCell = new QTableWidgetItem();
	  tw->setItem(i,0,sCell);
	}
	sCell->setText(sizeFormat(fi.size()));

	if(fi.absoluteFilePath() == startFile){
	  tw->selectRow(i);
	}
  }

  if(tw->currentRow() == -1){
	tw->selectRow(0);
  }

  tw->resizeColumnsToContents();

  hh->setMaximumHeight(18);
  hh->setStretchLastSection(true);
  tw->adjustSize();

  //qDebug() << "Current row:" << tw->currentRow();
  QString msg;
  msg = QString::asprintf("%d movie files",tw->rowCount());
  emit updateStatus(msg);
}

void qpmpWin::resizeEvent(QResizeEvent *event){

  QMainWindow::resizeEvent(event);
  emit updateTable();
}

void qpmpWin::on_actionQuit_triggered()
{
  close();
  QApplication::quit();
}

void qpmpWin::on_actionOpen_triggered()
{
  QString filter;
  QStringList fl;

  filter = tr("All files (*)");

  fl = QFileDialog::getOpenFileNames(this,"Open movie files",NULL,filter);

  if(fl.count() > 0){
	mFiles.clear();
	processFileList(fl);
  }
}

void qpmpWin::on_actionSaveAs_triggered()
{
  QString sFile;
  sFile = QFileDialog::getSaveFileName(this,"Save playlist",NULL);
  qDebug()<< sFile;
}

void qpmpWin::setFileList(QStringList list){
  //list.removeFirst();
  processFileList(list);
}

void qpmpWin::processFileList(QStringList list){
  QFileInfo fi;
  QFile fd;
  QMimeDatabase md;
  QMimeType mt;
  QString l;

  foreach(QString f, list){
	fi.setFile(f);

	if(fi.exists()){
	  mt = md.mimeTypeForFile(fi.filePath());

	  if(mt.name() == "text/plain"){
		qDebug() << f << "is a text file! Assuming playlist.";
		fd.setFileName(f);
		fd.open(QIODevice::ReadOnly);

		QTextStream ts(&fd);
		QStringList sl;

		while(!ts.atEnd()){
		  l = ts.readLine();
		  QFileInfo fi2(l);

		  if(l.startsWith("*")){
			l = l.remove(0,1);
			fi2 = QFileInfo(l);
			startFile = fi2.absoluteFilePath();
			qDebug() << "Starting file:" << startFile;
		  }

		  if(!list.contains(l) &&
			 !list.contains(fi2.absoluteFilePath()) &&
			 !list.contains(fi2.filePath())){
			sl.append(l);
		  }else{
			qDebug() << "Found duplicate path!" << l;
		  }
		}

		fd.close();
		QDir::setCurrent(fi.absolutePath());
		processFileList(sl);

	  }
	  if(mt.name().startsWith("video/") && !mFiles.contains(fi.absoluteFilePath())){
		mFiles.append(fi.absoluteFilePath());
	  }
	}
  }
  //qDebug() << "mFiles:" << mFiles;
  emit mFilesUpdated();
}

void qpmpWin::on_actionRefresh_triggered()
{
  emit mFilesUpdated();
}

QString qpmpWin::sizeFormat(quint64 size){
  qreal calc = size;
  QStringList list;
  list << "KB" << "MB" << "GB" << "TB" <<  "PB" << "EB";

  QStringListIterator i(list);
  QString unit("byte(s)");

  while(calc >= 1024.0 && i.hasNext()){
	unit = i.next();
	calc /= 1024.0;
  }

  return QString().setNum(calc, 'f', 2) + " " + unit;
}

void qpmpWin::on_actionAbout_triggered()
{
  QString msg;
  msg = "qpmp, a pmp inplementation in QT\n\n";
  msg += "Version: "+ver.toString()+"\n\n";
  msg += "Created by:\n";
  msg += "Fredrik Olausson <fredrik@bluppfisk.org>\n\n";

  QMessageBox::about(this,"About qpmp",msg);
}

void qpmpWin::toggleArgs(QAction *a,QString s){
  if(!a->isChecked()){
	int i;
	i = pArgs.indexOf(s);
	pArgs.removeAt(i);
  }else{
	pArgs << s;
  }
}

void qpmpWin::on_actionNo_Sound_triggered()
{
  toggleArgs(ui->actionNo_Sound,"--no-audio");
}

void qpmpWin::updateStatus(QString msg){
  ui->statusBar->showMessage(msg);
}

void qpmpWin::on_actionFullscreen_triggered()
{
  toggleArgs(ui->actionFullscreen,"--fs");
}

void qpmpWin::on_actionForce_4_3_triggered()
{
  if(ui->actionForce_16_9->isChecked()){
	ui->actionForce_16_9->setChecked(false);
	toggleArgs(ui->actionForce_16_9,"--video-aspect=16:9");
  }
  toggleArgs(ui->actionForce_4_3,"--video-aspect=4:3");
}

void qpmpWin::on_actionForce_16_9_triggered()
{
  if(ui->actionForce_4_3->isChecked()){
	ui->actionForce_4_3->setChecked(false);
	toggleArgs(ui->actionForce_4_3,"--video-aspect=4:3");
  }
  toggleArgs(ui->actionForce_16_9,"--video-aspect=16:9");
}

void qpmpWin::on_actionShuffle_triggered()
{
  std::random_shuffle(mFiles.begin(),mFiles.end());
  updateTable();
  ui->tableWidget->selectRow(0);
}

void qpmpWin::on_actionMove_file_triggered()
{
  if(ui->actionMove_file->isChecked()){
	updateStatus("Now moving files after played");
	mAction.mv = true;
  }else{
	mAction.mv = false;
  }
}

void qpmpWin::on_actionNo_delete_triggered()
{
  if(ui->actionNo_delete->isChecked()){
	updateStatus("Prohibiting deletion of files");
	mAction.ndel = true;
  }else{
	updateStatus("Deletion of files possible");
	mAction.ndel = false;
  }
}
