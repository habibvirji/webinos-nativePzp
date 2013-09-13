CC = gcc

LDFLAGS= -L ./src/ssl/polarssl/library/ -l polarssl
CFLAGS = -I ./src/ssl/polarssl/include/ -Wall  -g

all: src/main.c src/jsonHandler/json.c src/ssl/certificate.c src/common/configuration.c src/ssl/connect.c src/localDiscovery/localDiscovery.c src/common/logger.c
	$(CC) $(CFLAGS) src/common/logger.c src/localDiscovery/localDiscovery.c  src/jsonHandler/json.c src/ssl/certificate.c src/common/configuration.c src/ssl/connect.c  src/main.c $(LDFLAGS) -o native_webinos
