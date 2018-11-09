#include "server.h"

using namespace std;

int main()
{
	Server &server = Server::getInstance();
	server.start();
	system("pause");
}