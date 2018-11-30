#include "logondlg.h"
#include "ui_logondlg.h"
#include <QGridLayout>

LogonDlg::LogonDlg(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::LogonDlg)
{
	ui->setupUi(this);

	setWindowTitle(tr("新用户注册"));

	leUsername = new QLineEdit(this);
	lePassword = new QLineEdit(this);
	leRepeat = new QLineEdit(this);
	btnOK = new QPushButton(tr("确认"), this);
	btnCancel = new QPushButton(tr("取消"), this);
	leUsername->setPlaceholderText(tr("请输入用户名"));
	lePassword->setPlaceholderText(tr("请输入密码"));
	leRepeat->setPlaceholderText(tr("请再次输入密码以确认"));
	lePassword->setEchoMode(QLineEdit::Password);
	leRepeat->setEchoMode(QLineEdit::Password);

	connect(btnOK, &QPushButton::clicked, this, &LogonDlg::logon);
	connect(btnCancel, &QPushButton::clicked, this, [this]{
		emit rejected();
		hide();
	});

	QGridLayout * layout = new QGridLayout(this);
	layout->addWidget(leUsername, 0, 0);
	layout->addWidget(lePassword, 1, 0);
	layout->addWidget(leRepeat, 2, 0);
	layout->addWidget(btnOK, 3, 0);
	layout->addWidget(btnCancel, 4, 0);
	setLayout(layout);
}

LogonDlg::~LogonDlg()
{
	delete ui;
}

void LogonDlg::logon()
{
	// TODO
	return;
}

int LogonDlg::exec()
{
	leUsername->clear();
	lePassword->clear();
	leRepeat->clear();
	leUsername->setFocus();
	return QDialog::exec();
}
