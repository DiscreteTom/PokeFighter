#define _WINSOCK_DEPRECATED_NO_WARNINGS // ignore inet_ntoa errors
#define _CRT_SECURE_NO_WARNINGS					// ignore strcpy errors

#include "server.h"
#include "mystringfunc.h"
#include <conio.h>
#include <thread>
#include <iostream>

using namespace std;

Server &Server::getInstance()
{
	static Server result;
	return result;
}

Server::~Server()
{
#pragma region delete endpoints
	// all endpoints should be destroyed in Server::start()
	// add these lines here just in case :)
	mtx.lock();
	while (endpoints.size())
	{
		auto p = endpoints.back();
		delete p;
		endpoints.pop_back();
	}
	mtx.unlock();
#pragma endregion

	closesocket(serverSocket); // if socket has been closed, return WSAENOTSOCK, but that's ok
	closesocket(connSocket);

	/**
	 * WSACleanup, stop socket DLL
	 * - return 0 if WSACleanup has NOT been called(succeed)
	 * - return -1 if WSACleanup has been called
	*/
	while (WSACleanup() != -1)
		;
}

void Server::start()
{
#pragma region open database
	cout << "Server: Init database...";
	if (sqlite3_open("server.db", &db))
	{
		cout << "\nServer: Can NOT open database: " << sqlite3_errmsg(db) << endl;
		return;
	}
	cout << "Done.\n";
#pragma endregion

	// init socket DLL
	cout << "Server: Init socket DLL...";
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		cout << "\nServer: Init network protocol failed.\n";
		// system("pause");
		return;
	}
	cout << "Done.\n";

#pragma region init socket
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
		cout << "\nServer: Init socket failed.\n";
		closesocket(serverSocket);
		WSACleanup();
		// system("pause");
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
	// namespace conflicts: thread::bind and global::bind
	if (::bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "\nServer: Socket bind failed.\n";
		closesocket(serverSocket);
		WSACleanup();
		// system("pause");
		return;
	}
	cout << "Done.\n";

	// if request queue is full, client will get error: WSAECONNREFUSED
	cout << "Server: Socket listen...";
	if (listen(serverSocket, SERVER_REQ_QUEUE_LENGTH) == SOCKET_ERROR)
	{
		cout << WSAGetLastError();
		cout << "\nServer: Socket listen failed.\n";
		closesocket(serverSocket);
		WSACleanup();
		// system("pause");
		return;
	}
	cout << "Done.\n";
#pragma endregion

	// now listen successfully
	cout << "\nServer: Server is running at " << SERVER_PORT << endl;
	cout << "Press any key to stop server.\n\n";

	// init thread
	running = true;
	thread listenThread(&Server::listenFunc, this);
	thread terminateThread(&Server::terminateFunc, this);
	listenThread.join();
	terminateThread.join();

	// destroy all endpoints
	mtx.lock();
	while (endpoints.size())
	{
		auto p = endpoints.back();
		delete p;
		endpoints.pop_back();
	}
	mtx.unlock();

	closesocket(serverSocket);
	WSACleanup();

	sqlite3_close(db);

	cout << "\nServer: Server stoped.\n";
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
			break;
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
		auto strs = split(buf);
		if (strs.size() == 1 && strs[0].length() == 0)
		{
			// blank request, maybe client is closed
		}
		else if (strs.size() < 3)
		{
			//error
			cout << "Server: Invalid request: " << buf << endl;
			strcpy(buf, "Reject: Invalid request.\n");
			send(connSocket, buf, BUF_LENGTH, 0);
		}
		else if (strs[0] == "login")
			login(strs[1], strs[2]);
		else if (strs[0] == "logon")
			logon(strs[1], strs[2]);
		else
		{
			cout << "Server: Invalid request: " << buf << endl;
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
	if (!isValid(username))
	{
		cout << "Server: Got an invalid username: " << username << endl;
		strcpy(buf, "Reject: Invalid username.\n");
	}
	else if (!isValid(password))
	{
		cout << "Server: Got an invalid password: " << password << endl;
		strcpy(buf, "Reject: Invalid password.\n");
	}
	else
	{
		char **sqlResult;
		int nRow;
		int nColumn;
		char *errMsg;
		string sql = "SELECT id FROM User WHERE name = '" + username + "' AND password = '" + password + "'";
		if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
		{
			cout << "Server: Sqlite3 error: " << errMsg << endl;
			strcpy(buf, "Reject: Server database error.\n");
			sqlite3_free(errMsg);
		}
		else // sqlite select succeed
		{
			if (nRow == 0)
			{
				// username and password mismatch
				cout << "Server: Login: username '" << username << "' and password '" << password << "' mismatch.\n";
				strcpy(buf, "Reject: Username and password dismatch.\n");
			}
			else
			{
				// username exist
				// check user state
				bool userExist = false;
				mtx.lock();
				int id = atoi(sqlResult[1]); // sqlResult[0] == "id", sqlResult[1] == playerID
				for (auto endpoint : endpoints)
				{
					if (endpoint->getPlayerID() == id)
					{
						userExist = true;
						if (endpoint->isOnline())
						{
							strcpy(buf, "Reject: Account is already online.\n");
						}
						else
						{
							// not online, return port
							strcpy(buf, to_string(endpoint->getPort()).c_str());
						}
						break;
					}
				}
				mtx.unlock();

				if (!userExist) // add an endpoint
				{
					auto p = new Endpoint(id, db);
					int endpointPort = p->start();
					if (endpointPort == 0) // start ERROR, remove and delete this new endpoint
					{
						delete p;
						strcpy(buf, "Reject: Server endpoint error.\n");
					}
					else // start normally, add this endpoint to endpoints
					{
						lock_guard<mutex> lock(mtx);
						endpoints.push_back(p);
						strcpy(buf, to_string(endpointPort).c_str());
						thread th(&Server::mornitor, this, p);
						th.detach();
					}
				}
			}
			sqlite3_free_table(sqlResult);
		}
	}
	send(connSocket, buf, BUF_LENGTH, 0);
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
		cout << "Server: Got an invalid password: " << password << endl;
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
			strcpy(buf, "Reject: Server database error.\n");
			sqlite3_free(errMsg);
		}
		else
		{
			if (nRow == 0)
			{
				// username NOT exist, add this user
				string sql = "INSERT INTO User(name, password) VALUES('" + username + "', '" + password + "');";
				char *errMsg;
				if (sqlite3_exec(db, sql.c_str(), nonUseCallback, NULL, &errMsg) != SQLITE_OK)
				{
					cout << "Server: Sqlite3 error: " << errMsg << endl;
					strcpy(buf, "Reject: Server database error.\n");
				}
				else
				{
					cout << "Server: Add user: " << username << " password: " << password << endl;
					strcpy(buf, "Accept.\n");
				}
			}
			else
			{
				// username already exist
				cout << "Server: Logon: username '" << username << "' already exist.\n";
				strcpy(buf, "Reject: Duplicate username.\n");
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
		if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'))
		{
			//not a letter or a digit or '_'
			return false;
		}
	}
	return true;
}

void Server::mornitor(Endpoint *const endpoint)
{
	endpoint->process();

	// now endpoint reaches end
	mtx.lock();
	// remove from endpoints
	for (int i = 0; i < endpoints.size(); ++i)
	{
		if (endpoints[i] == endpoint)
		{
			endpoints.erase(endpoints.begin() + i);
			// if endpoints doesn't contain endpoint, that means endpoint has been deleted in ~Server()
			delete endpoint;
			break;
		}
	}
	mtx.unlock();
}