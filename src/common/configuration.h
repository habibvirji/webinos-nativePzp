#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "jsonHandler/json.h"

JSON *readConfig(char *filename);
char *readFile(char*);
char* getDeviceName();
int checkConfiguration();
char *getCSR();

#endif
