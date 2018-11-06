#pragma once

#include <string>
#include <vector>
//#include "netconfig.h" // DO NOT INCLUDE THIS

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
 * Network config
*/
const int SERVER_PORT = 7500;
const int SERVER_REQ_QUEUE_LENGTH = 4; // server request queue max length, usually 2-4
const int BUF_LENGTH = 1024;					 // server buffer length MUST equals client buffer length

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
	bool running;
	char buf[BUF_LENGTH];

	//about database
	sqlite3 *db;

	// about endpoint
	vector<Endpoint> endpoints;

	// interfaces
	void login(const string &username, const string &password);
	void logon(const string &username, const string &password);

	// authentication
	bool isValid(const string &str);

	// thread function
	void listenFunc();
	void terminateFunc();

public:
	Server();
	~Server();

	void start(); // init database and socket
};

inline int nonUseCallback(void *notUsed, int argc, char **argv, char **azColName) { return 0; }