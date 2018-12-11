#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QHostAddress>
#include "netconfig.h"
#include "pokemondlg.h"
#include "authentication.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
																					ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	setWindowTitle(tr("宠物小精灵对战程序客户端"));

	// start layout
	lbStartTitle = new QLabel(tr("宠物小精灵对战程序"), this);
	lbStartTitle->setObjectName("lbStartTitle");
	btnPlay = new QPushButton(tr("开始游戏"), this);
	btnPlay->setObjectName("btnPlay");
	btnExit = new QPushButton(tr("退出"), this);
	btnExit->setObjectName("btnExit");

	// login layout
	lbLoginLabel = new QLabel(tr("登录"), this);
	lbLoginLabel->setObjectName("lbLoginLabel");
	leUsername = new QLineEdit(this);
	leUsername->setObjectName("leUsername");
	leUsername->setPlaceholderText(tr("请输入用户名"));
	lePassword = new QLineEdit(this);
	lePassword->setObjectName("lePassword");
	lePassword->setPlaceholderText(tr("请输入密码"));
	lePassword->setEchoMode(QLineEdit::Password);
	btnLogin = new QPushButton(tr("登录"), this);
	btnLogin->setObjectName("btnLogin");
	btnLogon = new QPushButton(tr("注册"), this);
	btnLogon->setObjectName("btnLogon");
	btnBack = new QPushButton(tr("返回"), this);
	btnBack->setObjectName("btnBack");

	// main layout
	btnShowPokemonList = new QPushButton(tr("查看精灵"), this);
	btnShowPokemonList->setObjectName("btnShowPokemonList");
	btnLvUpBattle = new QPushButton(tr("升级比赛"), this);
	btnLvUpBattle->setObjectName("btnLvUpBattle");
	btnDuelBattle = new QPushButton(tr("决斗比赛"), this);
	btnDuelBattle->setObjectName("btnDuelBattle");
	btnDisplayAllPlayer = new QPushButton(tr("查看所有玩家"), this);
	btnDisplayAllPlayer->setObjectName("btnDisplayAllPlayer");
	btnChangePassword = new QPushButton(tr("修改密码"), this);
	btnChangePassword->setObjectName("btnChangePassword");
	btnLogout = new QPushButton(tr("退出登录"), this);
	btnLogout->setObjectName("btnLogout");

	// change password layout
	leNewPassword = new QLineEdit(this);
	leNewPassword->setObjectName("leNewPassword");
	btnOK = new QPushButton(tr("提交"), this);
	btnOK->setObjectName("btnOK");
	leNewPassword->setPlaceholderText(tr("请输入新密码"));
	leNewPassword->setEchoMode(QLineEdit::Password);

	// duel battle statistic layout
	lbWin = new QLabel(this);
	lbWin->setObjectName("lbWin");
	lbTotal = new QLabel(this);
	lbTotal->setObjectName("lbTotal");
	lbWinRate = new QLabel(this);
	lbWinRate->setObjectName("lbWinRate");

	// choose enemy layout
	lbChooseEnemy = new QLabel(tr("选择你的对手："), this);
	lbEnemyLV = new QLabel(tr("对手等级："), this);
	sbEnemyLV = new QSpinBox(this);
	btnEnemyRace0 = new QPushButton(tr("小火龙"), this);
	btnEnemyRace0->setIcon(QPixmap(":/img/img/charmander.png"));
	btnEnemyRace1 = new QPushButton(tr("妙蛙种子"), this);
	btnEnemyRace1->setIcon(QPixmap(":/img/img/bulbasaur.png"));
	btnEnemyRace2 = new QPushButton(tr("杰尼龟"), this);
	btnEnemyRace2->setIcon(QPixmap(":/img/img/squirtle.png"));
	btnEnemyRace3 = new QPushButton(tr("波波"), this);
	btnEnemyRace3->setIcon(QPixmap(":/img/img/pidgey.png"));
	sbEnemyLV->setMaximum(15);
	sbEnemyLV->setMinimum(1);

	// battle layout
	lbP1 = new QLabel(tr("精灵1"), this);
	lbP2 = new QLabel(tr("精灵2"), this);
	btnSkill_1 = new QPushButton(this);
	btnSkill_2 = new QPushButton(this);
	btnSkill_3 = new QPushButton(this);
	btnSkill_4 = new QPushButton(this);
	pbP1HP = new QProgressBar(this);
	pbP2HP = new QProgressBar(this);
	//	pbP1AtkInterval = new QProgressBar(this);
	//	pbP2AtkInterval = new QProgressBar(this);
	lbP1SkillName = new QLabel(this);
	lbP2SkillName = new QLabel(this);

	// pokemon table and player table
	table = new QTableWidget(this);

	// logon window
	logonDlg = new LogonDlg(this);

	// media
	mediaPlayer = new QMediaPlayer(this);

	connect(btnPlay, &QPushButton::clicked, this, [this] { changeState(LOGIN); });
	connect(btnExit, &QPushButton::clicked, this, [this] { qApp->quit(); });
	connect(btnLogin, &QPushButton::clicked, this, &MainWindow::login);
	connect(btnLogon, &QPushButton::clicked, this, [this] {
		if (logonDlg->exec() == QDialog::Accepted)
		{
			leUsername->setText(logonDlg->getUsername());
			lePassword->setText(logonDlg->getPassword());
		}
	});
	connect(btnBack, &QPushButton::clicked, this, [this] {
		switch (state)
		{
		case LOGIN:
			changeState(START);
			client->disconnectFromHost();
			break;
		case POKEMON_TABLE:
			if (currentPlayerID == 0)
			{
				changeState(MAIN);
			}
			else
			{
				changeState(PLAYER_TABLE);
				client->write("getPlayerList", BUF_LENGTH);
			}
			break;
		case PLAYER_TABLE:
		case CHANGE_PSW:
		case POKEMON_TABLE | LV_UP_BATTLE:
		case POKEMON_TABLE | DUEL_BATTLE:
			changeState(MAIN);
			break;
		case CHOOSE_ENEMY | LV_UP_BATTLE:
		case CHOOSE_ENEMY | DUEL_BATTLE:
			changeState((state ^ CHOOSE_ENEMY) | POKEMON_TABLE);
			client->write("getPokemonList", BUF_LENGTH);
			break;
		case LV_UP_BATTLE:
		case DUEL_BATTLE:
			changeState(state | CHOOSE_ENEMY);
			break;
		default:
			break;
		}
	});
	connect(btnLogout, &QPushButton::clicked, this, [this] {
		changeState(LOGIN);
		client->write("logout", BUF_LENGTH);
		client->disconnectFromHost();
	});
	connect(btnShowPokemonList, &QPushButton::clicked, this, [this] {
		changeState(POKEMON_TABLE);
		currentPlayerID = 0;
		client->write("getPokemonList", BUF_LENGTH);
	});
	connect(btnLvUpBattle, &QPushButton::clicked, this, [this] {
		currentPlayerID = 0;
		client->write("getPokemonList", BUF_LENGTH);
		changeState(POKEMON_TABLE | LV_UP_BATTLE);
	});
	connect(btnDuelBattle, &QPushButton::clicked, this, [this] {
		currentPlayerID = 0;
		client->write("getPokemonList", BUF_LENGTH);
		changeState(POKEMON_TABLE | DUEL_BATTLE);
	});
	connect(btnDisplayAllPlayer, &QPushButton::clicked, this, [this] {
		changeState(PLAYER_TABLE);
		client->write("getPlayerList", BUF_LENGTH);
	});
	connect(btnChangePassword, &QPushButton::clicked, this, [this] { changeState(CHANGE_PSW); });
	connect(btnOK, &QPushButton::clicked, this, [this] {
		if (!isValidPassword(lePassword->text()))
		{
			QMessageBox::warning(this, tr("错误"), tr("旧密码格式错误"));
			lePassword->clear();
		}
		else if (!isValidPassword(leNewPassword->text()))
		{
			QMessageBox::warning(this, tr("错误"), tr("新密码格式错误"));
			leNewPassword->clear();
		}
		else
		{
			QString str = "resetPassword ";
			str += lePassword->text();
			str += ' ';
			str += leNewPassword->text();
			client->write(str.toStdString().c_str(), BUF_LENGTH);
		}
	});
	connect(btnEnemyRace0, &QPushButton::clicked, [this] {
		battleStart = true;
		QString msg = "battle ";
		msg += battlePokemonID + ' ';
		msg += "0 ";
		msg += QString::number(sbEnemyLV->value());
		client->write(msg.toLocal8Bit(), BUF_LENGTH);
		changeState(state ^ CHOOSE_ENEMY);
	});
	connect(btnEnemyRace1, &QPushButton::clicked, [this] {
		battleStart = true;
		QString msg = "battle ";
		msg += battlePokemonID + ' ';
		msg += "1 ";
		msg += QString::number(sbEnemyLV->value());
		client->write(msg.toLocal8Bit(), BUF_LENGTH);
		changeState(state ^ CHOOSE_ENEMY);
	});
	connect(btnEnemyRace2, &QPushButton::clicked, [this] {
		battleStart = true;
		QString msg = "battle ";
		msg += battlePokemonID + ' ';
		msg += "2 ";
		msg += QString::number(sbEnemyLV->value());
		client->write(msg.toLocal8Bit(), BUF_LENGTH);
		changeState(state ^ CHOOSE_ENEMY);
	});
	connect(btnEnemyRace3, &QPushButton::clicked, [this] {
		battleStart = true;
		QString msg = "battle ";
		msg += battlePokemonID + ' ';
		msg += "3 ";
		msg += QString::number(sbEnemyLV->value());
		client->write(msg.toLocal8Bit(), BUF_LENGTH);
		changeState(state ^ CHOOSE_ENEMY);
	});
	connect(btnSkill_1, &QPushButton::clicked, [this] {
		client->write("0", BUF_LENGTH);
		btnSkill_1->setDisabled(true);
		btnSkill_2->setDisabled(true);
		btnSkill_3->setDisabled(true);
		btnSkill_4->setDisabled(true);
	});
	connect(btnSkill_2, &QPushButton::clicked, [this] {
		client->write("1", BUF_LENGTH);
		btnSkill_1->setDisabled(true);
		btnSkill_2->setDisabled(true);
		btnSkill_3->setDisabled(true);
		btnSkill_4->setDisabled(true);
	});
	connect(btnSkill_3, &QPushButton::clicked, [this] {
		client->write("2", BUF_LENGTH);
		btnSkill_1->setDisabled(true);
		btnSkill_2->setDisabled(true);
		btnSkill_3->setDisabled(true);
		btnSkill_4->setDisabled(true);
	});
	connect(btnSkill_4, &QPushButton::clicked, [this] {
		client->write("3", BUF_LENGTH);
		btnSkill_1->setDisabled(true);
		btnSkill_2->setDisabled(true);
		btnSkill_3->setDisabled(true);
		btnSkill_4->setDisabled(true);
	});

	connect(leUsername, &QLineEdit::returnPressed, btnLogin, &QPushButton::click);
	connect(lePassword, &QLineEdit::returnPressed, this, [this] {
		if (state == LOGIN)
		{
			btnLogin->click();
		}
		else if (state == CHANGE_PSW)
		{
			btnOK->click();
		}
	});
	connect(leNewPassword, &QLineEdit::returnPressed, btnOK, &QPushButton::click);

	client = new QTcpSocket(this);
	connect(client, &QTcpSocket::readyRead, this, &MainWindow::getServerMsg);

	changeState(START);

	changingPokemonName = false;
	gettingDuelStatistic = false;
	battleStart = false;

	//	setFixedSize(1600, 900);
	setFixedSize(1366, 966); // size of start.jpg

	mediaPlayer->setMedia(QUrl("qrc:/music/media/op.mp3"));
	mediaPlayer->setVolume(50);
	mediaPlayer->play();
}

