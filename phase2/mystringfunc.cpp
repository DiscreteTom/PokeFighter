#include "mystringfunc.h"

using namespace std;

vector<string> split(const string &str, char ch)
{
	vector<string> result;
	string t;
	for (auto c : str)
	{
		if (c != ch)
		{
			t += c;
		}
		else
		{
			result.push_back(t);
			t = "";
		}
	}
	if (str.length())
		result.push_back(t);
	return result;
}