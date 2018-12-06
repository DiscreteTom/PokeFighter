#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
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
			changeState(MAIN);
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
	connect(btnLvUpBattle, &QPushButton::clicked, this, [this]{ changeState(LV_UP_BATTLE); });
	connect(btnDuelBattle, &QPushButton::clicked, this, [this]{ changeState(DUEL_BATTLE); });
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

void MainWindow::changeState(MainWindow::State aim)
{
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
		btnBack->show();
		table->show();
		layout->addWidget(table, 0, 0);
		layout->addWidget(btnBack, 1, 0);
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
		 * <playerID> <playername> <online:0|1>
		 */
		auto players = msg.split('\n');

		//! players[players.size() - 1] == ""

		table->setRowCount(players.size() - 2);
		table->setColumnCount(4); // id - username - online - viewPokemon
		table->setHorizontalHeaderLabels({tr("玩家ID"), tr("用户名"), tr("在线情况"), tr("操作")});
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
			auto btn = new QPushButton(tr("查看小精灵"), this);
			connect(btn, &QPushButton::clicked, this, [this, detail] {
				changeState(POKEMON_TABLE);
				currentPlayerID = detail[0].toInt();
				QString str = "getPokemonList ";
				str += detail[0];
				client->write(str.toStdString().c_str(), BUF_LENGTH);
			});
			table->setCellWidget(tableRowIndex, 3, btn);
			++tableRowIndex;
		}
		break;
	}
	case POKEMON_TABLE:
	{
		if (!showPokemonDlg) // msg is pokemon table
		{
			auto pokemons = msg.split('\n');

			table->setRowCount(pokemons.size() - 1);
			table->setColumnCount(5);
			table->setHorizontalHeaderLabels({tr("精灵ID"), tr("名字"), tr("种族"), tr("等级"), tr("操作")});
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
			}

			connect(table, &QTableWidget::cellChanged, this, [this](int row, int column) {
				if (state == POKEMON_TABLE && column == 1)
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
				for (int i = 0; i < table->rowCount(); ++i){
					if (table->item(i, 0)->text() == pokemonID){
						table->item(i, 1)->setText(newName);// will call table::cellChanged
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
	default:
		break;
	}
}
