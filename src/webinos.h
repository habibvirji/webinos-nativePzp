#ifndef WEBINOS_H
#define WEBINOS_H

void logPrint(char *file, int line, char *fmt, ...);
#define LOG(...) (logPrint(__FILE__, __LINE__, __VA_ARGS__))

typedef struct _JSON{
    char         *key;
    int          integer;
    double       doub;
    char         *str;
    struct _JSON *object;
    int          type;
    struct _JSON *next;
}JSON;

/**
 * Parse JSON and sets str value in json structure
**/
void parseJSON(JSON **json, char *str);

/**
 * Add item (key, name) in JSON structure
**/
void addItem(JSON **json, char *key, char *str);

/**
 * Parse JSON and sets str value in json structure
**/
void stringifyJSON(JSON *json, char **str);

/**
 * Parse JSON and sets str value in json structure
**/
void printJSON (JSON  *obj);

/**
 * Parse JSON and sets str value in json structure
**/
void deleteJSON (JSON  *obj);

void createJSONString(char **buf, char *key, char *name);

/**
 * Reads configuration from the file
**/
JSON *readConfig(char *filename);

/**
 * Write configuration into the file
**/
int writeConfig(char *filename, JSON *json);

/**
 * Gets machine name of the PZP
**/
char* getDeviceName();

/**
 * Check whether configuration exists or does it need enrolment
**/
int checkConfiguration();

char *getCSR();
char *getFriendlyName();
/**
 * Find Peer PZP. Needed for Enrolment.
 * @params ipAddress - set IP address where service is found
 * @returns machine name. Caller needs to free the machine
**/
char *findPzp(char *ipAddress);

/**
 * connectPzp uses IP address from above
 * params address - Address is retrieved from this step.
**/
void connectPzp(char *machineName, char *ipAddress);

#endif