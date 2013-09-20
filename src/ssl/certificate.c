#include "polarssl/x509.h"
#include "polarssl/x509write.h"
#include "polarssl/base64.h"
#include "polarssl/entropy.h"
#include "../webinos.h"
#include "certificate.h"

#include <stdlib.h>
#include <stdio.h>

void generatePrivateKey(rsa_context *rsa){
    entropy_context entropy;
    ctr_drbg_context ctr_drbg;

    const char *pers = "rsa_genkey";
    entropy_init( &entropy );
    ctr_drbg_init( &ctr_drbg, entropy_func, &entropy,(const unsigned char *) pers, strlen(pers));

    // Generate the private key
    rsa_init(rsa, RSA_PKCS_V15, 0);
    rsa_gen_key(rsa, ctr_drbg_random, &ctr_drbg, 1024, 65537);
}

void generateCertificateRequest(JSON *json, rsa_context *rsa){
    char *oid = NULL;
    x509_req_name *req_name = NULL;
    x509_req_name *cur = req_name;
    FILE *f;
    unsigned char output_buf[4096];
    unsigned char base_buf[4096];
    unsigned char *c;
    size_t len = 0, olen = 4096;

    while(json != NULL) {
      if (strcmp(json->key, "commonName")==0)
        oid = OID_CN;
      else if(strcmp(json->key, "orgunit")==0)
        oid = OID_ORG_UNIT;
      else if(strcmp(json->key, "state")==0)
        oid = OID_STATE;
      else if (strcmp(json->key, "country")==0)
        oid = OID_COUNTRY;
      else if (strcmp(json->key, "orgname")==0)
        oid = OID_ORGANIZATION;
      else if (strcmp(json->key, "city")==0)
        oid = OID_LOCALITY;
      else if (strcmp(json->key, "email")==0)
        oid = OID_PKCS9_EMAIL;

      if (oid && json->str) {
        if (cur == NULL) {
            req_name = malloc(sizeof(x509_req_name));
            cur = req_name;
        } else {
            cur->next = malloc(sizeof(x509_req_name));
            cur = cur->next;
        }
        strncpy(cur->oid, oid, strlen(oid));
        cur->oid[strlen(oid)] = '\0';
        strncpy(cur->name, json->str, strlen(json->str));
        cur->name[strlen(json->str)] = '\0';
      }
      oid = NULL;
      json = json->next;
    }

    memset(output_buf, 0, 4096);
    len = x509_write_cert_req( output_buf, 4096, rsa, req_name, SIG_RSA_SHA1 );
    c = output_buf + 4095 - len;
    base64_encode( base_buf, &olen, c, len );
    c = base_buf;

    f = fopen("pzp_csr", "w" );
    fprintf(f, "-----BEGIN CERTIFICATE REQUEST-----\n");
    while (olen) {
        int use_len = olen;
        if (use_len > 64) use_len = 64;
        fwrite( c, 1, use_len, f );
        olen -= use_len;
        c += use_len;
        fprintf(f, "\n");
    }
    fprintf(f, "-----END CERTIFICATE REQUEST-----");
    fclose(f);
}
