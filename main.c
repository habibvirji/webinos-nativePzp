#include "jsonHandler/json.h"
#include "certificate.h"
#include "configuration.h"
#include "localDiscovery.h"
#include "connect.h"
#include "logger.h"

#include <string.h>
#include <stdlib.h>

#include "polarssl/rsa.h"

void foundPzpAddress(char *address){
    LOG("Connect to PZP to Enroll : %s", address);
    connectPzp(address);
}

void initializePzp(){
    WebinosJSON *config = readConfig("config.json");
    char *deviceName = getDeviceName();
    rsa_context rsa;

    addItem(&config,"commonName" , deviceName);
    LOG("Webinos File Configuration Read");

    generatePrivateKey(&rsa);
    LOG("Private Key Generated");

    generateCertificateRequest(config, &rsa);
    LOG("CSR Generated");

    findPzp(foundPzpAddress);
    LOG("Found PZP to Connect and Enroll");

    rsa_free(&rsa);
    free(config);
}
int main(int argc, char *argv[]){
    initializePzp();
    /*if (!checkConfiguration()) {

    } else {
        connectPzp();
    }*/
    return 0;
}


