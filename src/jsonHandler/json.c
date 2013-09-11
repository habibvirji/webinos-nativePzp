#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

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
    if (c == '\"' || c == ' ') {
     continue;
    } else if (c == '}' || c == ']' || c == ',' || c == ':' || c == '\0') {
        i++;
        return i;
    } else {
        i++;
    }
  }
}

char* findValue(char **value){
    char c;
    int i = 0;
    int incr=0;
    char *temp = *value;
        char  *str;
      
    if (temp[0] == '}') return NULL;

    if (temp && (*temp == '{' || *temp == '[')) {
        char *t = temp;
        char comp;
        comp=(*temp=='{') ? '}':']';
        while(*t++!=comp) {
            i++;
        }
        i++;
        str = malloc(i+1);
        memcpy(str, temp, i+1);
        return str;
    }
    str = calloc(1, calculateLength(temp)+1);

    while(temp!=NULL && (c = *temp++)) {
        incr++;
       if (c == '\"' || c == ' ') {
           continue;
       } else if (c == '}' || c == ']' || c == ',' || c == ':' || c == '\0'){
           *value += incr;
           str[i]='\0';
           return str;
       } else {
           str[i++] = c;
       }
    }
}

int checkInteger(char *temp) {
    int c;
    int count = 0;
    int len=0;
    if (*temp == '{' || *temp == '}' || *temp  =='[' || *temp == ']' && temp == "") return 0;
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
    if (*temp == '{' || *temp == '}' || *temp  =='[' || *temp == ']' && temp == "") return 0;
    if(temp) len=strlen(temp);
    while(temp && *temp!='\0') {
        c = *temp;
        if ((c >= 48 && c <= 57) || c== 46) count++;
        temp++;
    }
    return ((count == len) ? 1 : 0);
}

int checkString(char *temp) {
    char c;
    int count = 0;
    int len=0;
    if (*temp == '{' || *temp == '}' || *temp  =='[' || *temp == ']' && temp == "") return 0;
    if(temp) len=strlen(temp);
    while(*temp!='\0' && (c=*temp++)){
        if ((c >= 48 && c <= 57) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122)) count++;
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
            printf("[");
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
    while(tempValue && *tempValue != '}' && str) {
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
        } else if (checkString(tempValue)) {
            temp->str = calloc(1, strlen(tempValue)+1);
            memcpy(temp->str, tempValue, strlen(tempValue)+1);
}

        if (*str == 91) {
            parseJSON(&(temp->object), tempValue);
            char *tv = str;
            while(*tv!=']') tv++;
            str = ++tv;
        } else if(*str == 123) {
            parseJSON(&(temp->object), tempValue);
            char *tv = str;
            while(*tv!='}') tv++;
            str = ++tv;
}


        if (tempValue != NULL) {
            free(tempValue);
            tempValue = NULL;
        }

        if (type != ARRAY && str!="") tempKey   = findValue(&str);
        if (str && str[0] != '}') tempValue = findValue(&str);

        temp->next = NULL;
        if (*json == NULL) {
            *json = temp;
        } else {
            JSON *current = *json;
            while(current->next) current = current->next;
            current->next = temp;
        }
    }
    if (tempValue != NULL) {
        free(tempValue);
        tempValue = NULL;
    }
}


void stringifyJSON(char **str, JSON *json) {
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
                memcpy((*str)+len, json->key, strlen(json->key)+1);
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

       if(temp) {
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
            stringifyJSON(str, json->object);
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

void deleteJSON(JSON *json) {
    JSON *temp;
    while(json) {
        temp = json;
        if(json->key) free(json->key);
        if(json->str) free(json->str);
        if (json->object) deleteJSON(json->object);
        json = json->next;
    }
}
int main() {
    JSON *json = NULL;
    parseJSON(&json, "{firstName:\"z\",lastName:\"y\",integer:123,double:1.12,obj1:{ob1:ob2,ob3:ob4,ob5:ob6}}");
    parseJSON(&json, "{galli:{mota:moti,gadha:gadhi}}");
    parseJSON(&json, "{arr1:[q1,w2,q3]}");
    printJSON(json);
    char *msg = NULL;
    stringifyJSON(&msg, json);
    printf("\n%s\n", msg);
    free(msg);
    deleteJSON(json);
    //,
}