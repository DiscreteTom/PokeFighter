#include "battlecontroller.h"

BattleController::BattleController(Pokemon &pokemon1, Pokemon &pokemon2, bool autoFight) : p1(pokemon1), p2(pokemon2)
{
	_auto = autoFight;
}

void BattleController::start()
{
	p1.restoreAll();
	p2.restoreAll();

	timer1 = 0;
	timer2 = 0;

	while (1)
	{
		while (timer1 < MAX_TIMER && timer2 < MAX_TIMER)
		{
			timer1 += p1.speed();
			timer2 += p2.speed();
		}
		if (timer1 >= MAX_TIMER && timer2 >= MAX_TIMER)
		{
			if (p1.speed() >= p2.speed())
			{
				if (p1.attack(p2))
					break;
				if (p2.attack(p1))
					break;
			}
			else
			{
				if (p2.attack(p1))
					break;
				if (p1.attack(p2))
					break;
			}
			timer1 -= MAX_TIMER;
			timer2 -= MAX_TIMER;
		}
		else if (timer1 >= MAX_TIMER)
		{
			if (p1.attack(p2))
				break;
			timer1 -= MAX_TIMER;
		}
		else
		{
			if (p2.attack(p1))
				break;
			timer2 -= MAX_TIMER;
		}
	}

	if (p1.hp())
	{
		msg << p1.name() << " won!\n";
		return;
	}
	else
	{
		msg << p2.name() << "won!\n";
		return;
	}
}
