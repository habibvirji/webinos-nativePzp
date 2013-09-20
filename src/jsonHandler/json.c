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
        if (c == '}' || c == ']' || c == ',' || c == ':' || c == '\0') {
            i++;
            return i;
        } else {
            i++;
        }
    }
    return i;
}

void removeStartingWhiteSpaces(char **str){
    int i = 0;
    if (*str) {
		while(**(str+i) != '\0'){
			if (**(str+i) == ' ' ||  **(str+i) == '\"' || **(str+i) == '\n' || **(str+i) == '\t'  || **(str+i) == ',') {
				i++;
			} else {
				*str = (*str) + i;
	  
				break;
			}
		}
	}
}

void findKey(char **value, char **key){
    char c;
    int  i=0,incr=0;
    char *temp,*str;

    removeStartingWhiteSpaces(value);
    temp = *value;
   
    str = calloc(1, calculateLength(temp)+1);
    
    while(temp!=NULL && (c = *temp++)) {
       incr++;

       if (c=='\"' || c == ' ' || c == ',' || c == '\n' || c == '\t' || c == '{' || c == '[') {
           continue;
       } else if (c == ':' || c == '\0'){
           *value += incr;
           str[i]='\0';
		   if (i > 0 && strlen(str) > 1) *key = strdup(str);
           if (str) free(str);
           break;
       } else {
           str[i++] = c;
       }
    }
}

void findValue(char **value, JSON **json){
    char c;
    int  i=0,incr=0;
    char *temp,*str;
    removeStartingWhiteSpaces(value);
    temp = *value;
    if (temp && (temp[0] == '{' || temp[0] == '[')) {
		char comp=(*temp=='{') ? '}':']';
		while(*temp++!=comp) i++;		
		if (i > 1) {
			str  = calloc(1, i+3); // 1 for ending json marker and 1 for end char
			memcpy(str, *value, i+1);
			str[i+2] = '\0';
			parseJSON(&((*json)->object), str);
			*value = temp;
			if (str) free(str);
			return;
		}
	}
	if (temp != NULL) { 
		str = calloc(1, calculateLength(temp)+1);   
		while(temp!=NULL && (c = *temp++)) {
		   incr++;
		   if (c == '\n' ) {
			   sprintf(str+i, "\\n");
			   i+=2;
		   } else if (c == '\t' ) {
			   sprintf(str+i, "\\t");
			   i+=2;
		   } else if (c == '\"' ) {
			   continue;
		   } else if (c == '}' || c == ']' || c == ',' || c == '\0'){
               str[i]='\0';
			   if (i > 0 && strlen(str) > 1) (*json)->str = strdup(str);
               *value += incr;
               if (str) free(str);
			   break;
		   } else {
			   str[i++] = c;
		   }
		}
	}
}

void printJSON(JSON *json){
     while(json != NULL) {
        if (json->key) printf("\n[%s]: ", json->key);
        if (json->str) printf("%s,", json->str);
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
    while(str && *str!='\0') {
        JSON *temp = calloc(1, sizeof(JSON));	
        removeStartingWhiteSpaces(&str);   
        if (*str == 91) {
			findValue(&str, &temp);
		} else {
			findKey(&str, &(temp->key));
			findValue(&str, &temp);			
		}
        if (temp->key!= NULL || temp->str != NULL) {
			temp->next = NULL;
			if (*json == NULL) {
				*json = temp;
			} else {
				JSON *current = *json;
				while(current->next) current = current->next;
				current->next = temp;
			}
		}
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
            if (json->str) {
                len = ((*str == NULL) ? 0: strlen(*str));
                *str = realloc(*str, len+2);
                memcpy((*str)+len, ":\0", 2);
            }
        }

       if (json->str){
            sprintf(temp,"\"%s\"", json->str);
            printf("%s", json->str);
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

int checkMessageType(JSON *json, char *key) {
	while(json) {
		if(json->str) LOG(json->str);
		if (json->str && strcmp(json->str, key) == 0) {
			return 1;//true
		}
		if (json->object) {
			int i = checkMessageType(json->object,key);
			if (i == 1) return 1; 
		}
		json = json->next;
	}
	return 0;//false
}
/*int main() {
    JSON *json = NULL;
    parseJSON(&json, "{\n\"firstName\":\"z\",\n\"lastName\":\"y\",\n\"integer\":123,\n\"double\":1.12,\n\"obj1\":{\n\"ob1:\"ob2\", \n\"ob3\":\"ob4\", \n\"ob5\":\"ob6\"}}");
    parseJSON(&json, "{galli:{mota:moti,gadha:gadhi}}");
    //parseJSON(&json, "{arr1:[q1,w2,q3]}");
    printJSON(json);

    char *msg = NULL;
    stringifyJSON(json, &msg);
    printf("\n%s\n", msg);
    free(msg);
    deleteJSON(json);
   
}*/ 
