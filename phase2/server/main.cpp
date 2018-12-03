#include "server.h"
#include <cstdlib>
#include <ctime>

using namespace std;

int main()
{
	srand(time(NULL));
	Hub &hub = Hub::getInstance();
	hub.start();
	system("pause");
}