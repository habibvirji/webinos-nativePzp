#ifndef CERTIFICATE_H
#define CERTIFICATE_H

#include "polarssl/rsa.h"
#include "polarssl/ctr_drbg.h"

/**
 * Generates PZP private key
 * @params rsa - gets rsa key and returns back
**/
void generatePrivateKey(rsa_context *rsa);

/**
 * Generates certificate request for the PZP.
 * @params json - configuration of the certificate
 * @params rsa  - private key generated from above step
**/
void generateCertificateRequest(JSON* json, rsa_context *rsa);

#endif