MainWindow::~MainWindow()
{
	delete client;
	delete ui;
}

void MainWindow::changeState(int aim)
{
	setStyleSheet(tr("background-image:url(\":/img/img/start.jpg\");"));

	// hide all widget
	lbStartTitle->hide();
	btnPlay->hide();
	btnExit->hide();
	lbLoginLabel->hide();
	leUsername->hide();
	lePassword->hide();
	btnLogin->hide();
	btnLogon->hide();
	btnBack->hide();
	btnLogout->hide();
	btnShowPokemonList->hide();
	btnLvUpBattle->hide();
	btnDuelBattle->hide();
	btnChangePassword->hide();
	btnDisplayAllPlayer->hide();
	btnOK->hide();
	leNewPassword->hide();
	lbWin->hide();
	lbTotal->hide();
	lbWinRate->hide();
	lbChooseEnemy->hide();
	lbEnemyLV->hide();
	sbEnemyLV->hide();
	btnEnemyRace0->hide();
	btnEnemyRace1->hide();
	btnEnemyRace2->hide();
	btnEnemyRace3->hide();
	lbP1->hide();
	lbP2->hide();
	btnSkill_1->hide();
	btnSkill_2->hide();
	btnSkill_3->hide();
	btnSkill_4->hide();
	pbP1HP->hide();
	pbP2HP->hide();
	//	pbP1AtkInterval->hide();
	//	pbP2AtkInterval->hide();
	lbP1SkillName->hide();
	lbP2SkillName->hide();
	table->hide();
	table->clear();
	btnPlay->setDefault(false);
	btnBack->setDefault(false);
	btnLogin->setDefault(false);
	btnShowPokemonList->setDefault(false);
	btnOK->setDefault(false);

	disconnect(table, &QTableWidget::cellChanged, 0, 0);

	state = aim;

	// delete old layout
	delete ui->centralWidget->layout();
	layout = new QGridLayout(this);

	// show widgets in the certain state
	switch (state)
	{
	case START:
		lbStartTitle->show();
		btnPlay->show();
		btnExit->show();
		setTabOrder(btnPlay, btnExit);
		setTabOrder(btnExit, btnPlay);
		layout->addWidget(lbStartTitle, 0, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);
		layout->addWidget(btnPlay, 1, 0, 1, 1, Qt::AlignRight);
		layout->addWidget(btnExit, 2, 0, 1, 1, Qt::AlignRight);
		btnPlay->setDefault(true);
		break;
	case LOGIN:
		lbLoginLabel->show();
		leUsername->show();
		lePassword->clear();
		lePassword->show();
		btnLogin->show();
		btnLogon->show();
		btnBack->show();
		setTabOrder(leUsername, lePassword);
		setTabOrder(lePassword, btnLogin);
		setTabOrder(btnLogin, btnLogon);
		setTabOrder(btnLogon, btnBack);
		setTabOrder(btnBack, leUsername);
		layout->addWidget(lbLoginLabel, 0, 0, Qt::AlignHCenter | Qt::AlignTop);
		layout->addWidget(leUsername, 1, 0, Qt::AlignRight);
		layout->addWidget(lePassword, 2, 0, Qt::AlignRight);
		layout->addWidget(btnLogin, 3, 0, Qt::AlignRight);
		layout->addWidget(btnLogon, 4, 0, Qt::AlignRight);
		layout->addWidget(btnBack, 5, 0, Qt::AlignRight);
		btnLogin->setDefault(true);
		leUsername->setFocus();
		break;
	case MAIN:
		btnShowPokemonList->show();
		btnLvUpBattle->show();
		btnDuelBattle->show();
		btnDisplayAllPlayer->show();
		btnChangePassword->show();
		btnLogout->show();
		setTabOrder(btnShowPokemonList, btnLvUpBattle);
		setTabOrder(btnLvUpBattle, btnDuelBattle);
		setTabOrder(btnDuelBattle, btnDisplayAllPlayer);
		setTabOrder(btnDisplayAllPlayer, btnChangePassword);
		setTabOrder(btnChangePassword, btnLogout);
		setTabOrder(btnLogout, btnShowPokemonList);
		layout->addWidget(btnShowPokemonList, 0, 0, Qt::AlignRight);
		layout->addWidget(btnLvUpBattle, 1, 0, Qt::AlignRight);
		layout->addWidget(btnDuelBattle, 2, 0, Qt::AlignRight);
		layout->addWidget(btnDisplayAllPlayer, 3, 0, Qt::AlignRight);
		layout->addWidget(btnChangePassword, 4, 0, Qt::AlignRight);
		layout->addWidget(btnLogout, 5, 0, Qt::AlignRight);
		btnShowPokemonList->setDefault(true);
		break;
	case POKEMON_TABLE:
	case PLAYER_TABLE:
	case POKEMON_TABLE | LV_UP_BATTLE:
	case POKEMON_TABLE | DUEL_BATTLE:
		btnBack->show();
		table->show();
		if (state == (POKEMON_TABLE | DUEL_BATTLE))
		{
			lbWin->show();
			lbTotal->show();
			lbWinRate->show();
			layout->addWidget(lbWin, 0, 0, 1, 1, Qt::AlignHCenter);
			layout->addWidget(lbTotal, 0, 1, 1, 1, Qt::AlignHCenter);
			layout->addWidget(lbWinRate, 0, 2, 1, 1, Qt::AlignHCenter);
			layout->addWidget(table, 1, 0, 1, 3);
			layout->addWidget(btnBack, 2, 0, 1, 3);
		}
		else
		{
			layout->addWidget(table, 0, 0);
			layout->addWidget(btnBack, 1, 0);
		}
		btnBack->setDefault(true);
		break;
	case CHANGE_PSW:
		lePassword->clear();
		lePassword->show();
		leNewPassword->clear();
		leNewPassword->show();
		btnOK->show();
		btnBack->show();
		setTabOrder(lePassword, leNewPassword);
		setTabOrder(leNewPassword, btnOK);
		setTabOrder(btnOK, btnBack);
		setTabOrder(btnBack, lePassword);
		layout->addWidget(lePassword, 0, 0, 1, 1, Qt::AlignCenter);
		layout->addWidget(leNewPassword, 1, 0, 1, 1, Qt::AlignCenter);
		layout->addWidget(btnOK, 2, 0, 1, 1, Qt::AlignCenter);
		layout->addWidget(btnBack, 3, 0, 1, 1, Qt::AlignCenter);
		btnOK->setDefault(true);
		break;
	case CHOOSE_ENEMY | DUEL_BATTLE:
	case CHOOSE_ENEMY | LV_UP_BATTLE:
		lbChooseEnemy->show();
		lbEnemyLV->show();
		sbEnemyLV->show();
		btnEnemyRace0->show();
		btnEnemyRace1->show();
		btnEnemyRace2->show();
		btnEnemyRace3->show();
		btnBack->show();
		if (state == (CHOOSE_ENEMY | DUEL_BATTLE))
		{
			sbEnemyLV->setValue(15);
			sbEnemyLV->setDisabled(true);
		}
		else
		{
			sbEnemyLV->setDisabled(false);
		}
		layout->addWidget(lbChooseEnemy, 0, 0, 1, 2);
		layout->addWidget(lbEnemyLV, 0, 2, 1, 1);
		layout->addWidget(sbEnemyLV, 0, 3, 1, 1);
		layout->addWidget(btnEnemyRace0, 1, 0, 1, 2);
		layout->addWidget(btnEnemyRace1, 1, 2, 1, 2);
		layout->addWidget(btnEnemyRace2, 2, 0, 1, 2);
		layout->addWidget(btnEnemyRace3, 2, 2, 1, 2);
		layout->addWidget(btnBack, 3, 0, 1, 4);
		break;
	case LV_UP_BATTLE:
	case DUEL_BATTLE:
		setStyleSheet("");
		lbP1->show();
		lbP2->show();
		btnSkill_1->show();
		btnSkill_2->show();
		btnSkill_3->show();
		btnSkill_4->show();
		pbP1HP->show();
		//		pbP1AtkInterval->show();
		pbP2HP->show();
		//		pbP2AtkInterval->show();
		layout->addWidget(pbP1HP, 0, 0, 1, 2);
		//		layout->addWidget(pbP1AtkInterval, 1, 0, 1, 2);
		layout->addWidget(lbP1SkillName, 1, 0, 1, 2);
		layout->addWidget(pbP2HP, 0, 2, 1, 2);
		//		layout->addWidget(pbP2AtkInterval, 1, 2, 1, 2);
		layout->addWidget(lbP2SkillName, 1, 2, 1, 2);
		layout->addWidget(lbP1, 2, 0, 1, 2);
		layout->addWidget(lbP2, 2, 2, 1, 2);
		layout->addWidget(btnSkill_1, 3, 0, 1, 1);
		layout->addWidget(btnSkill_2, 3, 1, 1, 1);
		layout->addWidget(btnSkill_3, 3, 2, 1, 1);
		layout->addWidget(btnSkill_4, 3, 3, 1, 1);
		break;
	default:
		break;
	}
	ui->centralWidget->setLayout(layout);

	// special settings, must behind ui->centralWidget->setLayout()
	if (state == LOGIN)
	{
		leUsername->setFocus();
	}
	else if (state == MAIN)
	{
		btnShowPokemonList->setFocus();
	}
}

