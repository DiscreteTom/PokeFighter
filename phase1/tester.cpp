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
	Race_3 race3;
	Race_4 race4;

	Pokemon pokemon1(race1);
	Pokemon pokemon2(race2);
	Pokemon pokemon3(race3);
	Pokemon pokemon4(race4);

	pokemon1.gainExp(1000);
	pokemon2.gainExp(1000);
	pokemon3.gainExp(1000);
	pokemon4.gainExp(1000);

	//  BattleController battle(pokemon1, pokemon2);				//auto fight
	BattleController battle(pokemon1, pokemon3, false); //manual fight

	battle.start();

	system("pause");

	return 0;
}