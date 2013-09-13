#include "webinos.h"
#include "ssl/certificate.h"

#include <stdlib.h>
#include <unistd.h>

void initializePzp(){
    char buf[1024];
    getcwd(buf, sizeof(buf));
    strcat(buf, "/config.json");
    JSON *config = readConfig(buf);

    char *deviceName = getDeviceName();
    rsa_context rsa;

    addItem(&config,"commonName" , deviceName);
    printJSON(config);
    LOG("Webinos File Configuration Read");

    generatePrivateKey(&rsa);
    LOG("Private Key Generated");

    generateCertificateRequest(config, &rsa);
    LOG("CSR Generated");

    char *ipAddress = calloc(1, 16);
    char *machineName = findPzp(ipAddress);
    LOG("Found PZP - %s (IP Address:%s) to Connect and Enroll", machineName, ipAddress);
    connectPzp(machineName, ipAddress);

    rsa_free(&rsa);
    free(config);
    free(ipAddress);
    free(machineName);
}

int main(int argc, char *argv[]){
    initializePzp();
    /*if (!checkConfiguration()) {
        initializePzp();
    } else {
        connectPzp();
    }*/
    return 0;
}