void MainWindow::login()
{
	if (!isValidUsername(leUsername->text()))
	{
		QMessageBox::warning(this, tr("错误"), tr("用户名不合法"));
		leUsername->clear();
		lePassword->clear();
		return;
	}
	if (!isValidPassword(lePassword->text()))
	{
		QMessageBox::warning(this, tr("错误"), tr("密码不合法"));
		lePassword->clear();
		return;
	}
	client->connectToHost(QHostAddress("127.0.0.1"), 7500);
	QString msg = "login";
	msg += ' ';
	msg += leUsername->text();
	msg += ' ';
	msg += lePassword->text();

	btnLogin->setDisabled(true);

	if (client->write(msg.toLocal8Bit(), BUF_LENGTH) == -1)
	{
		// error
		QMessageBox::warning(this, tr("错误"), tr("服务器错误"));
		btnLogin->setDisabled(false);
	}
	lePassword->clear();
}

void MainWindow::getImproved(QLabel *lb)
{
	lb->move(lb->x(), lb->y() + 50);
	QThread::msleep(50);
	lb->move(lb->x(), lb->y() + 50);
	QThread::msleep(50);
	lb->move(lb->x(), lb->y() + 50);
	QThread::msleep(50);
	lb->move(lb->x(), lb->y() + 50);
	QThread::msleep(50);
	lb->move(lb->x(), lb->y() + 50);
	QThread::msleep(50);
	lb->move(lb->x(), lb->y() - 50);
	QThread::msleep(50);
	lb->move(lb->x(), lb->y() - 50);
	QThread::msleep(50);
	lb->move(lb->x(), lb->y() - 50);
	QThread::msleep(50);
	lb->move(lb->x(), lb->y() - 50);
	QThread::msleep(50);
	lb->move(lb->x(), lb->y() - 50);
	QThread::msleep(50);
}

