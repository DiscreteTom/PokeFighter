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

	enum State // use bitmask
	{
		START = 1,
		LOGIN = 2,
		MAIN = 4,
		POKEMON_TABLE = 8,
		PLAYER_TABLE = 16,
		CHANGE_PSW = 32,
		LV_UP_BATTLE = 64,
		DUEL_BATTLE = 128,
		CHOOSE_ENEMY = 256
	};

	int state;

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

	// duel battle statistic layout
	QLabel * lbWin;
	QLabel * lbTotal;
	QLabel * lbWinRate;

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
	QString battlePokemonID;
	bool gettingDuelStatistic;

	void changeState(int aim);
	void login();
private slots:
	void getServerMsg();
};

#endif // MAINWINDOW_H
