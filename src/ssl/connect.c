#include "polarssl/net.h"
#include "polarssl/ssl.h"
#include "polarssl/entropy.h"
#include <stdlib.h>
#include "../webinos.h"
#include "certificate.h"
#define BUF_SIZE 65536


void my_debug( void *ctx, int level, const char *str ) {
    ((void)level);
    fprintf( (FILE *) ctx, "%s", str );
    fflush(  (FILE *) ctx  );
}

void connectPzp(char *connMachineName, char *address){
    ssl_context ssl;
    int server_fd, len, ret;
    entropy_context entropy;
    ctr_drbg_context ctr_drbg;
    char buf[BUF_SIZE];

    const char *pers = "connectPzp";
    entropy_init( &entropy );
    ctr_drbg_init( &ctr_drbg, entropy_func, &entropy,(const unsigned char *) pers, strlen(pers));

    net_connect( &server_fd, address, 8040 );
    ssl_init(&ssl);
    ssl_set_endpoint( &ssl, SSL_IS_CLIENT );
    ssl_set_authmode( &ssl, SSL_VERIFY_NONE );
    ssl_set_rng( &ssl, ctr_drbg_random, &ctr_drbg );
    ssl_set_dbg( &ssl, my_debug, stdout );
    ssl_set_bio( &ssl, net_recv, &server_fd, net_send, &server_fd );

    /*while( ( ret = ssl_handshake( &ssl ) ) != 0 ) {
        if( ret != POLARSSL_ERR_NET_WANT_READ && ret != POLARSSL_ERR_NET_WANT_WRITE )
        {
            printf( " failed\n  ! ssl_handshake returned -0x%x\n\n", -ret );
            goto exit;
        }
    }*/

    LOG(connMachineName);
    char *sendBuf = NULL;
    JSON *json = NULL;
    createJSONString(&sendBuf, "{type", "prop");
    createJSONString(&sendBuf, "from",  getDeviceName());
    createJSONString(&sendBuf, "to",    connMachineName);
    createJSONString(&sendBuf, "payload", "{");
    createJSONString(&sendBuf, "type", "microPzpEnrollment");
    createJSONString(&sendBuf, "message", "{");
    createJSONString(&sendBuf, "csr", getCSR());
    createJSONString(&sendBuf, "friendlyName", getFriendlyName());
    createJSONString(&sendBuf, "}}}", NULL);
    LOG("%s", sendBuf);
    parseJSON(&json, sendBuf);
    printJSON(json);
    free(sendBuf);
    sendBuf=NULL;
    stringifyJSON(json, &sendBuf);
    deleteJSON(json);
    LOG( " %d bytes written\n\n%s", strlen(sendBuf),sendBuf );

    ssl_write( &ssl, (const unsigned char*)sendBuf, strlen(sendBuf));
    do {
        len = sizeof( buf ) - 1;
        memset( buf, 0, sizeof( buf ) );
        ret = ssl_read( &ssl, (unsigned char*)buf, len );

        if( ret == POLARSSL_ERR_NET_WANT_READ || ret == POLARSSL_ERR_NET_WANT_WRITE )
            continue;

        if( ret == POLARSSL_ERR_SSL_PEER_CLOSE_NOTIFY )
            break;

        if( ret < 0 ){
            printf( "failed\n  ! ssl_read returned %d\n\n", ret );
            break;
        }

        if( ret == 0 ){
            printf( "\n\nEOF\n\n" );
            break;
        }

        len = ret;
        printf( " %d bytes read\n\n%s", len, (char *) buf );
    } while( 1 );
    ssl_close_notify( &ssl );
    //exit:
       net_close( server_fd );
       ssl_free( &ssl );
}

