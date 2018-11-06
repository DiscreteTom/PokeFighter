#include "endpoint.h"
#include <iostream>

using namespace std;

Endpoint::Endpoint(int _playerID, sqlite3 *&_db) : db(_db), playerID(_playerID)
{
	port = 0;
	running = false;
}

Endpoint::~Endpoint()
{
	closesocket(endpointSocket);
	if (port)
		cout << "\nEndpoint[" << playerID << "] stoped at " << port << ".\n";
}

int Endpoint::start()
{
	/**
	 * init endpoint socket
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
		system("pause");
		return 0;
	}
	// cout << "Done.\n";

	// construct an address, including protocol & IP address & port
	sockaddr_in endpointAddr;
	endpointAddr.sin_family = AF_INET;
	endpointAddr.sin_port = htons(0);											 // port = 0 so windows will give us a free port
	endpointAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // any ip address

	// bind socket to an address
	// cout << "Endpoint[" << playerID << "]: Socket binding...";
	if (::bind(endpointSocket, (sockaddr *)&endpointAddr, sizeof(endpointAddr)) == SOCKET_ERROR)
	{
		cout << "Endpoint[" << playerID << "]: Socket bind failed.\n";
		closesocket(endpointSocket);
		system("pause");
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
		system("pause");
		return 0;
	}
	// cout << "Done.\n";

	// now listen successfully
	cout << "\nEndpoint[" << playerID << "]: Endpoint is running at " << port << "\n\n";

	running = true;

	return port;
}

void Endpoint::process()
{
	thread listenThread(&Endpoint::listenFunc, this);
	thread timerThread(&Endpoint::timer, this);
	listenThread.join();
	timerThread.join();
}

void Endpoint::listenFunc()
{
	// while (running){
	// 	// link
	// 	sockaddr_in clientAddr; // client address
	// 	int clientAddrLength = sizeof(clientAddr);
	// 	connSocket = accept(serverSocket, (sockaddr *)&clientAddr, &clientAddrLength);
	// 	if (connSocket == INVALID_SOCKET)
	// 	{
	// 		if (running)
	// 		{
	// 			// if not running, this thread must be terminated by terminateFunc
	// 			// in that case the string below is not needed
	// 		}
	// 		closesocket(connSocket);
	// 		continue;
	// 	}

	// 	// link successfully

	// }
}

void Endpoint::timer()
{
	using namespace std::chrono_literals;

	while (running)
	{
		// sleep for 5 minutes
		this_thread::sleep_for(50s);

		// judge socket state
		// TODO
		running = false;
	}

	closesocket(endpointSocket);
}