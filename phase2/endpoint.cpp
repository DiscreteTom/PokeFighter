#define _CRT_SECURE_NO_WARNINGS // ignore strcpy errors

#include "endpoint.h"
#include <iostream>
// #include <mstcpip.h> // for socket keep_alive
#include "mystringfunc.h"

using namespace std;

Endpoint::Endpoint(int _playerID, sqlite3 *&_db) : db(_db), playerID(_playerID)
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
	if (listen(endpointSocket, SERVER_REQ_QUEUE_LENGTH) == SOCKET_ERROR)
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
	char buf[BUF_LENGTH] = "";
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
		// TODO
		auto strs = split(buf);
		if (strs[0] == "logout")
		{
			running = false;
		}
		else
		{
			cout << "Endpoint[" << playerID << "]: Invalid request.\n";
			strcpy(buf, "Reject: Invalid request.\n");
			send(connSocket, buf, BUF_LENGTH, 0);
		}
		if (running)
			ret = recv(connSocket, buf, BUF_LENGTH, 0);
	}
	if (ret == SOCKET_ERROR || ret == 0)
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
	if (!cv.wait_for(lock, 10m, [this] { return online; }))
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