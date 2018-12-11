#pragma once

#include "pokemon.h"
#include "netconfig.h"
#include <string>

// about socket
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

class BattleController
{
private:
	Pokemon &p1, &p2;
	int timer1;
	int timer2;
	char buf[BUF_LENGTH];
	SOCKET &connSocket;
	string msg;

public:
	BattleController(Pokemon &playerPokemon, Pokemon &enemyPokemon, SOCKET &connSocket);
	bool start(); // return true if playerPokemon won
	// bool useSkill(int skillIndex);// return true if battle finished
};