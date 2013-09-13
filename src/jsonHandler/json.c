#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "../webinos.h"

#define INT     0
#define DOUBLE  1
#define STRING  2
#define BOOLEAN 3
#define ARRAY   4
#define OBJECT  5

int calculateLength(char *temp) {
  int i = 0;
  char c;
  while(temp != NULL && (c = *temp)) {
    temp++;
    if (c == '\"' || c == ' ' || c == '\n'|| c == '\t') {
     continue;
    } else if (c == '}' || c == ']' || c == ',' || c == ':' || c == '\0') {
        i++;
        return i;
    } else {
        i++;
    }
  }
  return i;
}

char* removeWhiteSpaces(char *str){
    int i = 0;
    while(str[i] != '\0'){
        if (str[i] == ' ' || str[i] == '\n' || str[i] == '\t'   || str[i] == '\t'  || str[i] == ',') {
            i++;
        } else {
            return (str+i);
        }
    }

    return (str+i);
}
char* findValue(char **value){
    char c;
    int i = 0;
    int incr=0;
    char *temp;
    char  *str = NULL;
    *value = removeWhiteSpaces(*value);
    temp = *value;
    if (temp[0] == '}' || temp[0]=='\0') return NULL;
    if (temp && (*temp == '{' || *temp == '[')) {
        char *t = temp;
        char comp;
        comp=(*temp=='{') ? '}':']';
        while(*t++!=comp) {
            i++;
        }
        i++;
        str = calloc(1, i+1);
        memcpy(str, temp, i);
        str[i+1] = '\0';
        return str;
    }
    str = calloc(1, calculateLength(temp)+1);

    while(temp!=NULL && (c = *temp++)) {
        incr++;
       if (c == '\"' || c == ' ' || c == '\n' || c == '\t') {
           continue;
       } else if (c == '}' || c == ']' || c == ',' || c == ':' || c == '\0'){
           *value += incr;
           str[i]='\0';
           return str;
       } else {
           str[i++] = c;
       }
    }
    return str;
}

int checkInteger(char *temp) {
    int c;
    int count = 0;
    int len=0;
    if ((*temp == '{' || *temp == '}' || *temp  =='[' || *temp == ']') && temp) return 0;
    if(temp) len=strlen(temp);
    while(temp && *temp!='\0'){
        c = *temp;
        if (c >= 48 && c <= 57) count++;
        temp++;
    }
    return ((count == len) ? 1 : 0);
}

int checkDouble(char *temp) {
    int c;
    int count = 0;
    int len=0;
    if ((*temp == '{' || *temp == '}' || *temp  =='[' || *temp == ']') && temp) return 0;
    if(temp) len=strlen(temp);
    while(temp && *temp!='\0') {
        c = *temp;
        if ((c >= 48 && c <= 57) || c== 46) count++;
        temp++;
    }
    return ((count == len) ? 1 : 0);
}

int checkAlphaNumeric(char *temp) {
    char c;
    int count = 0;
    int len=0;
    if ((*temp == '{' || *temp == '}' || *temp  =='[' || *temp == ']') && temp) return 0;
    if(temp) len=strlen(temp);
    while(*temp!='\0' && (c=*temp++)){
        if ((c >= 33 && c <= 126)) {
            count++;
        } // Also include \n and \t as csr has it
    }

    return ((count == len) ? 1 : 0);
}

void printJSON(JSON *json){
     while(json != NULL) {
        if (json->key) printf("\n[%s]: ", json->key);
        if (json->integer) printf("%d, ", json->integer);
        else if (json->doub) printf("%.02f, ", json->doub);
        else if (json->str) printf("\"%s\",", json->str);
        else if (json->object && !json->object->key) {
            printf("[\n");
            printJSON(json->object);
            printf("]");
        } else if (json->object && json->object->key) {
            printf("{");
            printJSON(json->object);
            printf("}");
        }
        json = json->next;
    }
     printf("\n");
}

void parseJSON(JSON **json, char *str) {
    char *tempKey=NULL, *tempValue=NULL;
    int type;
    if (*str == 91) {
        str++;
        tempValue = findValue(&str);
        type = ARRAY;
    } else if (*str == 123) {
        str++;
        tempKey   = findValue(&str);
        tempValue = findValue(&str);
        type = OBJECT;
    }
    while(tempValue && *tempValue != '}') {
        JSON *temp = calloc(1, sizeof(JSON));

        if (tempKey != NULL) {
            temp->key = strdup(tempKey);
            free(tempKey);
            tempKey = NULL;
        }
        if (checkInteger(tempValue)) {
            temp->integer = atoi(tempValue);
        } else if (checkDouble(tempValue)) {
            temp->doub = atof(tempValue);
        } else if (checkAlphaNumeric(tempValue)) {
            temp->str = calloc(1, strlen(tempValue)+1);
            memcpy(temp->str, tempValue, strlen(tempValue)+1);
        }

        str = removeWhiteSpaces(str);
        if (*str == 91) {
            parseJSON(&((temp)->object), tempValue);
            char *tv = str;
            while(*tv!=']') tv++;
            str = ++tv;
        } else if(*str == 123) {
            parseJSON(&((temp)->object), tempValue);
            char *tv = str;
            while(*tv!='}') tv++;
            str = ++tv;
        }

        temp->next = NULL;
        if (*json == NULL) {
            *json = temp;
        } else {
            JSON *current = *json;
            while(current->next) current = current->next;
            current->next = temp;
        }
        if (tempValue != NULL) {
            free(tempValue);
            tempValue = NULL;
        }

        if (type != ARRAY && *str != '\0') {
            tempKey   = findValue(&str);
        }
        if (*str != '\0'  && *str != '}'){
            tempValue = findValue(&str);
        }
    }
    if (tempValue != NULL) {
        free(tempValue);
        tempValue = NULL;
    }
}


