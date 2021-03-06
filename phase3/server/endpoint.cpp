#define _CRT_SECURE_NO_WARNINGS // ignore strcpy errors

#include "endpoint.h"
#include <iostream>
// #include <mstcpip.h> // for socket keep_alive
#include "mystringfunc.h"
#include "server.h"
#include "battlecontroller.h"

using namespace std;

Endpoint::Endpoint(int playerID, sqlite3 *&db, Hub &hub) : playerID(playerID), db(db), hub(hub)
{
	port = 0;
	running = false;
}

Endpoint::~Endpoint()
{
	running = false;
	while (timing)
	{
		// cout << "Try to stop timer.\n";
		unique_lock<mutex> lock(mtx);
		online = true;
		lock.unlock();
		cv.notify_one();
		lock.lock();
	}

	closesocket(endpointSocket);

	if (port)
		cout << "Endpoint[" << playerID << "]: Endpoint stoped at " << port << endl;
}

int Endpoint::start()
{
	// get playerUsername
	char **sqlResult;
	int nRow;
	int nColumn;
	char *errMsg;
	string sql = "SELECT name FROM User where id=" + to_string(playerID) + ";";
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
		return 0;
	}
	else if (nRow == 0)
	{
		cout << "Endpoint[" << playerID << "]: Database content error.\n";
		sqlite3_free_table(sqlResult);
		return 0;
	}
	else
	{
		playerUsername = sqlResult[1];
		sqlite3_free_table(sqlResult);
	}

	/**
	 * init endpoint socket
	 * 
	 * function: socket(int domain, int type, int protocol);
	 * domain: AF_INET or PF_INET
	 *   - AF for Address Family
	 *   - PF for Protocol Family
	 *   - in Windows, AF_INET == PF_INET
	 * type: SOCK_STREAM or SOCK_DGRAM or SOCK_RAW
	 * protocol: use IPPROTO_TCP for TCP/IP
	*/
	// cout << "Endpoint[" << playerID << "]: init socket...";
	endpointSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (endpointSocket == INVALID_SOCKET)
	{
		cout << "Endpoint[" << playerID << "]: Init socket failed.\n";
		closesocket(endpointSocket);
		// system("pause");
		return 0;
	}
	// cout << "Done.\n";

	// construct an address, including protocol & IP address & port
	sockaddr_in endpointAddr;
	endpointAddr.sin_family = AF_INET;
	endpointAddr.sin_port = htons(0);											 // port = 0 so windows will give us a free port
	endpointAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // any ip address

#pragma region set socket keep_alive
/**
 * I spend hours for the 40 lines of code below
 * And when I finish this module
 * I try to run without this block of code
 * IT STILL WORKS!
 * DAMN!
*/

// 	int iOptVal = 0;
// 	int iOptLen = sizeof(int);
// 	BOOL bOptVal = FALSE;
// 	int bOptLen = sizeof(BOOL);
// 	if (getsockopt(endpointSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&iOptVal, &iOptLen) == SOCKET_ERROR)
// 	{
// 		cout << "Endpoint[" << playerID << "]: Set keep_alive failed: " << WSAGetLastError() << "\n";
// 		closesocket(endpointSocket);
// 		return 0;
// 	}
// 	if (setsockopt(endpointSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&bOptVal, bOptLen) == SOCKET_ERROR)
// 	{
// 		cout << "Endpoint[" << playerID << "]: Set keep_alive failed: " << WSAGetLastError() << "\n";
// 		closesocket(endpointSocket);
// 		return 0;
// 	}
// 	/**
// 	 * set KeepAlive parameter
// 	 *
// 	 * struct tcp_keepalive{
// 	 *   ULONG onoff; // TRUE or FALSE
// 	 *   ULONG keepalivetime; // after this time(ms) without data, send heart-beat
// 	 *   ULONG keepaliveinterval; // heart-beat interval(ms)
// 	 * };
// 	 */
// 	tcp_keepalive alive_in;
// 	tcp_keepalive alive_out;
// 	alive_in.keepalivetime = 500;			 // 0.5s
// 	alive_in.keepaliveinterval = 1000; //1s
// 	alive_in.onoff = TRUE;						 // turn on keep_alive
// 	unsigned long ulBytesReturn = 0;
// 	if (WSAIoctl(endpointSocket, SIO_KEEPALIVE_VALS, &alive_in, sizeof(alive_in), &alive_out, sizeof(alive_out), &ulBytesReturn, NULL, NULL) == SOCKET_ERROR)
// 	{
// 		cout << "Endpoint[" << playerID << "]: Set keep_alive failed: " << WSAGetLastError() << "\n";
// 		closesocket(endpointSocket);
// 		return 0;
// 	}
#pragma endregion

	// bind socket to an address
	// cout << "Endpoint[" << playerID << "]: Socket binding...";
	if (::bind(endpointSocket, (sockaddr *)&endpointAddr, sizeof(endpointAddr)) == SOCKET_ERROR)
	{
		cout << "Endpoint[" << playerID << "]: Socket bind failed.\n";
		closesocket(endpointSocket);
		// system("pause");
		return 0;
	}
	// cout << "Done.\n";

	// now we get a free port by OS
	int endpointAddrLength = sizeof(endpointAddr);
	getsockname(endpointSocket, (sockaddr *)&endpointAddr, &endpointAddrLength);
	port = ntohs(endpointAddr.sin_port);

	// if request queue is full, client will get error: WSAECONNREFUSED
	// cout << "Endpoint[" << playerID << "]: Socket listen...";
	if (listen(endpointSocket, REQ_QUEUE_LENGTH) == SOCKET_ERROR)
	{
		cout << WSAGetLastError();
		cout << "Endpoint[" << playerID << "]: Socket listen failed.\n";
		closesocket(endpointSocket);
		// system("pause");
		return 0;
	}
	// cout << "Done.\n";

	// now listen successfully
	cout << "Endpoint[" << playerID << "]: Endpoint is running at " << port << "\n";

	running = true; // enable Endpoint::process()

	return port;
}

