#include <iostream>
#include "battlecontroller.h"
#include <ctime>

using namespace std;

int main()
{
	srand(time(NULL));
	cout << "init pokemon races ...\n";
	Race_1 race1;
	Race_2 race2;

	Pokemon pokemon1(race1, "DJ");
	Pokemon pokemon2(race2, "WC");

	pokemon1.getExp(1000);
	pokemon2.getExp(1000);

	// BattleController battle(pokemon1, pokemon2);				//auto fight
	BattleController battle(pokemon1, pokemon2, false); //manual fight

	battle.start();
	battle.start();

	system("pause");

	return 0;
}