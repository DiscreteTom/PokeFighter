#ifndef POKEMONDLG_H
#define POKEMONDLG_H

#include <QDialog>
#include <QTableWidget>
#include <QLabel>

namespace Ui
{
class PokemonDlg;
}

// dialog to show pokemon details
class PokemonDlg : public QDialog
{
	Q_OBJECT

public:
	explicit PokemonDlg(const QString &detail, bool myPokemonTable = true, QWidget *parent = 0);
	~PokemonDlg();

	QTableWidget *table; // attribute table
	QLabel *lbImg;			 // race image

signals:
	void pokemonChangeName(const QString &pokemonID, const QString &newName);

private:
	Ui::PokemonDlg *ui;
};

#endif // POKEMONDLG_H
