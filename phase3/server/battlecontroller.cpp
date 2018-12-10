#include "battlecontroller.h"

BattleController::BattleController(Pokemon &playerPokemon, Pokemon &enemyPokemon) : p1(pokemon1), p2(pokemon2)
{
}

void BattleController::start()
{
	// dbout << p1.name() << " VS " << p2.name() << "!\n";
	// dbout << "Battle Start!\n\n";

	p1.restoreAll();
	p2.restoreAll();

	timer1 = 0;
	timer2 = 0;

	while (1)
	{
		while (timer1 < p1.cspeed() && timer2 < p2.cspeed())
		{
			++timer1;
			++timer2;
		}

		if (timer1 >= p1.cspeed() && timer2 >= p2.cspeed())
		{
			if (p1.cspeed() >= p2.cspeed())
			{
				if (p1.attack(p2, false))// player manual fight
					break;
				if (p2.attack(p1, true)) // enemy auto fight
					break;
			}
			else
			{
				if (p2.attack(p1, true))
					break;
				if (p1.attack(p2, false))
					break;
			}
			timer1 = timer2 = 0;
		}
		else if (timer1 >= p1.cspeed())
		{
			//p2 attack
			if (p2.attack(p1, true))
				break;
			timer1 = 0;
		}
		else
		{
			if (p1.attack(p2, false))
				break;
			timer2 = 0;
		}
	}

	if (p1.hp())
	{
		// dbout << p1.name() << " won!\n\n";
		return;
	}
	else
	{
		// dbout << p2.name() << " won!\n\n";
		return;
	}
}
