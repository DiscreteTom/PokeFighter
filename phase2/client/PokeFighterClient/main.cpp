#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	// set style sheet
	QFile file(":/qss/qss/main.qss");
	file.open(QFile::ReadOnly);
	QTextStream filetext(&file);
	QString stylesheet = filetext.readAll();
	qApp->setStyleSheet(stylesheet);
	file.close();

	MainWindow w;
	w.show();

	return a.exec();
}
