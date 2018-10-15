#pragma once

#include "pokemon.h"

class BattleController
{
private:
	static const int MAX_TIMER = 100;
	Pokemon &p1, p2;
	bool _auto; //auto fight
	int timer1;
	int timer2;

public:
	BattleController(Pokemon &pokemon1, Pokemon &pokemon2, bool autoFight = true);
	void start(); 
};