void Endpoint::process()
{
	while (running)
	{
		online = false;
		timing = true;

		thread timerThread(&Endpoint::timer, this);
		thread listenThread(&Endpoint::listenFunc, this);
		timerThread.join();
		listenThread.join();
	}
}

void Endpoint::listenFunc()
{
	// link
	sockaddr_in clientAddr; // client address
	int clientAddrLength = sizeof(clientAddr);
	connSocket = accept(endpointSocket, (sockaddr *)&clientAddr, &clientAddrLength);
	while (timing)
	{
		// cout << "Try to stop timer.\n";
		unique_lock<mutex> lock(mtx);
		online = true;
		lock.unlock();
		cv.notify_one();
		lock.lock();
	}
	if (connSocket == INVALID_SOCKET)
	{
		// TODO
		return;
	}

	// link successfully
	int ret = recv(connSocket, buf, BUF_LENGTH, 0);
	/**
	 * recv(connSocket, buf, BUF_LENGTH, 0)
	 * - return bytes of buf
	 * - return 0 if client socket closed or get an empty line
	 * - return SOCKET_ERROR(-1) if server socket is closed or client unexpectedly terminated
	*/
	while (ret != 0 && ret != SOCKET_ERROR && running) // normal
	{
		// parse command here
		auto strs = split(buf);
		if (strs[0] == "logout")
		{
			running = false;
		}
		else if (strs[0] == "getPlayerList")
		{
			getPlayerList();
		}
		else if (strs[0] == "resetPassword" && strs.size() == 3)
		{
			resetPassword(strs[1], strs[2]);
		}
		else if (strs[0] == "getPokemonList" && strs.size() < 3)
		{
			if (strs.size() == 2)
			{
				getPokemonList(stoi(strs[1]));
			}
			else
			{
				getPokemonList(playerID);
			}
		}
		else if (strs[0] == "getPokemon" && strs.size() == 2)
		{
			getPokemonByID(stoi(strs[1]));
		}
		else if (strs[0] == "pokemonChangeName" && strs.size() == 3)
		{
			pokemonChangeName(strs[1], strs[2]);
		}
		else if (strs[0] == "getDuelStatistic")
		{
			getDuelStatistic();
		}
		else if (strs[0] == "battle" && strs.size() == 5)
		{
			if (strs[1] == "DUEL")
				isDuel = true;
			else
				isDuel = false;
			battle(strs[2], stoi(strs[3]), stoi(strs[4]));
		}
		else if (strs[0] == "chooseBet" && strs.size() == 1)
		{
			chooseBet();
		}
		else if (strs[0] == "discard" && strs.size() == 2)
		{
			discard(strs[1]);
		}
		else
		{
			cout << "Endpoint[" << playerID << "]: Invalid request.\n";
			strcpy(buf, "Reject: Invalid request.\n");
			// send(connSocket, buf, BUF_LENGTH, 0);
		}
		if (running)
			ret = recv(connSocket, buf, BUF_LENGTH, 0);
	}
	if (!running)
		; // this object was destroyed by dtor
	else if (ret == SOCKET_ERROR || ret == 0)
	{
		cout << "Endpoint[" << playerID << "]: Client unexpected offline, start timing.\n";
	}
	else
	{
		// running == false, user logout
		cout << "Endpoint[" << playerID << "]: User logout.\n";
	}
	closesocket(connSocket);
}

