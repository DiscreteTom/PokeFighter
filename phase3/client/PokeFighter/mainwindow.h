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
#include <QSpinBox>
#include <QProgressBar>
#include "pokemondlg.h"

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
		CHOOSE_ENEMY = 256,
		CHOOSE_BET = 512
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
	QPushButton *btnLvUpBattle;
	QPushButton *btnDuelBattle;
	QPushButton *btnDisplayAllPlayer;
	QPushButton *btnChangePassword;
	QPushButton *btnLogout;

	// change password layout
	QLineEdit *leNewPassword;
	QPushButton *btnOK;

	// duel battle statistic layout
	QLabel *lbWin;
	QLabel *lbTotal;
	QLabel *lbWinRate;

	// choose enemy layout
	QLabel *lbChooseEnemy;
	QLabel *lbEnemyLV;
	QSpinBox *sbEnemyLV;
	QPushButton *btnEnemyRace0;
	QPushButton *btnEnemyRace1;
	QPushButton *btnEnemyRace2;
	QPushButton *btnEnemyRace3;

	// battle layout
	QLabel *lbP1; // img
	QLabel *lbP2; // img
	QPushButton *btnSkill_1;
	QPushButton *btnSkill_2;
	QPushButton *btnSkill_3;
	QPushButton *btnSkill_4;
	QProgressBar *pbP1HP;
	QProgressBar *pbP2HP;
	QLabel *lbP1SkillName;
	QLabel *lbP2SkillName;

	// choose bet layout
	QLabel *lbBet[3];
	QPushButton *btnBet[3];
	PokemonDlg *pkmDlg[3];

	// pokemon table and player table
	QTableWidget *table;
	QLabel *lbPokemonNumBadge;
	QLabel *lbPokemonMasterBadge;

	// logon window
	LogonDlg *logonDlg;

	// pokemon dialog
	QTcpSocket *client;

	// media
	QMediaPlayer *mediaPlayer;

	// user data
	QString username;
	int currentPlayerID; // if 0 means current player is the player himself
	bool showPokemonDlg;
	bool changingPokemonName;
	QString battlePokemonID;
	bool gettingDuelStatistic;
	bool battleStart;
	int currentPokemonLV;
	int chooseBetIndex;

	void changeState(int aim);
	void login();
	void getImproved(QLabel *lb);	// simple animation
	void getDecreased(QLabel *lb); // simple animation
	void mySleep(int n);
private slots:
	void getServerMsg();
};

#endif // MAINWINDOW_H
