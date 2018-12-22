#ifndef POKEMONDLG_H
#define POKEMONDLG_H

#include <QDialog>
#include <QTableWidget>
#include <QLabel>

namespace Ui
{
class PokemonDlg;
}

// dialog about pokemon detail
class PokemonDlg : public QDialog
{
	Q_OBJECT

public:
	explicit PokemonDlg(const QString &detail, bool myPokemonTable = true, QWidget *parent = 0);
	~PokemonDlg();

	QTableWidget *table; // pokemon detail table
	QLabel *lbImg;			 // pokemon image

	QString getID() { return table->item(0, 0)->text(); }
	const QPixmap *getPixmap() { return lbImg->pixmap(); }
	QString getName() { return table->item(1, 0)->text(); }

signals:
	void pokemonChangeName(const QString &pokemonID, const QString &newName);

private:
	Ui::PokemonDlg *ui;
};

#endif // POKEMONDLG_H
