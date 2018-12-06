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
#include <QMediaPlayer>

namespace Ui
{
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

	enum State
	{
		START,
		LOGIN,
		MAIN,
		POKEMON_TABLE,
		PLAYER_TABLE,
		CHANGE_PSW,
		LV_UP_BATTLE,
		DUEL_BATTLE
	};

	State state;

	QGridLayout *layout;

	// start layout
	QLabel *lbStartTitle;
	QPushButton *btnPlay;
	QPushButton *btnExit;

	// login layout
	QLabel *lbLoginLabel;
	QLineEdit *leUsername;
	QLineEdit *lePassword;
	QPushButton *btnLogin;
	QPushButton *btnLogon;
	QPushButton *btnBack;

	// main layout
	QPushButton *btnShowPokemonList;
	QPushButton * btnLvUpBattle;
	QPushButton * btnDuelBattle;
	QPushButton *btnDisplayAllPlayer;
	QPushButton * btnChangePassword;
	QPushButton *btnLogout;

	// change password layout
	QLineEdit * leNewPassword;
	QPushButton * btnOK;


	// pokemon table and player table
	QTableWidget *table;

	// logon window
	LogonDlg *logonDlg;

	// pokemon dialog
	QTcpSocket *client;

	// media
	QMediaPlayer * mediaPlayer;

	// user data
	QString username;
	int currentPlayerID; // if 0 means current player is the player himself
	bool showPokemonDlg;
	bool changingPokemonName;

	void changeState(State aim);
	void login();
private slots:
	void getServerMsg();
};

#endif // MAINWINDOW_H
