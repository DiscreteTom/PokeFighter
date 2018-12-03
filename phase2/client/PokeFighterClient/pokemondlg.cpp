#include "pokemondlg.h"
#include "ui_pokemondlg.h"

PokemonDlg::PokemonDlg(const QString &detail, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::PokemonDlg)
{
	ui->setupUi(this);

	table = new QTableWidget(this);
	table->setColumnCount(9);
	table->setRowCount(1);
	table->setHorizontalHeaderLabels({tr("精灵ID"), tr("名字"), tr("种族"), tr("攻击力"), tr("防御力"), tr("生命值"), tr("速度"), tr("等级"), tr("经验")});

	auto details = detail.split(' ');
	for (int i = 0; i < 9; ++i){
		table->setItem(0, i, new QTableWidgetItem(details[i]));
	}

	setAttribute(Qt::WA_DeleteOnClose);

	show();
}

PokemonDlg::~PokemonDlg()
{
	delete ui;
}