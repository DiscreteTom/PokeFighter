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
#include <QListWidget>
#include <QList>

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
		MAIN,
		POKEMON_LIST,
		PLAYER_LIST
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
	QPushButton * btnShowPokemonList;
	QPushButton * btnDisplayAllPlayer;

	// pokemon list and player list
	QListWidget * list;
	QList<int> * playerID_List;

	// logon window
	LogonDlg * logonDlg;

	QTcpSocket * client;

	// user data
	QString username;

	void changeState(State aim);
	void login();

private slots:
	void getServerMsg();
};

#endif // MAINWINDOW_H
