#include <iostream>
#include "battlecontroller.h"
#include <ctime>

using namespace std;

int main()
{
	srand(time(NULL));
	cout << "init pokemon races ...\n";
	Race<0> race1;
	Race<1> race2;
	Race<2> race3;
	Race<3> race4;

	Pokemon pokemon1(race1, "pokemon1");
	Pokemon pokemon2(race2, "pokemon2");
	Pokemon pokemon3(race3);
	Pokemon pokemon4(race4);

	pokemon1.gainExp(1000);
	 pokemon2.gainExp(1000);
	// pokemon3.gainExp(100);
	// pokemon4.gainExp(100);

	//  BattleController battle(pokemon1, pokemon2);				//auto fight
	BattleController battle(pokemon1, pokemon2, false); //manual fight

	battle.start();

	system("pause");

	return 0;
}