void stringifyJSON(JSON *json, char **str) {
    int len = 0;
    int type = OBJECT;
    char temp[2048];
    len = (*str == NULL) ? 0: strlen(*str);

    char *g = realloc(*str, len+2);
    if (g != NULL) {
       *str = g;
       if (json->key) memcpy((*str)+len, "{\0", 2);
       else {
            memcpy((*str)+len, "[\0", 2);
            type = ARRAY;
       }
    }
    while(json) {
        memset(temp, 0x00, 2048);
        if (json->key) {
            char *v;
            len = ((*str == NULL) ? 0: strlen(*str));
            v = realloc(*str, len+ strlen(json->key)+3);
            if (v!=NULL) {
                *str = v;
                memcpy((*str)+len, "\"", 1);
                memcpy((*str)+len+1, json->key, strlen(json->key));
                memcpy((*str)+len+1+strlen(json->key), "\"\0", 2);
            }
            if (json->str || json->integer || json->doub) {
                len = ((*str == NULL) ? 0: strlen(*str));
                *str = realloc(*str, len+2);
                memcpy((*str)+len, ":\0", 2);
            }
        }

       if (json->integer){
            sprintf(temp,"%d", json->integer);
       } else if (json->doub){
            sprintf(temp,"%f", json->doub);
       } else if (json->str){
            sprintf(temp,"\"%s\"", json->str);
       }
       if (json->next!=NULL && json->object==NULL)
            sprintf(temp, "%s,", temp);

       if(temp!=NULL) {
            char *p;
            len = (*str == NULL) ? 0: strlen(*str);
            p = realloc(*str, len + strlen(temp)+1);
            if (p) {
                *str = p;
                memcpy((*str)+len, temp, strlen(temp)+1);
            }
       }

        if (json->object){
            *str = realloc(*str, strlen(*str) + 2);
            if (json && json->object){
                memcpy((*str)+strlen(*str), ":\0", 2);
            }
            stringifyJSON(json->object, str);
            *str = realloc(*str, strlen(*str) + 2);
            if (json && json->next) {
                memcpy((*str)+strlen(*str), ",\0", 2);
            }
        }
        json = json->next;
    }
    len = (*str == NULL) ? 0: strlen(*str);
    char *n = realloc(*str, len+2);
    if (n != NULL) {
       *str = n;
       if (type!= ARRAY){
            memcpy((*str)+len, "}\0", 2);
       } else {
            memcpy((*str)+len, "]\0", 2);
       }

    }
}

void addItem(JSON **json, char *key, char *name) {
    char *buf = calloc(1, strlen(key)+strlen(name)+4);
    sprintf(buf,"{%s:%s}", key , name);
    parseJSON(json, buf );
    free(buf);
}

void createJSONString(char **buf, char *key, char *name){
    int len = (*buf != NULL)? strlen(*buf):0;
    int keyLen = (key != NULL)? strlen(key):0;
    int nameLen = (name != NULL)? strlen(name):0;
    char *temp= realloc(*buf, keyLen+nameLen+len+4);
    if (temp) {
        if (*buf && *(temp+len-1) == '{') len-=1;
        if (key) sprintf(temp+len, "%s", key);
        if (name) sprintf(temp+len+keyLen, ":%s,", name);
        *buf = temp;
    }
    LOG(*buf);
}

void deleteJSON(JSON *json) {
    JSON *temp;
    while(json) {
        temp = json;
        if(json->key) free(json->key);
        if(json->str) free(json->str);
        if (json->object) deleteJSON(json->object);
        free(json);
        json = temp->next;
    }
}
/*int main() {
    JSON *json = NULL;
    parseJSON(&json, "{\n\"firstName\":\"z\",\n\"lastName\":\"y\",\n\"integer\":123,\n\"double\":1.12,\n\"obj1\":{\n\"ob1:\"ob2\", \n\"ob3\":\"ob4\", \n\"ob5\":\"ob6\"}}");
    parseJSON(&json, "{galli:{mota:moti,gadha:gadhi}}");
    parseJSON(&json, "{arr1:[q1,w2,q3]}");
    printJSON(json);

    char *msg = NULL;
    stringifyJSON(json, &msg);
    printf("\n%s\n", msg);
    free(msg);
    deleteJSON(json);
    //,
} */
