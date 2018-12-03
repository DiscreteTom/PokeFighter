#ifndef POKEMONDLG_H
#define POKEMONDLG_H

#include <QDialog>
#include <QTableWidget>

namespace Ui {
class PokemonDlg;
}

class PokemonDlg : public QDialog
{
	Q_OBJECT

public:
	explicit PokemonDlg(const QString & detail, QWidget *parent = 0);
	~PokemonDlg();

	QTableWidget * table;

private:
	Ui::PokemonDlg *ui;
};

#endif // POKEMONDLG_H
