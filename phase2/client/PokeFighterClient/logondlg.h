#ifndef LOGONDLG_H
#define LOGONDLG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

namespace Ui {
class LogonDlg;
}

class LogonDlg : public QDialog
{
	Q_OBJECT

public:
	explicit LogonDlg(QWidget *parent = 0);
	~LogonDlg();

public slots:
	int exec();

private:
	Ui::LogonDlg *ui;

	QLineEdit * leUsername;
	QLineEdit * lePassword;
	QLineEdit * leRepeat; // repeat password
	QPushButton * btnOK;
	QPushButton * btnCancel;

	void logon();
};

#endif // LOGONDLG_H
