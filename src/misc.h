#ifndef __MISC__
#define __MISC__

char *dupcat(const char *s1, ...);
char *dupncat(const char *s1, unsigned int n);
char** str_split(char* a_str, const char a_delim);

#endif