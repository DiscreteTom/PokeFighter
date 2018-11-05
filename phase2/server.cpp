#define _WINSOCK_DEPRECATED_NO_WARNINGS // ignore inet_ntoa errors
#define _CRT_SECURE_NO_WARNINGS					// ignore strcpy errors

#include "server.h"
#include "mystringfunc.h"
#include <conio.h>
#include <thread>
#include <iostream>

using namespace std;

Server::Server()
{
}

Server::~Server()
{
	closesocket(serverSocket); // if serverSocket has been closed, return WSAENOTSOCK
	closesocket(connSocket);
	//stop socket DLL

	/**
	 * WSACleanup
	 * - return 0 if WSACleanup has NOT been called(succeed)
	 * - return -1 if WSACleanup has been called
	*/
	while (WSACleanup() != -1)
		;
}

void Server::start()
{
	// open database
	if (sqlite3_open("server.db", &db))
	{
		cout << "Server: Can NOT open database: " << sqlite3_errmsg(db) << endl;
		return;
	}

	//init socket DLL
	cout << "Server: Init socket DLL...";
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		cout << "Server: Init network protocol failed.\n";
		system("pause");
		return;
	}
	cout << "Done.\n";

	/**
	 * init server socket
	 * function: socket(int domain, int type, int protocol);
	 * domain: AF_INET or PF_INET
	 *   - AF for Address Family
	 *   - PF for Protocol Family
	 *   - in Windows, AF_INET == PF_INET
	 * type: SOCK_STREAM or SOCK_DGRAM or SOCK_RAW
	 * protocol: use IPPROTO_TCP for TCP/IP
	*/
	cout << "Server: Init server socket...";
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET)
	{
		cout << "Server: Init socket failed.\n";
		closesocket(serverSocket);
		WSACleanup();
		system("pause");
		return;
	}
	cout << "Done.\n";

	// construct an address, including protocol & IP address & port
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // any ip address

	// bind socket to an address
	cout << "Server: Socket binding...";
	if (::bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "Server: Socket bind failed.\n";
		closesocket(serverSocket);
		WSACleanup();
		system("pause");
		return;
	}
	cout << "Done.\n";

	// if request queue is full, client will get error: WSAECONNREFUSED
	cout << "Server: Socket listen...";
	if (listen(serverSocket, SERVER_REQ_QUEUE_LENGTH) == SOCKET_ERROR)
	{
		cout << WSAGetLastError();
		cout << "Server: Socket listen failed.\n";
		closesocket(serverSocket);
		WSACleanup();
		system("pause");
		return;
	}
	cout << "Done.\n";

	// now listen successfully
	cout << "\nServer: Server is running at " << SERVER_PORT << endl;
	cout << "Press any key to stop server.\n\n";

	// init thread
	running = true;
	thread listenThread(&Server::listenFunc, this);
	thread terminateThread(&Server::terminateFunc, this);
	listenThread.join();
	terminateThread.join();

	closesocket(serverSocket);
	WSACleanup();

	sqlite3_close(db);

	cout << "Server stoped.\n\n";
}

void Server::listenFunc()
{
	while (running)
	{
		// link
		sockaddr_in clientAddr; // client address
		int clientAddrLength = sizeof(clientAddr);
		connSocket = accept(serverSocket, (sockaddr *)&clientAddr, &clientAddrLength);
		if (connSocket == INVALID_SOCKET)
		{
			if (running)
			{
				// if not running, this thread must be terminated by terminateFunc
				// in that case the string below is not needed
				cout << "Server: Link to client failed.\n";
			}
			closesocket(connSocket);
			continue;
		}

		// link successfully
		cout << "Server: " << inet_ntoa(clientAddr.sin_addr) << " connected.\n";

		/**
		 * process data
		 * format:
		 * - "login\n<username>\n<password>"
		 * - "logon\n<username>\n<password>"
		*/
		recv(connSocket, buf, BUF_LENGTH, 0);
		auto strs = split(buf, '\n');
		if (strs.size() < 3)
		{
			//error
			cout << "Server: Invalid request.\n";
			strcpy(buf, "Reject: Invalid request.\n");
			send(connSocket, buf, BUF_LENGTH, 0);
		}
		else if (strs[0] == "login")
			login(strs[1], strs[2]);
		else if (strs[0] == "logon")
			logon(strs[1], strs[2]);
		else
		{
			cout << "Server: Invalid request.\n";
			strcpy(buf, "Reject: Invalid request.\n");
			send(connSocket, buf, BUF_LENGTH, 0);
		}
		closesocket(connSocket);
	}
}

void Server::terminateFunc()
{
	_getch();
	running = false;
	closesocket(serverSocket);
}

void Server::login(const string &username, const string &password)
{
}
void Server::logon(const string &username, const string &password)
{
	if (!isValid(username))
	{
		cout << "Server: Got an invalid username: " << username << endl;
		strcpy(buf, "Reject: Invalid username.\n");
	}
	else if (!isValid(password))
	{
		cout << "Server: Got an invalid password";
		strcpy(buf, "Reject: Invalid password.\n");
	}
	else
	{
		char **sqlResult;
		int nRow;
		int nColumn;
		char *errMsg;
		string sql = "SELECT name FROM User WHERE name = '" + username + "'";
		if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
		{
			cout << "Server: Sqlite3 error: " << errMsg << endl;
			sqlite3_free(errMsg);
		}
		else
		{
			if (nRow == 0)
			{
				// username NOT exist, add this user
				//TODO
			}
			else
			{
				// username already exist
				cout << "Server: Logon: username already exist.\n";
				strcpy(buf, "Reject: duplicate username.\n");
			}
			sqlite3_free_table(sqlResult);
		}
	}
	send(connSocket, buf, BUF_LENGTH, 0);
}

bool Server::isValid(const string &str)
{
	for (auto c : str)
	{
		if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'))
		{
			//not a letter or a digit or '_'
			return false;
		}
	}
	return true;
}
