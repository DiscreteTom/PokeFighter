#pragma once

#include "netconfig.h"
#include <thread>
#include <mutex>
#include <string>
#include <condition_variable>
#include "pokemon.h"

// about socket
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

// about database
#include "sqlite3.h"
#pragma comment(lib, "sqlite3.lib")

using namespace std;

class Hub;

class Endpoint
{
	// about server
	Hub &hub;

	// about database
	sqlite3 *&db;

	// about network
	int port;
	string ip;
	SOCKET endpointSocket;
	SOCKET connSocket;
	volatile bool running;
	volatile bool online;
	char buf[BUF_LENGTH];

	// about player
	int playerID;
	string playerUsername;
	bool isDuel;

	// about thread
	mutex mtx;
	condition_variable cv;
	volatile bool timing; // thread function timer is running

	// thread function
	void timer();
	void listenFunc();

	// interface function
	void resetPassword(const string &oldPassword, const string &newPassword);
	void getPlayerList();
	void getPokemonList(int playerID);
	void getPokemonByID(int pokemonID);
	void pokemonChangeName(const string &pokemonID, const string &newName);
	void battle(int pokemonID, bool autoFight = false);
	void useSkill(int skillID);
	void getDuelStatistic();
	void battle(const string &pokemonID, int enemyRaceID, int enemyLV);
	void chooseBet();
	void discard(const string &pokemonID);

	// other functions
	void savePokemonToDB(const Pokemon &p, int id = -1);

public:
	Endpoint(int playerID, sqlite3 *&db, Hub &hub);
	~Endpoint();

	int start();		// return port, return 0 if ERROR
	void process(); // return to delete this endpoint

	bool isOnline() const { return online; }
	int getPlayerID() const { return playerID; }
	string getPlayerName() const { return playerUsername; }
	int getPort() const { return port; }
};