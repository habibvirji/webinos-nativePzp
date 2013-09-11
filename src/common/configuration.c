#include <stdlib.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <pwd.h>
#include "logger.h"
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "configuration.h"

char* getDeviceName(){
    char *name;
    struct utsname *buf = (struct utsname*) malloc(sizeof(struct utsname));
    uname(buf);
    name =  buf->nodename;
    LOG("Webinos Device Name: %s", name);
    free(buf);
    return name;
}

char *getHomeDirectory() {
    struct passwd *pw = getpwuid(getuid());
    return pw->pw_dir;
}

char *getCSR(){
    char *read = readFile("pzp_csr");
    return read;
}

char *getFriendlyName() {
    WebinosJSON* config = readConfig("config.json");
    char *friendlyName = NULL;
    while(config){
        if (config->name == "friendlyName") {
            friendlyName = config->str;
            break;
        }
        config = config->next;
    }

    if (!friendlyName) {
        LOG("Please enter device friendlyName: ")
        scanf("%s", &friendlyName)
        (&config, "friendlyName", friendlyName);
        writeConfig("config.json", config);
    }
    deleteJSON(&config);
    return friendlyName;
}

JSON *readConfig(char *fileName){
    JSON *result = NULL;
    char *str = readFile(fileName);
    if(str) {
        parseJSON(&result, str);
    }
    free(str);
    return result;
}

int writeConfig(char *filename, JSON *json){
    int ret = 0;
    char *string = NULL;
    stringifyJSON(&string, json);
    FILE *f = fopen(filename, "w+");
    fwrite(string, sizeof(char), strlen(string), f);
    fclose(f);
    return ret;
}

char *readFile(char *filename){
    int fsize;
    FILE *file = fopen(filename, "r");
    char *string = NULL;
    if (file!=NULL){
        fseek(file, 0, SEEK_END); // Read file end
        fsize = ftell(file); // get the size
        fseek(file, 0, SEEK_SET); // Go back to start
        string = malloc(fsize + 1);
        fread(string, fsize, 1, file);
        fclose(file);
        string[fsize] = 0;
    }
    return string;
}

int checkConfiguration(){
    int ret = 0;
    char *homeDir = getHomeDirectory();
    char *listFile[] = {"webinos", "webinos/keys", "webinos/userData", "webinos/certificates",
    "webinos/certificates/internal", "webinos/certificates/external"};
    struct dirent **namelist;
    int n, i = 0;
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s/.%s", homeDir, listFile[0]);
    LOG("Found Home Directory : %s", buf);
    n = scandir(buf, &namelist, 0, alphasort);
    if (n > 0) {
       while (n--) {
           LOG("Directories/Files inside the webinos directories are : %s", namelist[n]->d_name);
           free(namelist[n]);
       }
       ret = 1;
       free(namelist);
    } else {
        n = strlen(*listFile);
        while(listFile[i] && i != (n-1)){
            memset(buf, 0, sizeof(buf));
            snprintf(buf, sizeof(buf), "%s/.%s", homeDir, listFile[i]);
            LOG("Created Directory: s%s", buf);
            int err = mkdir(buf, 0777);
            if (err != 0) {
                LOG("Error creating directory");
                perror("\nError: ");
                break;
            }
            i++;
        }
    }
    return ret;
}

