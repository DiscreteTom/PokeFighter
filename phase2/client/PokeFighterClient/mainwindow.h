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
#include <QTableWidget>

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
		POKEMON_TABLE,
		PLAYER_TABLE
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

	// pokemon table and player table
	QTableWidget * table;

	// logon window
	LogonDlg * logonDlg;

	// pokemon dialog
	QTcpSocket * client;

	// user data
	QString username;
	bool myPokemonTable;
	bool showPokemonDlg;

	void changeState(State aim);
	void login();

private slots:
	void getServerMsg();
};

#endif // MAINWINDOW_H
