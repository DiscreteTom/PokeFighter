#include "server.h"
#include "mystringfunc.h"
#include <conio.h>
#include <thread>

Server::Server()
{
}

Server::~Server()
{
	closesocket(serverSocket); // if serverSocket has been closed, return WSAENOTSOCK
	closesocket(connSocket);
	//stop socket DLL
	while (WSACleanup() != WSANOTINITIALISED)
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
	cout << "Server: Socket binding...\n";
	if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "Server: Socket bind failed.\n";
		closesocket(serverSocket);
		WSACleanup();
		system("pause");
		return;
	}
	cout << "Done.\n";

	// if request queue is full, client will get error: WSAECONNREFUSED
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
	cout << "Server: Server is running at " << SERVER_PORT << endl;

	// init thread
	running = true;
	thread listenThread(listenFunc, this);
	thread terminateThread(terminateFunc, this);
	listenThread.join();
	terminateThread.join();

	closesocket(serverSocket);
	WSACleanup();

	sqlite3_close(db);
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
			cout << "Server: Link to client failed.\n";
			closesocket(connSock);
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
		char buf[BUF_LENGTH] = "";
		recv(connSocket, buf, BUF_LENGTH, 0);
		auto strs = split(buf, '\n');
		if (strs.size() < 3)
		{
			//error
			cout << "Server: Invalid request.\n";
			strcpy_s(buf, "Reject: Invalid request.\n");
			send(connSocket, buf, BUF_LENGTH, 0);
		}
		else if (strs[0] == "login")
			login(strs[1], strs[2]);
		else if (strs[0] == "logon")
			logon(strs[1], strs[2]);
		else
		{
			cout << "Server: Invalid request.\n";
			strcpy_s(buf, "Reject: Invalid request.\n");
			send(connSocket, buf, BUF_LENGTH, 0);
		}
		closesocket(connSocket);
	}
}

void Server::terminateFunc()
{
	getch();
	running = false;
}

void Server::login(const string &username, const string &password)
{
}
void Server::logon(const string &username, const string &password)
{
	if (!isValid(username))
	{
		cout << "Server: Got an invalid username: " << username << endl;
		strcpy_s(buf, "Reject: Invalid username.\n");
	}
	else if (!isValid(password))
	{
		cout << "Server: Got an invalid password";
		strcpy_s(buf, "Reject: Invalid password.\n");
	}
	else
	{
		// TODO about database
	}
	send(connSocket, buf, BUF_LENGTH, 0);
}