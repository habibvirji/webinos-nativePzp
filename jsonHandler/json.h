#ifndef JSONHANDLER_JSON_H
#define JSONHANDLER_JSON_H

typedef struct json {
    int    type; // Type of the object
    char   *key;
    int    num;
    double doub;
    char   *str;
    struct json *obj;
    struct json *next;
}WebinosJSON;

typedef struct hash {
    int size;
    WebinosJSON **json;
}WebinosHash;

void addItem(WebinosHash **obj, char *name, char *value);
void addArray(WebinosJSON** obj, char *value);
void addObject(WebinosJSON **obj, char *value);

WebinosHash* createHash(int size);
WebinosHash *parseJSON(char *str);
char *stringifyJSON(WebinosHash *json, char *spacer);

void deleteObject(WebinosHash **obj);
void printObject (WebinosHash  *obj);

#endif