#include "regexp.h"

/* Replace all the occurences of string Current with string Replacement in the string Str */
char *strReplace(char *Str, char *Current, char *Replacement)
{
	static char Buff[4096];
	char *p;

	if(!(p = strstr(Str, Current)))
		return Str;

	strncpy(Buff, Str, p-Str);
	Buff[p-Str] = '\0';

	sprintf(Buff+(p-Str), "%s%s", Replacement, p+strlen(Current));

	return Buff;
}

