#pragma once

#include <string>
#include <vector>
#include <mutex>
#include "netconfig.h"

// about socket
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

// about database
#include "sqlite3.h"
#pragma comment(lib, "sqlite3.lib")

// about endpoint
#include "endpoint.h"

using namespace std;

/**
 * define interfaces before login
 * use interfaces in class Endpoint after login
 * 
 * this class is Singleton
*/
class Hub
{
private:
	// about network
	const int HUB_PORT = 7500;
	SOCKET hubSocket;
	SOCKET connSocket;
	volatile bool running;
	char buf[BUF_LENGTH];

	//about database
	sqlite3 *db;

	// about endpoint
	vector<Endpoint *> endpoints;

	// about thread
	mutex mtx; // to protect endpoints

	// interfaces
	void login(const string &username, const string &password);
	void logon(const string &username, const string &password);

	// authentication
	bool isValidUsername(const string &str);
	bool isValidPassword(const string &str);

	// thread function
	void listenFunc();
	void terminateFunc();
	void mornitor(Endpoint *const endpoint);

	// for singleton
	Hub(){};
	Hub(Hub const &) = delete;
	Hub(Hub &&) = delete;
	Hub &operator=(Hub const &) = delete;
	~Hub();

public:
	static Hub &getInstance();

	void start();				 // init database and socket
	string getAllUser(); // for endpoint::getPlayerList
};

inline int nonUseCallback(void *notUsed, int argc, char **argv, char **azColName) { return 0; }