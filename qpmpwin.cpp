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
  mAction.rm = false;

  startDir = QFileInfo(".").absolutePath();

  pArgs << "--fs";

  connect(this,SIGNAL(mFilesUpdated()),this,SLOT(updateTable()));
  connect(this,SIGNAL(rowChanged(int)),this,SLOT(updatecRow(int)));
  ui->setupUi(this);
  setupTable();
  ui->tableWidget->installEventFilter(this);
}

void qpmpWin::setMoveFile(bool fl){
  ui->actionMove_file->setChecked(fl);
  mAction.mv = fl;
}

void qpmpWin::setnDel(bool fl){
  ui->actionNo_delete->setChecked(fl);
  mAction.ndel = fl;
}

void qpmpWin::setNoSound(bool fl){
  ui->actionNo_Sound->setChecked(fl);
  if(fl){
	if(pArgs.indexOf("--no-audio") == -1){
	  pArgs << "--no-audio";
	}
  }
}

bool qpmpWin::eventFilter(QObject *watched, QEvent *event){

  if(event->type() == QEvent::KeyPress && ui->tableWidget->currentRow() > -1){
	QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

	if(keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return){
	  startPlayer();
	  return true;
	}
	if(ui->tableWidget->currentRow() > 0){
	  if(keyEvent->key() == Qt::Key_Delete || keyEvent->key() == Qt::Key_Backspace){
		deleteFile();
		return true;
	  }
	  if(keyEvent->key() == Qt::Key_M){
		//int row = ui->tableWidget->currentRow();
		moveFile(ui->tableWidget->item(cRow,1)->toolTip());
		emit rowChanged(cRow-1);
		return true;
	  }
	  if(keyEvent->key() == Qt::Key_G){
		//int row = ui->tableWidget->currentRow();
		moveFile(ui->tableWidget->item(cRow,1)->toolTip(),"_gg");
		emit rowChanged(cRow-1);
		return true;
	  }
	  if(keyEvent->key() == Qt::Key_R){
		emit rowChanged(cRow-1);
		startPlayer();
		return true;
	  }
	}
  }
  return QObject::eventFilter(watched,event);
}

void qpmpWin::deleteFile(){
  if(mAction.ndel){
	return;
  }

  QString f;
  QFile fd;
  int row;

  row = ui->tableWidget->currentRow();
  if(row == -1 || row == 0){
	return;
  }
  f = ui->tableWidget->item(row,1)->toolTip();

  fd.setFileName(f);
  if(fd.remove()){
	qDebug() << "Removed file:" << f;
	mFiles.removeAt(mFiles.indexOf(f));
	emit mFilesUpdated();
	emit rowChanged(row-1);
  }else{
	qDebug() << "Failed to remove file:" << f;
  }

  qDebug() << "File to delete:" << f;
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

  row += 1;

  if(row >= tw->rowCount()){
	qDebug() << "No more rows!";
	return;
  }

  mF = tw->item(row,1)->toolTip();

  args = pArgs;
  args << mF;

  proc->start(mP,args);

  QString msg = "mpv args: ";
  msg += pArgs.join(", ");

  emit statusChanged(msg);

  if(!proc->waitForFinished(-1)){
	qDebug() << "Program had an error";
  }

  if(mAction.rm){
	moveFile(mF,".delete");
	row -= 1;
  }
  if(mAction.mv){
	moveFile(mF);
	row -= 1;
  }
  emit rowChanged(row);
}

void qpmpWin::setupTable(){
  QTableWidget *tw = ui->tableWidget;
  QHeaderView *hv = tw->verticalHeader();

  hv->setSectionResizeMode(QHeaderView::Fixed);
  hv->setDefaultSectionSize(18);
  tw->setSelectionBehavior(QTableWidget::SelectRows);
  tw->setRowCount(mFiles.count());
  tw->setSelectionMode(QTableWidget::SingleSelection);
  tw->setRowCount(1);
}

void qpmpWin::updateTable(){
  QTableWidget *tw = ui->tableWidget;
  QHeaderView *hh = tw->horizontalHeader();

  tw->setRowCount(mFiles.count()+1);

  foreach(QString s,mFiles){
	int i = mFiles.indexOf(s)+1;
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
	  emit rowChanged(i);
	}
  }

  if(tw->currentRow() == -1){
	emit rowChanged(0);
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
  QString m;
  m = "Current dir: " + QDir(".").absolutePath();
  m += " : " + msg;
  ui->statusBar->showMessage(m);
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
  emit rowChanged(0);
}

void qpmpWin::on_actionMove_file_triggered()
{
  if(ui->actionMove_file->isChecked()){
	ui->actionDelete->setChecked(false);
	updateStatus("Now moving files after played");
	mAction.mv = true;
	mAction.rm = false;
  }else{
	mAction.mv = false;
  }
}

void qpmpWin::on_actionNo_delete_triggered()
{
  if(ui->actionNo_delete->isChecked()){
	ui->actionDelete->setChecked(false);
	updateStatus("Prohibiting deletion of files");
	mAction.ndel = true;
	mAction.rm = false;
  }else{
	updateStatus("Deletion of files possible");
	mAction.ndel = false;
  }
}

void qpmpWin::saveFileList(QString file){
  QFile f(file);

  if(!f.open(QIODevice::WriteOnly)){
	qDebug() << "Unable to open file!" << file;
	return;
  }
  QTextStream ts(&f);
  foreach(QString l, mFiles){
	ts << l << "\n";
  }
  f.close();
  updateStatus("File list saved to "+file);
}

void qpmpWin::on_actionSaveAs_triggered()
{
  sFile = QFileDialog::getSaveFileName(this,"Save playlist",NULL);
  saveFileList(sFile);
}

void qpmpWin::on_actionSave_triggered()
{
  QString appDir = QApplication::applicationDirPath();
  QString cDir = QDir(".").absolutePath();

  if(appDir != cDir){
	if(sFile.isEmpty()){
	  saveFileList("__savelist.txt");
	}else{
	  saveFileList(sFile);
	}
  }else{
	emit on_actionSaveAs_triggered();
  }
}

void qpmpWin::moveFile(QString s,QString sDir){
  QString nfn;
  QString nfp;
  QFile f(s);
  QFileInfo fi(f);
  QDir d(fi.absoluteDir());

  nfp = d.absolutePath() + "/" + sDir;
  nfn = nfp + "/" + fi.fileName();
  if(d.mkpath(nfp)){
	qDebug() << "Created dir:" << nfp;
	if(f.rename(nfn)){
	  mFiles.removeAt(mFiles.indexOf(s));
	  emit mFilesUpdated();
	}else{
	  qDebug() << "Moving file " << fi.absoluteFilePath() << "to" << nfn << "failed";
	}
  }
}

void qpmpWin::updatecRow(int row){
  cRow = row;
  ui->tableWidget->selectRow(row);
}

void qpmpWin::on_actionDelete_triggered()
{
  if(ui->actionDelete->isChecked()){
	if(ui->actionNo_delete->isChecked()){
	  ui->actionDelete->setChecked(false);
	}else{
	  ui->actionMove_file->setChecked(false);
	  updateStatus("Moving files to ./.delete");
	  mAction.rm = true;
	  mAction.mv = false;
	}
  }else {
	mAction.rm = false;
  }
}
