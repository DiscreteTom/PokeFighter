#include "battlecontroller.h"

BattleController::BattleController(Pokemon &playerPokemon, Pokemon &enemyPokemon, SOCKET &connSocket) : p1(pokemon1), p2(pokemon2), connSocket(connSocket)
{
}

bool BattleController::start()
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
				recv(connSocket, buf, BUF_LENGTH, 0);
				int index;
				sscanf(buf, "%d", index);
				if (p1.attack(p2, index)) // player manual fight
					break;
				if (p2.attack(p1)) // enemy auto fight
					break;
			}
			else
			{
				if (p2.attack(p1))
					break;
				recv(connSocket, buf, BUF_LENGTH, 0);
				int index;
				sscanf(buf, "%d", index);
				if (p1.attack(p2, index))
					break;
			}
			timer1 = timer2 = 0;
		}
		else if (timer1 >= p1.cspeed())
		{
			//p2 attack
			if (p2.attack(p1))
				break;
			timer1 = 0;
		}
		else
		{
			recv(connSocket, buf, BUF_LENGTH, 0);
			int index;
			sscanf(buf, "%d", index);
			if (p1.attack(p2, index))
				break;
			timer2 = 0;
		}
	}

	if (p1.hp())
	{
		// dbout << p1.name() << " won!\n\n";
		return true;
	}
	else
	{
		// dbout << p2.name() << " won!\n\n";
		return false;
	}
}
