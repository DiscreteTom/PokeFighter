#include "battlecontroller.h"

BattleController::BattleController(Pokemon &pokemon1, Pokemon &pokemon2, bool autoFight) : p1(pokemon1), p2(pokemon2)
{
	_auto = autoFight;
}

void BattleController::start()
{
	dbout << p1.name() << " VS " << p2.name() << "!\n";
	dbout << "Battle Start!\n\n";

	p1.restoreAll();
	p2.restoreAll();

	timer1 = p1.speed();
	timer2 = p2.speed();

	while (1)
	{
		while (timer1 > 0 && timer2 > 0)
		{
			--timer1;
			--timer2;
		}

		if (!timer1 && !timer2)
		{
			if (p1.speed() >= p2.speed())
			{
				if (p1.attack(p2, _auto))
					break;
				if (p2.attack(p1, _auto))
					break;
			}
			else
			{
				if (p2.attack(p1, _auto))
					break;
				if (p1.attack(p2, _auto))
					break;
			}
		}
		else if (!timer1)
		{
			//p2 attack
			if (p2.attack(p1, _auto))
				break;
		}
		else
		{
			if (p1.attack(p2, _auto))
				break;
		}
	}

	if (p1.hp())
	{
		dbout << p1.name() << " won!\n\n";
		return;
	}
	else
	{
		dbout << p2.name() << " won!\n\n";
		return;
	}
}
