#pragma once

#include "netconfig.h"
#include <thread>
#include <mutex>
#include <string>

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
	bool running;

	// about player
	int playerID;

	// about thread
	mutex mtx;

	// thread function
	void timer();
	void listenFunc();

public:
	Endpoint(int _playerID, sqlite3 *&_db);
	~Endpoint();

	int start();		// return port, return 0 if ERROR
	void process(); // return to delete this endpoint
};