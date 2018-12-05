#include "authentication.h"


bool isValidPassword(const QString &str)
{
	if (str.length() > 30 || str.length() < 6)
		return false;
	for (auto c : str){
		if (c != '_' && !(c <= 'z' && c >= 'a') && !(c <= 'Z' && c >= 'A') && !(c >= '0' && c <= '9'))
			return false;
	}
	return true;
}

bool isValidUsername(const QString &str)
{
	if (str.length() > 30 || str.length() < 6){
		return false;
	}
	for (auto c : str){
		if (c == '\t' || c == '\b' || c == '\t'){
			return false;
		}
	}
	return true;
}