void Endpoint::timer()
{
	using namespace std::chrono_literals;

	// cout << "Start timing.\n";

	/**
	 * wait for player re-login for 10 minutes
	 * 
	 * condition_variable::wait_for(lock, time, condition);
	 * - lock is for variables in this function
	 *   - now lock is for bool running
	 * - return false when time out and condition == false
	 * - return true when otherwise
	*/
	unique_lock<mutex> lock(mtx);
	if (!cv.wait_for(lock, 10min, [this] { return online; }))
	{
		// player is offline
		running = false;
		timing = false;
		closesocket(endpointSocket);
		// cout << "Time up.\n";
	}
	else
	{
		timing = false;
		// cout << "Stop timing.\n";
	}
}

void Endpoint::resetPassword(const string &oldPassword, const string &newPassword)
{
	// check oldPassword
	char **sqlResult;
	int nRow;
	int nColumn;
	char *errMsg;
	string sql;
	sql = "SELECT name FROM User where id=" + to_string(playerID) + " and password='" + oldPassword + "';";
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
		// strcpy(buf, "Reject: Server error.\n");
		strcpy(buf, "服务器错误");
		send(connSocket, buf, BUF_LENGTH, 0);
		return;
	}
	else if (nRow == 0)
	{
		// wrong password
		sqlite3_free_table(sqlResult);
		// strcpy(buf, "Reject: wrong old password.\n");
		strcpy(buf, "旧密码不正确");
		send(connSocket, buf, BUF_LENGTH, 0);
		return;
	}
	else
	{
		sqlite3_free_table(sqlResult);
	}

	// update password
	sql = "update User set password='" + newPassword + "' where id=" + to_string(playerID) + ";";
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
		// strcpy(buf, "Reject: Server error.\n");
		strcpy(buf, "服务器错误");
		send(connSocket, buf, BUF_LENGTH, 0);
		return;
	}
	else
	{
		sqlite3_free_table(sqlResult);
		strcpy(buf, "Accept.\n");
		send(connSocket, buf, BUF_LENGTH, 0);
	}
	return;
}

void Endpoint::getPlayerList()
{
	strcpy(buf, hub.getAllUser().c_str());
	send(connSocket, buf, BUF_LENGTH, 0);
}

void Endpoint::getPokemonList(int playerID)
{
	// get all pokemon from database
	char **sqlResult;
	int nRow;
	int nColumn;
	char *errMsg;
	string sql;
	sql = "SELECT id, name, race, lv FROM Pokemon where userid=" + to_string(playerID) + ";";
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
		strcpy(buf, "Reject: Server error.\n");
		send(connSocket, buf, BUF_LENGTH, 0);
		return;
	}
	if (nRow < 3 && playerID == this->playerID)
	{
		// add pokemons for user till his pokemon number be 3
		for (int i = 0; i < 3 - nRow; ++i)
		{
			int raceIndex = rand() % 4;
			Pokemon t(raceIndex);
			savePokemonToDB(t);
		}
		getPokemonList(playerID);
	}
	else
	{
		string result;
		for (int i = 0; i < nRow; ++i)
		{
			result += sqlResult[4 * (i + 1)]; // id
			result += ' ';
			result += sqlResult[4 * (i + 1) + 1]; // name
			result += ' ';
			result += Pokemon::races[stoi(sqlResult[4 * (i + 1) + 2])]->raceName(); // race
			result += ' ';
			result += sqlResult[4 * (i + 1) + 3]; // lv
			result += '\n';
		}
		strcpy(buf, result.c_str());
		send(connSocket, buf, BUF_LENGTH, 0);
	}
	sqlite3_free_table(sqlResult);
}