void MainWindow::getDecreased(QLabel *lb)
{
	// P1 lose HP
	lb->hide();
	QThread::msleep(200);
	lb->show();
	QThread::msleep(200);
	lb->hide();
	QThread::msleep(200);
	lb->show();
	QThread::msleep(200);
}

void MainWindow::getServerMsg()
{
	auto ret = client->read(BUF_LENGTH);

	if (state == LOGIN)
		client->disconnectFromHost();

	QString msg = QString::fromLocal8Bit(ret);

	if (changingPokemonName)
	{
		if (msg != "Accept.\n")
		{
			QMessageBox::warning(this, tr("错误"), tr("修改精灵名字失败"));
		}
		else
		{
			QMessageBox::information(this, tr("精灵修改名字"), tr("精灵名字已更新"));
		}

		changingPokemonName = false;
		return;
	}

	switch (state)
	{
	case LOGIN:
	{
		btnLogin->setDisabled(false);
		int port = msg.toInt();
		if (port == 0)
		{
			// login failed
			QMessageBox::warning(this, tr("错误"), msg);
		}
		else
		{
			// success
			username = leUsername->text();
			client->connectToHost(QHostAddress("127.0.0.1"), port);
			changeState(MAIN);
		}
		break;
	}
	case PLAYER_TABLE:
	{
		/**
		 * msg format:
		 * <playerID> <playername> <online:0|1> <win> <total>
		 */
		auto players = msg.split('\n');

		//! players[players.size() - 1] == ""

		table->setRowCount(players.size() - 2);
		table->setColumnCount(7); // id - username - online - win - total - win rate - viewPokemon
		table->setHorizontalHeaderLabels({tr("玩家ID"), tr("用户名"), tr("在线情况"), tr("决斗次数"), tr("决斗获胜次数"), tr("胜率"), tr("操作")});
		table->verticalHeader()->hide();
		table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
		table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

		int tableRowIndex = 0;
		for (int i = 0; i < players.size() - 1; ++i)
		{
			auto player = players[i];
			auto detail = player.split(' ');
			// detail[0] is id, detail[1] is username
			if (detail[1] == username)
				continue; // not show player himself
			auto t = new QTableWidgetItem(detail[0]);
			t->setFlags(t->flags() ^ Qt::ItemIsEnabled);
			if (detail[2] == '0')
				t->setBackgroundColor(QColor("#eff0f1"));
			table->setItem(tableRowIndex, 0, t);
			t = new QTableWidgetItem(detail[1]);
			t->setFlags(t->flags() ^ Qt::ItemIsEnabled);
			if (detail[2] == '0')
				t->setBackgroundColor(QColor("#eff0f1"));
			table->setItem(tableRowIndex, 1, t);
			t = new QTableWidgetItem(detail[2] == '1' ? tr("在线") : tr("离线"));
			t->setFlags(t->flags() ^ Qt::ItemIsEnabled);
			if (detail[2] == '0')
				t->setBackgroundColor(QColor("#eff0f1"));
			table->setItem(tableRowIndex, 2, t);
			t = new QTableWidgetItem(detail[3]);
			t->setFlags(t->flags() ^ Qt::ItemIsEnabled);
			if (detail[2] == '0')
				t->setBackgroundColor(QColor("#eff0f1"));
			table->setItem(tableRowIndex, 3, t);
			t = new QTableWidgetItem(detail[4]);
			t->setFlags(t->flags() ^ Qt::ItemIsEnabled);
			if (detail[2] == '0')
				t->setBackgroundColor(QColor("#eff0f1"));
			table->setItem(tableRowIndex, 4, t);
			t = new QTableWidgetItem(detail[4] == "0" ? "-" : QString::number(detail[3].toDouble() / detail[4].toDouble()));
			t->setFlags(t->flags() ^ Qt::ItemIsEnabled);
			if (detail[2] == '0')
				t->setBackgroundColor(QColor("#eff0f1"));
			table->setItem(tableRowIndex, 5, t);
			auto btn = new QPushButton(tr("查看小精灵"), this);
			connect(btn, &QPushButton::clicked, this, [this, detail] {
				changeState(POKEMON_TABLE);
				currentPlayerID = detail[0].toInt();
				QString str = "getPokemonList ";
				str += detail[0];
				client->write(str.toStdString().c_str(), BUF_LENGTH);
			});
			table->setCellWidget(tableRowIndex, 6, btn);
			++tableRowIndex;
		}
		break;
	}
	case POKEMON_TABLE:
	case POKEMON_TABLE | LV_UP_BATTLE:
	case POKEMON_TABLE | DUEL_BATTLE:
	{
		if (gettingDuelStatistic)
		{
			gettingDuelStatistic = false;
			auto detail = msg.split(' ');
			lbWin->setText(tr("获胜次数：") + detail[0]);
			lbTotal->setText(tr("决斗次数：") + detail[1]);
			lbWinRate->setText(tr("胜率：") + (detail[1] == '0' ? "-" : QString::number(detail[0].toDouble() / detail[1].toDouble())));
			break;
		}
		if (!showPokemonDlg) // msg is pokemon table
		{
			auto pokemons = msg.split('\n');

			table->setRowCount(pokemons.size() - 1);
			if (state == POKEMON_TABLE)
			{
				table->setColumnCount(5);
				table->setHorizontalHeaderLabels({tr("精灵ID"), tr("名字"), tr("种族"), tr("等级"), tr("操作")});
			}
			else
			{
				// battle table
				table->setColumnCount(6);
				table->setHorizontalHeaderLabels({tr("精灵ID"), tr("名字"), tr("种族"), tr("等级"), tr("查看"), tr("选中")});
			}
			table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
			table->verticalHeader()->hide();
			for (int i = 0; i < pokemons.size() - 1; ++i)
			{
				auto detail = pokemons[i].split(' ');
				// detail[0] is name, detail[1] is race, detail[2] is lv
				auto t = new QTableWidgetItem(detail[0]);
				t->setFlags(t->flags() ^ Qt::ItemIsEditable);
				table->setItem(i, 0, t);
				t = new QTableWidgetItem(detail[1]);
				t->setToolTip(tr("双击以更改精灵名称"));
				table->setItem(i, 1, t);
				t = new QTableWidgetItem(detail[2]);
				t->setFlags(t->flags() ^ Qt::ItemIsEditable);
				table->setItem(i, 2, t);
				t = new QTableWidgetItem(detail[3]);
				t->setFlags(t->flags() ^ Qt::ItemIsEditable);
				table->setItem(i, 3, t);
				auto btn = new QPushButton(tr("查看详情"), this);
				connect(btn, &QPushButton::clicked, this, [this, detail] {
					QString str = "getPokemon ";
					str += detail[0];
					client->write(str.toStdString().c_str(), BUF_LENGTH);
					showPokemonDlg = true;
				});
				table->setCellWidget(i, 4, btn);
				if (state != POKEMON_TABLE)
				{
					// battle table
					auto btn = new QPushButton(tr("就决定是你了！"), this);
					connect(btn, &QPushButton::clicked, this, [this, detail] {
						battlePokemonID = detail[0];
						changeState((state ^ POKEMON_TABLE) | CHOOSE_ENEMY);
					});
					table->setCellWidget(i, 5, btn);
				}
			}
			if (state == (DUEL_BATTLE | POKEMON_TABLE))
			{
				if (!gettingDuelStatistic)
				{
					gettingDuelStatistic = true;
					client->write("getDuelStatistic", BUF_LENGTH);
				}
			}

			connect(table, &QTableWidget::cellChanged, this, [this](int row, int column) {
				if ((state & POKEMON_TABLE) == POKEMON_TABLE && column == 1)
				{
					// pokemon name changed
					QString str = "pokemonChangeName ";
					str += table->item(row, 0)->text() + ' ' + table->item(row, 1)->text();
					client->write(str.toLocal8Bit(), BUF_LENGTH);
					changingPokemonName = true;
				}
			}); // must be connected after data input
		}
		else // msg is pokemon detail
		{
			auto dlg = new PokemonDlg(msg, currentPlayerID == 0, this);
			connect(dlg, &PokemonDlg::pokemonChangeName, this, [this](const QString &pokemonID, const QString &newName) {
				for (int i = 0; i < table->rowCount(); ++i)
				{
					if (table->item(i, 0)->text() == pokemonID)
					{
						table->item(i, 1)->setText(newName); // will call table::cellChanged
					}
				}
			});
			showPokemonDlg = false;
		}
		break;
	}
	case CHANGE_PSW:
		if (msg == "Accept.\n")
		{
			QMessageBox::information(this, tr("密码重置"), tr("密码重置成功"));
			changeState(MAIN);
		}
		else
		{
			QMessageBox::warning(this, tr("错误"), msg);
		}
		break;
	case DUEL_BATTLE:
	case LV_UP_BATTLE:
	{
		if (battleStart)
		{
			btnSkill_1->setDisabled(true);
			btnSkill_2->setDisabled(true);
			btnSkill_3->setDisabled(true);
			btnSkill_4->setDisabled(true);
			// get battle config message
			auto ps = msg.split('\n');
			auto detail = ps[0].split(' ');
			if (detail[0] == "妙蛙种子")
			{
				lbP1->setPixmap(QPixmap(":/img/img/bulbasaur.png"));
			}
			else if (detail[0] == "小火龙")
			{
				lbP1->setPixmap(QPixmap(":/img/img/charmander.png"));
			}
			else if (detail[0] == "杰尼龟")
			{
				lbP1->setPixmap(QPixmap(":/img/img/squirtle.png"));
			}
			else if (detail[0] == "波波")
			{
				lbP1->setPixmap(QPixmap(":/img/img/pidgey.png"));
			}
			pbP1HP->setMaximum(detail[1].toInt());
			pbP1HP->setValue(detail[1].toInt());
			btnSkill_1->setText(detail[2]);
			btnSkill_2->setText(detail[4]);
			btnSkill_3->setText(detail[6]);
			btnSkill_4->setText(detail[8]);
			btnSkill_1->setToolTip(detail[3]);
			btnSkill_2->setToolTip(detail[5]);
			btnSkill_3->setToolTip(detail[7]);
			btnSkill_4->setToolTip(detail[9]);
			btnSkill_2->setText(btnSkill_2->text() + ' ' + detail[10]);
			btnSkill_3->setText(btnSkill_3->text() + ' ' + detail[11]);
			btnSkill_4->setText(btnSkill_4->text() + ' ' + detail[12]);
			detail = ps[1].split(' ');
			if (detail[0] == "妙蛙种子")
			{
				lbP2->setPixmap(QPixmap(":/img/img/bulbasaur.png"));
			}
			else if (detail[0] == "小火龙")
			{
				lbP2->setPixmap(QPixmap(":/img/img/charmander.png"));
			}
			else if (detail[0] == "杰尼龟")
			{
				lbP2->setPixmap(QPixmap(":/img/img/squirtle.png"));
			}
			else if (detail[0] == "波波")
			{
				lbP2->setPixmap(QPixmap(":/img/img/pidgey.png"));
			}
			pbP2HP->setMaximum(detail[1].toInt());
			pbP2HP->setValue(detail[1].toInt());
			battleStart = false;
			client->write("done", BUF_LENGTH);
			break;
		}
		if (msg == "turn")
		{
			btnSkill_1->setDisabled(false);
			btnSkill_2->setDisabled(false);
			btnSkill_3->setDisabled(false);
			btnSkill_4->setDisabled(false);
			break;
		}
		auto detail = msg.split(' ');
		if (detail[0] == '1')
		{
			lbP1SkillName->setText(detail[1]);
			QThread::msleep(500);
			if (detail[2] == '1')
			{
				// dodge!
				lbP2SkillName->setText(tr("闪避！"));
				QThread::msleep(500);
				lbP2SkillName->clear();
				break;
			}
			// not dodge
			if (pbP2HP->value() > detail[3].toInt())
			{
				lbP2SkillName->setText(QString::number(detail[3].toInt() - pbP2HP->value()));
				getDecreased(lbP2);
				lbP2SkillName->clear();
			}
			else if (pbP2HP->value() < detail[3].toInt())
			{
				// hp increased
				lbP2SkillName->setText(QString::number(detail[3].toInt() - pbP2HP->value()));
				getImproved(lbP2);
				lbP2SkillName->clear();
			}
			pbP2HP->setValue(detail[3].toInt());
			if (detail[4] == '0')
			{
				lbP2SkillName->setText(tr("攻击下降"));
				getDecreased(lbP2);
				lbP2SkillName->clear();
			}
			else if (detail[4] == '2')
			{
				lbP2SkillName->setText(tr("攻击上升"));
				getImproved(lbP2);
				lbP2SkillName->clear();
			}
			if (detail[5] == '0')
			{
				lbP2SkillName->setText(tr("防御下降"));
				getDecreased(lbP2);
				lbP2SkillName->clear();
			}
			else if (detail[5] == '2')
			{
				lbP2SkillName->setText(tr("防御上升"));
				getImproved(lbP2);
				lbP2SkillName->clear();
			}
			if (detail[6] == '0')
			{
				lbP2SkillName->setText(tr("速度下降"));
				getDecreased(lbP2);
				lbP2SkillName->clear();
			}
			else if (detail[6] == '2')
			{
				lbP2SkillName->setText(tr("速度上升"));
				getImproved(lbP2);
				lbP2SkillName->clear();
			}
			if (pbP1HP->value() > detail[10].toInt())
			{
				// hp decreased
				lbP1SkillName->setText(QString::number(detail[10].toInt() - pbP1HP->value()));
				getDecreased(lbP1);
				lbP1SkillName->clear();
			}
			else if (pbP1HP->value() < detail[10].toInt())
			{
				// hp increased
				lbP1SkillName->setText(QString::number(detail[10].toInt() - pbP1HP->value()));
				getImproved(lbP1);
				lbP1SkillName->clear();
			}
			pbP1HP->setValue(detail[10].toInt());
			if (detail[11] == '0')
			{
				lbP1SkillName->setText(tr("攻击下降"));
				getDecreased(lbP1);
				lbP1SkillName->clear();
			}
			else if (detail[11] == '2')
			{
				lbP1SkillName->setText(tr("攻击上升"));
				getImproved(lbP1);
				lbP1SkillName->clear();
			}
			if (detail[12] == '0')
			{
				lbP1SkillName->setText(tr("防御下降"));
				getDecreased(lbP1);
				lbP1SkillName->clear();
			}
			else if (detail[12] == '2')
			{
				lbP1SkillName->setText(tr("防御上升"));
				getImproved(lbP1);
				lbP1SkillName->clear();
			}
			if (detail[13] == '0')
			{
				lbP1SkillName->setText(tr("速度下降"));
				getDecreased(lbP1);
				lbP1SkillName->clear();
			}
			else if (detail[13] == '2')
			{
				lbP1SkillName->setText(tr("速度上升"));
				getImproved(lbP1);
				lbP1SkillName->clear();
			}
			auto skillName = btnSkill_2->text().split(' ')[0];
			btnSkill_2->setText(skillName + ' ' + detail[14]);
			skillName = btnSkill_3->text().split(' ')[0];
			btnSkill_3->setText(skillName + ' ' + detail[15]);
			skillName = btnSkill_4->text().split(' ')[0];
			btnSkill_4->setText(skillName + ' ' + detail[16]);
			lbP1SkillName->clear();
		}
		else
		{
			// detail[0] == '0', p2's turn
			lbP2SkillName->setText(detail[1]);
			QThread::msleep(500);
			if (detail[2] == '1')
			{
				// dodge!
				lbP1SkillName->setText(tr("闪避！"));
				QThread::msleep(500);
				lbP1SkillName->clear();
				break;
			}
			// not dodge
			if (pbP1HP->value() > detail[3].toInt())
			{
				lbP1SkillName->setText(QString::number(detail[3].toInt() - pbP1HP->value()));
				getDecreased(lbP1);
				lbP1SkillName->clear();
			}
			else if (pbP1HP->value() < detail[3].toInt())
			{
				// hp increased
				lbP1SkillName->setText(QString::number(detail[3].toInt() - pbP1HP->value()));
				getImproved(lbP1);
				lbP1SkillName->clear();
			}
			pbP1HP->setValue(detail[3].toInt());
			if (detail[4] == '0')
			{
				lbP1SkillName->setText(tr("攻击下降"));
				getDecreased(lbP1);
				lbP1SkillName->clear();
			}
			else if (detail[4] == '2')
			{
				lbP1SkillName->setText(tr("攻击上升"));
				getImproved(lbP1);
				lbP1SkillName->clear();
			}
			if (detail[5] == '0')
			{
				lbP1SkillName->setText(tr("防御下降"));
				getDecreased(lbP1);
				lbP1SkillName->clear();
			}
			else if (detail[5] == '2')
			{
				lbP1SkillName->setText(tr("防御上升"));
				getImproved(lbP1);
				lbP1SkillName->clear();
			}
			if (detail[6] == '0')
			{
				lbP1SkillName->setText(tr("速度下降"));
				getDecreased(lbP1);
				lbP1SkillName->clear();
			}
			else if (detail[6] == '2')
			{
				lbP1SkillName->setText(tr("速度上升"));
				getImproved(lbP1);
				lbP1SkillName->clear();
			}
			if (pbP2HP->value() > detail[10].toInt())
			{
				// hp decreased
				lbP2SkillName->setText(QString::number(detail[10].toInt() - pbP2HP->value()));
				getDecreased(lbP2);
				lbP2SkillName->clear();
			}
			else if (pbP2HP->value() < detail[10].toInt())
			{
				// hp increased
				lbP2SkillName->setText(QString::number(detail[10].toInt() - pbP2HP->value()));
				getImproved(lbP2);
				lbP2SkillName->clear();
			}
			pbP2HP->setValue(detail[10].toInt());
			if (detail[11] == '0')
			{
				lbP2SkillName->setText(tr("攻击下降"));
				getDecreased(lbP2);
				lbP2SkillName->clear();
			}
			else if (detail[11] == '2')
			{
				lbP2SkillName->setText(tr("攻击上升"));
				getImproved(lbP2);
				lbP2SkillName->clear();
			}
			if (detail[12] == '0')
			{
				lbP2SkillName->setText(tr("防御下降"));
				getDecreased(lbP2);
				lbP2SkillName->clear();
			}
			else if (detail[12] == '2')
			{
				lbP2SkillName->setText(tr("防御上升"));
				getImproved(lbP2);
				lbP2SkillName->clear();
			}
			if (detail[13] == '0')
			{
				lbP2SkillName->setText(tr("速度下降"));
				getDecreased(lbP2);
				lbP2SkillName->clear();
			}
			else if (detail[13] == '2')
			{
				lbP2SkillName->setText(tr("速度上升"));
				getImproved(lbP2);
				lbP2SkillName->clear();
			}
			auto skillName = btnSkill_2->text().split(' ')[0];
			btnSkill_2->setText(skillName + ' ' + detail[7]);
			skillName = btnSkill_3->text().split(' ')[0];
			btnSkill_3->setText(skillName + ' ' + detail[8]);
			skillName = btnSkill_4->text().split(' ')[0];
			btnSkill_4->setText(skillName + ' ' + detail[9]);
			lbP2SkillName->clear();
		}

		// judge result
		if (pbP2HP->value() == 0)
		{
			QMessageBox::information(this, tr("恭喜"), tr("你赢得了战斗"));
			changeState(MAIN);
		}
		else if (pbP1HP->value() == 0)
		{
			QMessageBox::information(this, tr("抱歉"), tr("您战败了"));
			changeState(MAIN);
		}
		client->write("done", BUF_LENGTH);

		break;
	}
	default:
		break;
	}
}
