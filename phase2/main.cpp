#include "server.h"

using namespace std;

int main()
{
	Hub &hub = Hub::getInstance();
	hub.start();
	system("pause");
}