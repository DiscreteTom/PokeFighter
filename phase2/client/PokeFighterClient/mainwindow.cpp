#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QHostAddress>
#include "netconfig.h"
#include "pokemondlg.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
																					ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	// set style sheet
	QFile file(":/qss/main.qss");
	file.open(QFile::ReadOnly);
	QTextStream filetext(&file);
	QString stylesheet = filetext.readAll();
	this->setStyleSheet(stylesheet);
	file.close();

	setWindowTitle(tr("宠物小精灵对战程序客户端"));

	// start layout
	lbStartTitle = new QLabel(tr("宠物小精灵对战程序"), this);
	btnPlay = new QPushButton(tr("开始游戏"), this);
	btnExit = new QPushButton(tr("退出"), this);

	// login layout
	lbLoginLabel = new QLabel(tr("登录"), this);
	leUsername = new QLineEdit(this);
	leUsername->setPlaceholderText(tr("请输入用户名"));
	lePassword = new QLineEdit(this);
	lePassword->setPlaceholderText(tr("请输入密码"));
	lePassword->setEchoMode(QLineEdit::Password);
	btnLogin = new QPushButton(tr("登录"), this);
	btnLogon = new QPushButton(tr("注册"), this);
	btnBack = new QPushButton(tr("返回"), this);

	// main layout
	btnLogout = new QPushButton(tr("退出登录"), this);
	btnShowPokemonList = new QPushButton(tr("查看精灵"), this);
	btnDisplayAllPlayer = new QPushButton(tr("查看当前在线玩家"), this);

	// pokemon table and player table
	table = new QTableWidget(this);

	// logon window
	logonDlg = new LogonDlg(this);

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
			if (myPokemonTable)
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
		myPokemonTable = true;
		client->write("getPokemonList", BUF_LENGTH);
	});
	connect(btnDisplayAllPlayer, &QPushButton::clicked, this, [this] {
		changeState(PLAYER_TABLE);
		client->write("getPlayerList", BUF_LENGTH);
	});
	connect(leUsername, &QLineEdit::returnPressed, btnLogin, &QPushButton::click);
	connect(lePassword, &QLineEdit::returnPressed, btnLogin, &QPushButton::click);

	client = new QTcpSocket(this);
	connect(client, &QTcpSocket::readyRead, this, &MainWindow::getServerMsg);

	changeState(START);

	changingPokemonName = false;
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
	btnDisplayAllPlayer->hide();
	table->hide();
	table->clear();

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
		layout->addWidget(lbStartTitle, 0, 0, Qt::AlignCenter);
		layout->addWidget(btnPlay, 1, 0, Qt::AlignCenter);
		layout->addWidget(btnExit, 2, 0, Qt::AlignCenter);
		btnPlay->setDefault(true);
		break;
	case LOGIN:
		lbLoginLabel->show();
		leUsername->show();
		lePassword->show();
		btnLogin->show();
		btnLogon->show();
		btnBack->show();
		layout->addWidget(lbLoginLabel, 0, 0, Qt::AlignCenter);
		layout->addWidget(leUsername, 1, 0, Qt::AlignCenter);
		layout->addWidget(lePassword, 2, 0, Qt::AlignCenter);
		layout->addWidget(btnLogin, 3, 0, Qt::AlignCenter);
		layout->addWidget(btnLogon, 4, 0, Qt::AlignCenter);
		layout->addWidget(btnBack, 5, 0, Qt::AlignCenter);
		btnLogin->setDefault(true);
		leUsername->setFocus();
		break;
	case MAIN:
		btnLogout->show();
		btnShowPokemonList->show();
		btnDisplayAllPlayer->show();
		layout->addWidget(btnShowPokemonList, 0, 0, Qt::AlignCenter);
		layout->addWidget(btnDisplayAllPlayer, 1, 0, Qt::AlignCenter);
		layout->addWidget(btnLogout, 2, 0, Qt::AlignCenter);
		btnShowPokemonList->setDefault(true);
		break;
	case POKEMON_TABLE:
	case PLAYER_TABLE:
		btnBack->show();
		table->show();
		layout->addWidget(table, 0, 0, Qt::AlignCenter);
		layout->addWidget(btnBack, 1, 0, Qt::AlignCenter);
		btnBack->setDefault(true);
		break;
	default:
		break;
	}
	ui->centralWidget->setLayout(layout);

	// special settings, must behind ui->centralWidget->setLayout()
	if (state == LOGIN)
	{
		leUsername->setFocus();
	} else if (state == MAIN){
		btnShowPokemonList->setFocus();
	}
}

