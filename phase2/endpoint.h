#pragma once

#include "netconfig.h"
#include <thread>
#include <mutex>
#include <string>
#include <condition_variable>

// about socket
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

// about database
#include "sqlite3.h"
#pragma comment(lib, "sqlite3.lib")

using namespace std;

class Endpoint
{
	// about database
	sqlite3 *&db;

	// about network
	int port;
	string ip;
	SOCKET endpointSocket;
	SOCKET connSocket;
	volatile bool running;
	volatile bool online;

	// about player
	int playerID;

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
	void battle(int pokemonID, bool autoFight = false);
	void useSkill(int skillID);

public:
	Endpoint(int playerID, sqlite3 *&db);
	~Endpoint();

	int start();		// return port, return 0 if ERROR
	void process(); // return to delete this endpoint

	bool isOnline() const { return online; }
	int getPlayerID() const { return playerID; }
	int getPort() const { return port; }
};