#define _WINSOCK_DEPRECATED_NO_WARNINGS // ignore inet_ntoa errors
#define _CRT_SECURE_NO_WARNINGS					// ignore strcpy errors

#include "server.h"
#include "mystringfunc.h"
#include <conio.h>
#include <thread>
#include <iostream>
#include <map>

using namespace std;

Hub &Hub::getInstance()
{
	static Hub result;
	return result;
}

Hub::~Hub()
{
#pragma region delete endpoints
	// all endpoints should be destroyed in Hub::start()
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

	closesocket(hubSocket); // if socket has been closed, return WSAENOTSOCK, but that's ok
	closesocket(connSocket);

	/**
	 * WSACleanup, stop socket DLL
	 * - return 0 if WSACleanup has NOT been called(succeed)
	 * - return -1 if WSACleanup has been called
	*/
	while (WSACleanup() != -1)
		;
}

void Hub::start()
{
#pragma region open database
	cout << "Hub: Init database...";
	if (sqlite3_open("server.db", &db))
	{
		cout << "\nHub: Can NOT open database: " << sqlite3_errmsg(db) << endl;
		return;
	}
	// create tables whether they are already exist or not
	char *errMsg;
	string sql = "create table User(";
	sql += "id integer primary key,";
	sql += "name text unique not null,";
	sql += "password text not null";
	sql += ");";
	if (sqlite3_exec(db, sql.c_str(), nonUseCallback, NULL, &errMsg) != SQLITE_OK)
	{
		sqlite3_free(errMsg);
	}
	sql = "create table Pokemon(";
	sql += "id integer primary key,";
	sql += "userid integer not null,";
	sql += "name text not null,";
	sql += "race int not null,";
	sql += "atk int not null,";
	sql += "def int not null,";
	sql += "maxHp int not null,";
	sql += "speed int not null,";
	sql += "lv int not null,";
	sql += "exp int not null";
	sql += ");";
	if (sqlite3_exec(db, sql.c_str(), nonUseCallback, NULL, &errMsg) != SQLITE_OK)
	{
		sqlite3_free(errMsg);
	}
	cout << "Done.\n";
#pragma endregion

	// init socket DLL
	cout << "Hub: Init socket DLL...";
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		cout << "\nHub: Init network protocol failed.\n";
		// system("pause");
		return;
	}
	cout << "Done.\n";

#pragma region init socket
	/**
	 * init hub socket
	 * function: socket(int domain, int type, int protocol);
	 * domain: AF_INET or PF_INET
	 *   - AF for Address Family
	 *   - PF for Protocol Family
	 *   - in Windows, AF_INET == PF_INET
	 * type: SOCK_STREAM or SOCK_DGRAM or SOCK_RAW
	 * protocol: use IPPROTO_TCP for TCP/IP
	*/
	cout << "Hub: Init hub socket...";
	hubSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (hubSocket == INVALID_SOCKET)
	{
		cout << "\nHub: Init socket failed.\n";
		closesocket(hubSocket);
		WSACleanup();
		// system("pause");
		return;
	}
	cout << "Done.\n";

	// construct an address, including protocol & IP address & port
	sockaddr_in hubAddr;
	hubAddr.sin_family = AF_INET;
	hubAddr.sin_port = htons(HUB_PORT);
	hubAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // any ip address

	// bind socket to an address
	cout << "Hub: Socket binding...";
	// namespace conflicts: thread::bind and global::bind
	if (::bind(hubSocket, (sockaddr *)&hubAddr, sizeof(hubAddr)) == SOCKET_ERROR)
	{
		cout << "\nHub: Socket bind failed.\n";
		closesocket(hubSocket);
		WSACleanup();
		// system("pause");
		return;
	}
	cout << "Done.\n";

	// if request queue is full, client will get error: WSAECONNREFUSED
	cout << "Hub: Socket listen...";
	if (listen(hubSocket, REQ_QUEUE_LENGTH) == SOCKET_ERROR)
	{
		cout << WSAGetLastError();
		cout << "\nHub: Socket listen failed.\n";
		closesocket(hubSocket);
		WSACleanup();
		// system("pause");
		return;
	}
	cout << "Done.\n";
#pragma endregion

	// now listen successfully
	cout << "\nHub: Hub is running at " << HUB_PORT << endl;
	cout << "Press any key to stop hub.\n\n";

	// init thread
	running = true;
	thread listenThread(&Hub::listenFunc, this);
	thread terminateThread(&Hub::terminateFunc, this);
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

	closesocket(hubSocket);
	WSACleanup();

	sqlite3_close(db);

	cout << "\nHub: Hub stoped.\n";
}

void Hub::listenFunc()
{
	while (running)
	{
		// link
		sockaddr_in clientAddr; // client address
		int clientAddrLength = sizeof(clientAddr);
		connSocket = accept(hubSocket, (sockaddr *)&clientAddr, &clientAddrLength);
		if (connSocket == INVALID_SOCKET)
		{
			if (running)
			{
				// if not running, this thread must be terminated by terminateFunc
				// in that case the string below is not needed
				cout << "Hub: Link to client failed.\n";
			}
			closesocket(connSocket);
			break;
		}

		// link successfully
		cout << "Hub: " << inet_ntoa(clientAddr.sin_addr) << " connected.\n";

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
			cout << "Hub: Invalid request: " << buf << endl;
			strcpy(buf, "Reject: Invalid request.\n");
			send(connSocket, buf, BUF_LENGTH, 0);
		}
		else if (strs[0] == "login")
			login(strs[1], strs[2]);
		else if (strs[0] == "logon")
			logon(strs[1], strs[2]);
		else
		{
			cout << "Hub: Invalid request: " << buf << endl;
			strcpy(buf, "Reject: Invalid request.\n");
			send(connSocket, buf, BUF_LENGTH, 0);
		}
		closesocket(connSocket);
	}
}

