#ifndef HELPER_H
#define HELPER_H

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

#define VERSION "1.0.3"

static int serviceEnable = 0;

void trim(char* txt);
void removeChar(char *str, char garbage);
void strtolower(char *str);
int parseMessage(char *payload, int len, int verbose);

void printfDaw(const char *__restrict __format, ...);
int fprintfDaw(FILE *__restrict __stream, const char *__restrict __format, ...);
void setServiceEnable(int val);

#endif // HELPER_H
