#pragma once

#include <string>
#include "netconfig.h"

// about socket
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

// about database
#include "sqlite3.h"
#pragma comment(lib, "sqlite3.lib")

using namespace std;

/**
 * define interfaces before login
 * use interfaces in class Endpoint after login
*/
class Server
{
private:
	// about socket
	SOCKET serverSocket;
	SOCKET connSocket;
	bool running;

	//about database
	sqlite3 *db;

	// interfaces
	void login(const string &username, const string &password);
	void logon(const string &username, const string &password);

	// authentication
	bool isValid(const string & str);

	// thread function
	void listenFunc();
	void terminateFunc();

public:
	Server();
	~Server();

	void start();
};