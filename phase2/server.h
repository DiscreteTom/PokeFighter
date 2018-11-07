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
*/
class Server
{
private:
	// about network
	SOCKET serverSocket;
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
	bool isValid(const string &str);

	// thread function
	void listenFunc();
	void terminateFunc();
	void mornitor(Endpoint *const endpoint);

public:
	Server();
	~Server();

	void start(); // init database and socket
};

inline int nonUseCallback(void *notUsed, int argc, char **argv, char **azColName) { return 0; }