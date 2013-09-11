#ifndef JSONHANDLER_JSON_H
#define JSONHANDLER_JSON_H

typedef struct _JSON{
    char   *key;
   int          integer;
    double doub;
    char   *str;
   struct _JSON *object;
   int          type;
   struct _JSON *next;
}JSON;

void parseJSON(JSON **json, char *str);
void stringifyJSON( char **str, JSON *json);
void printObject (JSONs  *obj);

#endif