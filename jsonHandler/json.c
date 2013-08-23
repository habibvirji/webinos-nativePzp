#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "json.h"
#include <stdio.h>

#define INT     0
#define DOUBLE  1
#define STRING  2
#define BOOLEAN 3
#define ARRAY   4
#define OBJECT  5

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static unsigned int hashPrime;

// http://pokristensson.com/code/strmap/strmap.c

typedef struct _JSONValue{
        int    integer;
        double doub;
        char  *str;
        struct _JSON *object;
}JSONValue;

typedef struct _JSON{
        char         *key;
        JSONValue    *value;
        int          count;
        int          type;
        struct _JSON *next;
}JSON;


char* findValue(char *value){
    char c;
    char str[512];
    memset(str, 0, sizeof(str));
    while(c = *value++) {
       if (c == '{' || c == '[' || c == '\"' || c == ' ') continue;
       else if (c == '}' || c == ']' || c == ',"' || c == ':') return str;
       *str++ = c;
    }
}

int checkInteger(char *temp) {
    int c;
    int len = strlen(temp);
    while(c=*temp++ && (c >= 48 && c <= 57));
    return ((c == temp[len]) ? 1 : 0);
}

int checkDouble(char *temp) {
    int c;
    int len = strlen(temp);
    while(c=*temp++ && ((c >= 48 && c <= 57) || c== 46));
    return ((c == temp[len]) ? 1 : 0);
}

int checkSting(char *temp) {
    char c;
    int len = strlen(temp);
    while(c=*temp++ && ((c >= 48 && c <= 57) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122)));
    return ((c == temp[len]) ? 1 : 0);
}

void addValue(JSONValue **value, char *str) {
    char *temp = findValue(str);
    if (temp == NULL) addValue(value, str++);
    if (checkInteger(temp)) value->integer = atoi(temp);
    else if (checkDouble(temp)) value->doub = atof(temp);
    else if (checkString(temp)) value->str = strdup(temp);
    else if (*temp == 91) addValue((*value)->value, *temp+1);
    else if (*temp == 123) addValue((*value)->value, *temp+1);
}

JSON *addItem(char *key, char *value) {
    char *tempKey   = strdup(key);
    char *tempValue = strdup(value);
    removeWhiteSpace(tempKey);
    removeWhiteSpace(tempValue);
    JSON *json = calloc(1, sizeof(JSON));
    if (json){
        json->type = (*tempValue == "{") ? OBJECT: (*tempValue == "[")  ? ARRAY: -1;
        if (json->type == OBJECT) {
            char *temp = findValue(*tempValue);
            json->key = strdup(temp);
        }
        temp = findValue(*tempValue);
        addValue(json->value, temp);
    }
    exit:
        free(json);
        free(tempKey);
        free(tempValue)
}

char* parseJSON(JSON *parse) {
      addItem()
}


int main() {
    JSON *json = calloc(1, sizeof(JSON));

    JSON *parse = addItem("{firstName: z, lastName: y}");
    printJSON(parse);
    temp = stringifyJSON(parse);
    printf("Stringified: %s", temp);
}