void Hub::terminateFunc()
{
	_getch();
	running = false;
	closesocket(hubSocket);
}

void Hub::login(const string &username, const string &password)
{
	if (!isValidUsername(username))
	{
		cout << "Hub: Got an invalid username: " << username << endl;
		strcpy(buf, "Reject: Invalid username.\n");
	}
	else if (!isValidPassword(password))
	{
		cout << "Hub: Got an invalid password: " << password << endl;
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
			cout << "Hub: Sqlite3 error: " << errMsg << endl;
			// strcpy(buf, "Reject: Hub database error.\n");
			strcpy(buf, "服务器数据库错误");
			sqlite3_free(errMsg);
		}
		else // sqlite select succeed
		{
			if (nRow == 0)
			{
				// username and password mismatch
				cout << "Hub: Login: username '" << username << "' and password '" << password << "' mismatch.\n";
				// strcpy(buf, "Reject: Username and password dismatch.\n");
				strcpy(buf, "用户名或密码错误");
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
							// strcpy(buf, "Reject: Account is already online.\n");
							strcpy(buf, "用户已在其他设备登录");
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
					auto p = new Endpoint(id, db, *this);
					int endpointPort = p->start();
					if (endpointPort == 0) // start ERROR, remove and delete this new endpoint
					{
						delete p;
						// strcpy(buf, "Reject: Hub endpoint error.\n");
						strcpy(buf, "服务器错误");
					}
					else // start normally, add this endpoint to endpoints
					{
						lock_guard<mutex> lock(mtx);
						endpoints.push_back(p);
						strcpy(buf, to_string(endpointPort).c_str());
						thread th(&Hub::mornitor, this, p);
						th.detach();
					}
				}
			}
			sqlite3_free_table(sqlResult);
		}
	}
	send(connSocket, buf, BUF_LENGTH, 0);
}
void Hub::logon(const string &username, const string &password)
{
	if (!isValidUsername(username))
	{
		cout << "Hub: Got an invalid username: " << username << endl;
		// strcpy(buf, "Reject: Invalid username.\n");
		strcpy(buf, "不合法的用户名");
	}
	else if (!isValidPassword(password))
	{
		cout << "Hub: Got an invalid password: " << password << endl;
		// strcpy(buf, "Reject: Invalid password.\n");
		strcpy(buf, "不合法的密码");
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
			cout << "Hub: Sqlite3 error: " << errMsg << endl;
			// strcpy(buf, "Reject: Hub database error.\n");
			strcpy(buf, "服务器数据库错误");
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
					cout << "Hub: Sqlite3 error: " << errMsg << endl;
					sqlite3_free(errMsg);
					strcpy(buf, "服务器数据库错误");
					// strcpy(buf, "Reject: Hub database error.\n");
				}
				else
				{
					cout << "Hub: Add user: " << username << " password: " << password << endl;
					strcpy(buf, "Accept.\n");
				}
			}
			else
			{
				// username already exist
				cout << "Hub: Logon: username '" << username << "' already exist.\n";
				//strcpy(buf, "Reject: Duplicate username.\n");
				strcpy(buf, "用户名已存在");
			}
			sqlite3_free_table(sqlResult);
		}
	}
	send(connSocket, buf, BUF_LENGTH, 0);
}

bool Hub::isValidUsername(const string &str)
{
	for (auto c : str)
	{
		if (c == '\b' || c == '\n' || c == '\t')
		{
			// contains \b \n \t
			return false;
		}
	}
	return true;
}

bool Hub::isValidPassword(const string &str)
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

void Hub::mornitor(Endpoint *const endpoint)
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
			// if endpoints doesn't contain endpoint, that means endpoint has been deleted in ~Hub()
			delete endpoint;
			break;
		}
	}
	mtx.unlock();
}

/**
 * format:
 * <userID> <userName> <online:0|1>
 */
string Hub::getAllUser()
{
	struct temp{
		string name;
		bool online;
	};
	char **sqlResult;
	int nRow;
	int nColumn;
	char *errMsg;
	string sql = "SELECT id, name FROM User;";
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
	{
		cout << "Hub: Sqlite3 error: " << errMsg << endl;
		// strcpy(buf, "Reject: Hub database error.\n");
		// strcpy(buf, "服务器数据库错误");
		sqlite3_free(errMsg);
	}

	map<int, temp> playerMap;
	for (int i = 0; i < nRow; ++i){
		temp t = {sqlResult[2 * (i + 1) + 1], false};
		playerMap.insert(make_pair(stoi(sqlResult[2 * (i + 1)]), t));
	}

	sqlite3_free_table(sqlResult);


	string result;
	mtx.lock();
	for (auto endpoint : endpoints)
	{
		playerMap[endpoint->getPlayerID()].online = true;
	}
	mtx.unlock();

	for (auto & player : playerMap){
		result += to_string(player.first);
		result += ' ';
		result += player.second.name;
		result += ' ';
		if (player.second.online)result += '1';
		else result += '0';
		result += '\n';
	}

	return result;
}