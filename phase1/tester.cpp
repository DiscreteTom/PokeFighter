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

	Pokemon pokemon1(race1);
	Pokemon pokemon2(race2);

	pokemon1.gainExp(100);
	pokemon2.gainExp(100);

	//  BattleController battle(pokemon1, pokemon2);				//auto fight
	BattleController battle(pokemon1, pokemon2, false); //manual fight

	battle.start();

	system("pause");

	return 0;
}