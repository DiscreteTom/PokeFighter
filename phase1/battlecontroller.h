#pragma once

#include "pokemon.h"

class BattleController
{
private:
	Pokemon &p1, p2;
	bool _auto; //auto fight
	int timer1;
	int timer2;

public:
	BattleController(Pokemon &pokemon1, Pokemon &pokemon2, bool autoFight = true);
	void start();
};