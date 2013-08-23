CC = gcc

LDFLAGS= -L ./polarssl/library/ -l polarssl  -l avahi-client -l avahi-common
CFLAGS = -I ./polarssl/include/ -g  -Wall

all: main.c jsonHandler/json.c certificate.c configuration.c connect.c localDiscovery.c logger.c
	 $(CC) $(CFLAGS) logger.c localDiscovery.c  jsonHandler/json.c certificate.c configuration.c connect.c  main.c $(LDFLAGS) -o native_webinos



