#ifndef CERTIFICATE_H
#define CERTIFICATE_H

#include "polarssl/rsa.h"
#include "polarssl/ctr_drbg.h"

void generatePrivateKey(rsa_context *rsa);
void generateCertificateRequest(WebinosJSON* json, rsa_context *rsa);

#endif