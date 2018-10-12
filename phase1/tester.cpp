#include <iostream>
#include "pokemon.h"

using namespace std;

int main()
{
	cout << "init pokemon races ...\n";
	Race_1 race1;
	Race_2 race2;

	Pokemon pokemon1(&race1, "WC");
	Pokemon pokemon2(&race2, "DJ");

	pokemon1.attack(&pokemon2, 0);
	system("pause");
	pokemon2.attack(&pokemon1, 1);
	system("pause");
	pokemon1.attack(&pokemon2, 1);
	system("pause");
	pokemon2.attack(&pokemon1, 3);
	system("pause");
	pokemon1.attack(&pokemon2, 2);
	system("pause");
	pokemon2.attack(&pokemon1, 0);
	system("pause");
	pokemon1.attack(&pokemon2, 3);
	system("pause");
	pokemon2.attack(&pokemon1, 2);
	system("pause");
}