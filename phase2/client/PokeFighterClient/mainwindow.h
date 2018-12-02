#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QFrame>
#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QTcpSocket>
#include "logondlg.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	Ui::MainWindow *ui;

	enum State{
		START,
		LOGIN,
		MAIN
	};

	State state;

	QGridLayout * layout;

	// start layout
	QLabel * lbStartTitle;
	QPushButton * btnPlay;
	QPushButton * btnExit;

	// login layout
	QLabel * lbLoginLabel;
	QLineEdit * leUsername;
	QLineEdit * lePassword;
	QPushButton * btnLogin;
	QPushButton * btnLogon;
	QPushButton * btnBack;

	// main layout
	QPushButton * btnLogout;
	QPushButton * btnDisplayAllPlayer;

	// logon window
	LogonDlg * logonDlg;

	QTcpSocket * client;

	void changeState(State aim);
	void login();

private slots:
	void getServerMsg();
};

#endif // MAINWINDOW_H