void Endpoint::savePokemonToDB(const Pokemon &p, int id)
{
	if (id == -1) // pokemon not exist, insert it to database
	{
		string sql = "INSERT INTO Pokemon(userid, name, race, atk, def, maxHp, speed, lv, exp) VALUES('";
		sql += to_string(playerID) + "','";
		sql += p.name() + "',";
		sql += to_string(p.raceIndex()) + ",";
		sql += to_string(p.atk()) + ",";
		sql += to_string(p.def()) + ",";
		sql += to_string(p.maxHp()) + ",";
		sql += to_string(p.speed()) + ",";
		sql += to_string(p.lv()) + ",";
		sql += to_string(p.exp()) + ");";
		char *errMsg;
		if (sqlite3_exec(db, sql.c_str(), nonUseCallback, NULL, &errMsg) != SQLITE_OK)
		{
			cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
			sqlite3_free(errMsg);
		}
	}
	else // pokemon already exist, update it
	{
		string sql = "update Pokemon set atk=";
		sql += to_string(p.atk()) + ", def=";
		sql += to_string(p.def()) + ", maxHp=";
		sql += to_string(p.maxHp()) + ", speed=";
		sql += to_string(p.speed()) + ", lv=";
		sql += to_string(p.lv()) + ", exp=";
		sql += to_string(p.exp());
		sql += " where id=";
		sql += to_string(id) + ";";
		char *errMsg;
		if (sqlite3_exec(db, sql.c_str(), nonUseCallback, NULL, &errMsg) != SQLITE_OK)
		{
			cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
			sqlite3_free(errMsg);
		}
	}
}

void Endpoint::getPokemonByID(int pokemonID)
{
	char **sqlResult;
	int nRow;
	int nColumn;
	char *errMsg;
	string sql;
	sql = "SELECT id, name, race, atk, def, maxHp, speed, lv, exp FROM Pokemon where id=" + to_string(pokemonID) + ";";
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
		strcpy(buf, "Reject: Server error.\n");
		send(connSocket, buf, BUF_LENGTH, 0);
		return;
	}
	string result;
	result += sqlResult[9 + 0]; // id
	result += ' ';
	result += sqlResult[9 + 1]; // name
	result += ' ';
	result += Pokemon::races[stoi(sqlResult[9 + 2])]->raceName(); // race
	result += ' ';
	result += sqlResult[9 + 3]; // atk
	result += ' ';
	result += sqlResult[9 + 4]; // def
	result += ' ';
	result += sqlResult[9 + 5]; // maxHp
	result += ' ';
	result += sqlResult[9 + 6]; // speed
	result += ' ';
	result += sqlResult[9 + 7]; // lv
	result += ' ';
	result += sqlResult[9 + 8]; // exp
	result += '\n';
	strcpy(buf, result.c_str());
	send(connSocket, buf, BUF_LENGTH, 0);
	sqlite3_free_table(sqlResult);
}

void Endpoint::pokemonChangeName(const string &pokemonID, const string &newName)
{
	string sql = "update Pokemon set name = '" + newName + "' where id=" + pokemonID + ";";

	char *errMsg;
	if (sqlite3_exec(db, sql.c_str(), nonUseCallback, NULL, &errMsg) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
		strcpy(buf, errMsg);
		send(connSocket, buf, BUF_LENGTH, 0);
	}
	else
	{
		strcpy(buf, "Accept.\n");
		send(connSocket, buf, BUF_LENGTH, 0);
	}
}

void Endpoint::getDuelStatistic()
{
	char **sqlResult;
	int nRow;
	int nColumn;
	char *errMsg;
	string sql;
	sql = "SELECT win, total FROM User where id=" + to_string(playerID) + ";";
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
		strcpy(buf, "Reject: Server error.\n");
		send(connSocket, buf, BUF_LENGTH, 0);
		return;
	}
	string result = sqlResult[2];
	result += ' ';
	result += sqlResult[3];
	strcpy(buf, result.c_str());
	send(connSocket, buf, BUF_LENGTH, 0);
	sqlite3_free_table(sqlResult);
}

