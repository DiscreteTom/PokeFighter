#pragma once

#include "pokemon.h"

class BattleController
{
private:
	Pokemon &p1, &p2;
	int timer1;
	int timer2;

public:
	BattleController(Pokemon &playerPokemon, Pokemon &enemyPokemon);
	void start(); 
};