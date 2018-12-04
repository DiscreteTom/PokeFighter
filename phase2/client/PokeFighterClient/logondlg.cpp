#include "logondlg.h"
#include "ui_logondlg.h"
#include <QGridLayout>
#include <QHostAddress>
#include "netconfig.h"

LogonDlg::LogonDlg(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::LogonDlg)
{
	ui->setupUi(this);

	setWindowTitle(tr("新用户注册"));

	// about ui
	leUsername = new QLineEdit(this);
	lePassword = new QLineEdit(this);
	leRepeat = new QLineEdit(this);
	btnOK = new QPushButton(tr("确认"), this);
	btnCancel = new QPushButton(tr("取消"), this);
	leUsername->setPlaceholderText(tr("请输入用户名"));
	leUsername->setToolTip(tr("不能包含空白字符，长度为6-30位"));
	lePassword->setPlaceholderText(tr("请输入密码"));
	lePassword->setToolTip(tr("由字母、数字、下划线组成，长度为6-30位"));
	leRepeat->setPlaceholderText(tr("请再次输入密码以确认"));
	lePassword->setEchoMode(QLineEdit::Password);
	leRepeat->setEchoMode(QLineEdit::Password);

	connect(btnOK, &QPushButton::clicked, this, &LogonDlg::logon);
	connect(btnCancel, &QPushButton::clicked, this, [this]{ reject(); });

	// about layout
	QGridLayout * layout = new QGridLayout(this);
	layout->addWidget(leUsername, 0, 0);
	layout->addWidget(lePassword, 1, 0);
	layout->addWidget(leRepeat, 2, 0);
	layout->addWidget(btnOK, 3, 0);
	layout->addWidget(btnCancel, 4, 0);
	setLayout(layout);

	// about network
	client = new QTcpSocket(this);
	connect(client, &QTcpSocket::readyRead, this, &LogonDlg::readServerMsg);
}

LogonDlg::~LogonDlg()
{
	delete client;
	delete ui;
}

QString LogonDlg::getUsername() const
{
	return leUsername->text();
}

QString LogonDlg::getPassword() const
{
	return lePassword->text();
}

void LogonDlg::logon()
{
	if (!isValidUsername(leUsername->text())){
		QMessageBox::warning(this, tr("不合法的用户名"), tr("账号必需由字母、数字、下划线组成且长度介于6-30"));
		return;
	} else if (!isValidPassword(lePassword->text())){
		QMessageBox::warning(this, tr("不合法的密码"), tr("密码必需由字母、数字、下划线组成且长度介于6-30"));
		return;
	} else if (lePassword->text() != leRepeat->text()){
		QMessageBox::warning(this, tr("错误"), tr("两次输入密码不一致"));
		return;
	}
	client->connectToHost(QHostAddress("127.0.0.1"), 7500);

	QString msg = "logon";
	msg += ' ';
	msg += leUsername->text();
	msg += ' ';
	msg += lePassword->text();

	btnOK->setDisabled(true);

	if (client->write(msg.toLocal8Bit(), BUF_LENGTH) == -1){
		// error occur
		QMessageBox::warning(this, tr("错误"), tr("服务器出错"));
		btnOK->setDisabled(false);
	}
}

bool LogonDlg::isValidPassword(const QString &str)
{
	if (str.length() > 30 || str.length() < 6)
		return false;
	for (auto c : str){
		if (c != '_' && !(c <= 'z' && c >= 'a') && !(c <= 'Z' && c >= 'A') && !(c >= '0' && c <= '9'))
			return false;
	}
	return true;
}

bool LogonDlg::isValidUsername(const QString &str)
{
	if (str.length() > 30 || str.length() < 6){
		return false;
	}
	for (auto c : str){
		if (c == '\t' || c == '\b' || c == '\t'){
			return false;
		}
	}
	return true;
}


void LogonDlg::readServerMsg()
{
	auto ret = client->read(BUF_LENGTH);

	btnOK->setDisabled(false);
	client->disconnectFromHost();

	if (QString(ret) == "Accept.\n"){
		QMessageBox::information(this, tr("注册成功"), tr("注册成功"));
		accept();
	}
	else{
		QMessageBox::warning(this, tr("错误"), QString::fromLocal8Bit(ret));
	}

}

int LogonDlg::exec()
{
	leUsername->clear();
	lePassword->clear();
	leRepeat->clear();
	leUsername->setFocus();
	return QDialog::exec();
}

void LogonDlg::reject()
{
	client->disconnectFromHost();
	QDialog::reject();
}
