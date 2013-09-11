#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int setMulticastConnection(){
    int s, flag = 1, ittl = 255;
    struct sockaddr_in in;
    struct ip_mreq mc;
    char ttl = 255;

    bzero(&in, sizeof(in));
    in.sin_family = AF_INET;
    in.sin_port = htons(5353);
    in.sin_addr.s_addr = 0;

    if((s = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
        return 0;
    }
 
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag));
    if(bind(s,(struct sockaddr*)&in,sizeof(in))) {
        close(s);
        return 0;
    }

    mc.imr_multiaddr.s_addr = inet_addr("224.0.0.251");
    mc.imr_interface.s_addr = htonl(INADDR_ANY);

    setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mc, sizeof(mc));
    setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
    setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ittl, sizeof(ittl));

    flag =  fcntl(s, F_GETFL, 0);
    flag |= O_NONBLOCK;
    fcntl(s, F_SETFL, flag);

    return s;
}

struct {
    unsigned short int id;
    unsigned short int flags;
    unsigned short int qcount;
    unsigned short int ancount;
    unsigned short int nscount;
    unsigned short int  rcount;
} header;

typedef struct _msg{

    char *serviceName;
    unsigned short int  qtype;
    unsigned short int  qclass;
}mDNSMsg;

void findPzp(){
    int socketDesc = setMulticastConnection();
    unsigned char buf[9000];
    unsigned char outMsg[1000];
    char *tmp;
    char servType[] = "_pzp._tcp.local";
    struct sockaddr_in to, dest;
    int size = sizeof(struct sockaddr);
    fd_set fds;
    int msgSize =0;
    bzero(&to, sizeof(to));
    bzero(&dest, sizeof(dest));
    to.sin_family = AF_INET;
    to.sin_port = htons(4321);
    to.sin_addr.s_addr = inet_addr("224.0.0.251");

    unsigned short int yes  = htons(1)  ;
    unsigned short int no = 0;
    int i = 0, j =0, psize =0, pos=0;

    memcpy(outMsg,          &no, 2);
    msgSize+=2;
    memcpy(outMsg+msgSize,  &no, 2);
    msgSize+=2;
    memcpy(outMsg+msgSize,  &yes, 2);
    msgSize+=2;
    memcpy(outMsg+msgSize,  &no, 2);
    msgSize+=2;
    memcpy(outMsg+msgSize,  &no, 2);
    msgSize+=2;
    memcpy(outMsg+msgSize, &no, 2);
    msgSize+=2;
    tmp = calloc(1, strlen(servType));

    while(servType[j]){
        if (servType[j] == '.') {
            memcpy(outMsg+msgSize, &i, 1);// length of the string;
            msgSize+=1;
            memcpy(outMsg+msgSize, tmp, i);// length of the string;
            msgSize+=i;
            i =0;
         } else {
            tmp[i++] = servType[j];
        }
        (j++);
    }
    if (tmp) {
        memcpy(outMsg+msgSize, &i, 1);// length of the string;
        msgSize+=1;
        memcpy(outMsg+msgSize, tmp, i+1);// length of the string;
        msgSize+=i+1;
    }
    strncpy(tmp, outMsg+12, msgSize -12);

    yes = htons(1);
    memcpy(outMsg+msgSize, &yes, 2);
    msgSize+=2;

    yes = htons(1);
    memcpy(outMsg+msgSize, &yes, 2);
    msgSize+=2;

    sendto(socketDesc, outMsg, msgSize , 0, (struct sockaddr*)&to, sizeof(to));
    struct timeval t;
    t.tv_sec = 10; // Every 10 sec look for a PZP
    t.tv_usec = 0;

    while(1) {
        FD_ZERO(&fds);
        FD_SET(socketDesc,&fds);
        select(socketDesc+1,&fds,0,0, &t);

        if(FD_ISSET(socketDesc,&fds)) {
            if ((psize = recvfrom(socketDesc, buf, sizeof(buf), 0, (struct sockaddr*)&dest, &size)) > 0 ) {
                i = 0;
                /*printf("\nPacket Size: (%d) Packet Data:",psize);
                while (i != psize){
                    printf("  0x%x", buf[i++]);
                }             */
                // byte 3 contains flags that gives indication if it is a response
                if (buf[2] == 132) {///0x84 that means response
                    pos = 12;

                    while(buf[pos] != 0) {
                        pos += 1 + buf[pos]; // 1 for length
                    }

                    char *deb = strndup(buf+12, pos-12);
                    printf("\n debug : %s %s", deb, tmp);
                    free(deb);
                    pos += 1; // end character

                    if (strncmp(tmp, buf+12, pos-12) == 0){ // Check string that we sent matches
                        // I just need IP address rest of the information does not matter
                        while(pos < psize){
                            if(buf[pos + 1] == 1 ) { // Looking for Only IPV4 address
                                pos += 2 + 2 + 4;   //  Type (2)+ ClassCode (2)+ TTL (4)
                                if (pos+1 == 4) {
                                    pos += 2;
                                    printf("\n IP ADDRESS FOUND: %d.%d.%d.%d", buf[pos+1], buf[pos+2], buf[pos+3],
                                    buf[pos+4]);
                                }
                            } else {
                                pos += 2 + 2 + 4;   // Type (2)+ ClassCode (2)+ TTL (4)
                                pos += buf[pos+1];  // Length
                                if(pos < psize) pos+=4;// Offset
                            }
                        }
                    }
                }
            }
        }
    }

}

int main() {
    findPzp();
}