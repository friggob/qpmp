#include "qpmpwin.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>

int main(int argc, char *argv[])
{
  QCommandLineParser *cp = new QCommandLineParser();


  QCommandLineOption D("D", "Set noDelete flag");
  QCommandLineOption n("n", "Set no sound flag");
  QCommandLineOption m("m", "Move files to ./sett when watched.");

  QList<QCommandLineOption> ol;
  ol << D << n << m;


  cp->setApplicationDescription("Movie playlist application");
  cp->addOptions(ol);
  cp->addHelpOption();
  cp->addVersionOption();
  cp->addPositionalArgument("[files]","Movie files, or playlist files, to play");

  QApplication a(argc, argv);
  a.setApplicationName("qpmp");
  a.setApplicationVersion(APP_VERSION);

  cp->process(a);

  qpmpWin w;

  w.setnDel(cp->isSet("D"));
  w.setNoSound(cp->isSet("n"));
  w.setMoveFile(cp->isSet("m"));

  w.setFileList(cp->positionalArguments());

  w.show();

  return a.exec();
}
