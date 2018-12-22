#ifndef LOGONDLG_H
#define LOGONDLG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QTcpSocket>

namespace Ui
{
class LogonDlg;
}

class LogonDlg : public QDialog
{
	Q_OBJECT

public:
	explicit LogonDlg(QWidget *parent = 0);
	~LogonDlg();

	QString getUsername() const;
	QString getPassword() const;

public slots:
	int exec();
	void reject();

private:
	Ui::LogonDlg *ui;

	QLineEdit *leUsername;
	QLineEdit *lePassword;
	QLineEdit *leRepeat; // repeat password
	QPushButton *btnOK;
	QPushButton *btnCancel;

	QTcpSocket *client;

	void logon();
private slots:
	void readServerMsg();
};

#endif // LOGONDLG_H
