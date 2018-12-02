#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
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

	// logon window
	logonDlg = new LogonDlg(this);

	connect(btnPlay, &QPushButton::clicked, this, [this]{ changeState(LOGIN); });
	connect(btnLogon, &QPushButton::clicked, this, [this]{
		if (logonDlg->exec() == QDialog::Accepted){
			leUsername->setText(logonDlg->getUsername());
			lePassword->setText(logonDlg->getPassword());
		}
	});
	connect(btnExit, &QPushButton::clicked, this, [this]{ qApp->quit(); });
	connect(btnBack, &QPushButton::clicked, this, [this]{ changeState(START); });

	changeState(START);
}

MainWindow::~MainWindow()
{
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

	state = aim;

	// delete old layout
	delete ui->centralWidget->layout();
	layout = new QGridLayout(this);

	// show widgets in the certain state
	switch(state){
	case START:
		lbStartTitle->show();
		btnPlay->show();
		btnExit->show();
		layout->addWidget(lbStartTitle, 0, 0, Qt::AlignCenter);
		layout->addWidget(btnPlay, 1, 0, Qt::AlignCenter);
		layout->addWidget(btnExit, 2, 0, Qt::AlignCenter);
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
		break;
	default:
		break;
	}
	ui->centralWidget->setLayout(layout);
}