void MainWindow::login()
{
	client->connectToHost(QHostAddress("127.0.0.1"), 7500);
	QString msg = "login";
	msg += ' ';
	msg += leUsername->text();
	msg += ' ';
	msg += lePassword->text();

	btnLogin->setDisabled(true);

	if (client->write(msg.toStdString().c_str(), BUF_LENGTH) == -1)
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

	QString msg(ret);

	if (changingPokemonName){
		if (msg != "Accept.\n"){
			QMessageBox::warning(this, tr("错误"), tr("修改精灵名字失败"));
		} else {
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
			QMessageBox::warning(this, tr("错误"), tr("用户名或密码错误"));
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
		auto players = msg.split('\n');

		//! players[players.size() - 1] == ""

		table->setRowCount(players.size() - 2);
		table->setColumnCount(3); // id - username - viewPokemon
		table->setHorizontalHeaderLabels({tr("玩家ID"), tr("用户名"), tr("操作")});

		int tableRowIndex = 0;
		for (int i = 0; i < players.size() - 1; ++i)
		{
			auto player = players[i];
			auto detail = player.split(' ');
			// detail[0] is id, detail[1] is username
			if (detail[1] == username)
				continue; // not show player himself
			table->setItem(tableRowIndex, 0, new QTableWidgetItem(detail[0]));
			table->setItem(tableRowIndex, 1, new QTableWidgetItem(detail[1]));
			auto btn = new QPushButton(tr("查看小精灵"), this);
			connect(btn, &QPushButton::clicked, this, [this, detail] {
				changeState(POKEMON_TABLE);
				myPokemonTable = false;
				QString str = "getPokemonList ";
				str += detail[0];
				client->write(str.toStdString().c_str(), BUF_LENGTH);
			});
			table->setCellWidget(tableRowIndex, 2, btn);
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
			table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
			table->verticalHeader()->hide();
			for (int i = 0; i < pokemons.size() - 1; ++i)
			{
				auto detail = pokemons[i].split(' ');
				// detail[0] is name, detail[1] is race, detail[2] is lv
				table->setItem(i, 0, new QTableWidgetItem(detail[0]));
				table->setItem(i, 1, new QTableWidgetItem(detail[1]));
				table->setItem(i, 2, new QTableWidgetItem(detail[2]));
				table->setItem(i, 3, new QTableWidgetItem(detail[3]));
				auto btn = new QPushButton(tr("查看详情"), this);
				connect(btn, &QPushButton::clicked, this, [this, detail] {
					QString str = "getPokemon ";
					str += detail[0];
					client->write(str.toStdString().c_str(), BUF_LENGTH);
					showPokemonDlg = true;
				});
				table->setCellWidget(i, 4, btn);
			}
		}
		else // msg is pokemon detail
		{
			auto dlg = new PokemonDlg(msg, myPokemonTable, this);
			connect(dlg, &PokemonDlg::pokemonChangeName, this, [this](const QString & pokemonID, const QString & newName){
				QString str = "pokemonChangeName ";
				str += pokemonID + ' ' + newName;
				client->write(str.toStdString().c_str(), BUF_LENGTH);
				changingPokemonName = true;
			});
			showPokemonDlg = false;
		}
		break;
	}
	default:
		break;
	}
}
