#ifndef CONFIGURATION_H
#define CONFIGURATION_H

WebinosJSON *readConfig(char *filename);
char *readFile(char*);
char* getDeviceName();
int checkConfiguration();
char *getCSR();

#endif