void Endpoint::battle(const string &pokemonID, int enemyRaceID, int enemyLV)
{
	char **sqlResult;
	int nRow;
	int nColumn;
	char *errMsg;
	string sql;
	sql = "SELECT id, name, race, atk, def, maxHp, speed, lv, exp FROM Pokemon where id=" + pokemonID + ";";
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
		strcpy(buf, "Reject: Server error.\n");
		send(connSocket, buf, BUF_LENGTH, 0);
		return;
	}
	// construct player pokemon
	Pokemon p1 = Pokemon(sqlResult[10], stoi(sqlResult[11]), stoi(sqlResult[12]), stoi(sqlResult[13]), stoi(sqlResult[14]), stoi(sqlResult[15]), stoi(sqlResult[16]), stoi(sqlResult[17]));

	int currentPokemonID = stoi(sqlResult[9]);

	string result = "";
	result += p1.raceName() + ' ';
	result += to_string(p1.maxHp()) + ' ';
	for (int i = 0; i < 4; ++i)
	{
		result += p1.skillName(i) + ' ';
		result += p1.skillDscp(i) + ' ';
	}
	result += to_string(p1.pp(0)) + ' ';
	result += to_string(p1.pp(1)) + ' ';
	result += to_string(p1.pp(2)) + ' ';
	result += to_string(p1.lv()) + '\n';

	// construct enemy pokemon
	Pokemon *p2 = Pokemon::getEnemy(enemyRaceID, enemyLV);

	result += p2->raceName() + ' ';
	result += to_string(p2->maxHp()) + ' ';

	strcpy(buf, result.c_str());
	send(connSocket, buf, BUF_LENGTH, 0);

	// construct battle controller
	BattleController battle = BattleController(p1, *p2, connSocket);

	char **sqlResult2;
	int nRow2;
	int nColumn2;
	char *errMsg2;
	string sql2 = "select win, total from User where id=" + to_string(playerID) + ";";
	if (sqlite3_get_table(db, sql2.c_str(), &sqlResult2, &nRow2, &nColumn2, &errMsg2) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg2 << endl;
		sqlite3_free(errMsg2);
		strcpy(buf, "Reject: Server error.\n");
		send(connSocket, buf, BUF_LENGTH, 0);
		return;
	}

	if (battle.start())
	{
		// win
		if (isDuel)
		{
			// change win rate
			string sql3 = "update User set win=" + to_string(stoi(sqlResult2[2]) + 1) + ", total=" + to_string(stoi(sqlResult2[3]) + 1) + " where id=" + to_string(playerID) + ";";
			char *errMsg3;
			if (sqlite3_exec(db, sql3.c_str(), nonUseCallback, NULL, &errMsg3) != SQLITE_OK)
			{
				cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg3 << endl;
				sqlite3_free(errMsg3);
			}

			// get a new pokemon
			savePokemonToDB(*p2);
		}

		// p1 gain exp
		if (p1.lv() > p2->lv() + 5)
		{
			// no exp
		}
		else if (p1.lv() >= p2->lv())
		{
			p1.gainExp((p2->lv() - p1.lv() + 5) + 3 + f(2));
		}
		else
		{
			p1.gainExp((p2->lv() - p1.lv()) * 5 + f(5));
		}
	}
	else
	{
		//lose
		// change win rate
		string sql3 = "update User set total=" + to_string(stoi(sqlResult2[3]) + 1) + " where id=" + to_string(playerID) + ";";
		char *errMsg3;
		if (sqlite3_exec(db, sql3.c_str(), nonUseCallback, NULL, &errMsg3) != SQLITE_OK)
		{
			cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg3 << endl;
			sqlite3_free(errMsg3);
		}
	}
	sqlite3_free_table(sqlResult2);

	savePokemonToDB(p1, currentPokemonID);
	delete p2;

	sqlite3_free_table(sqlResult);
}

void Endpoint::chooseBet()
{
	// get all pokemon id
	char **sqlResult;
	int nRow;
	int nColumn;
	char *errMsg;
	string sql;
	sql = "SELECT id FROM Pokemon where userid=" + to_string(playerID) + ";";
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
		strcpy(buf, "Reject: Server error.\n");
		send(connSocket, buf, BUF_LENGTH, 0);
		return;
	}

	vector<string> ids;
	for (int i = 0; i < nRow; ++i)
	{
		ids.push_back(sqlResult[i + 1]);
	}

	// get 3 random id
	int id[3];
	int index[3];
	for (int i = 0; i < 3; ++i)
	{
		index[i] = rand() % ids.size();
		id[i] = stoi(ids[index[i]]);
		ids.erase(ids.begin() + index[i]);
	}

	sqlite3_free_table(sqlResult);

	// get 3 pokemon details
	for (int i = 0; i < 3; ++i)
	{
		getPokemonByID(id[i]);
		recv(connSocket, buf, BUF_LENGTH, 0); // read done
	}
}

void Endpoint::discard(const string &pokemonID)
{
	string sql = "delete from Pokemon where id=";
	sql += pokemonID + ";";
	char *errMsg;
	if (sqlite3_exec(db, sql.c_str(), nonUseCallback, NULL, &errMsg) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
	}
}