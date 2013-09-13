#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // for inet_addr
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../webinos.h"

// Standard multicast connection. Listening at address 224.0.0.251 at post 5353
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


char* findPzp(char *ret){
    int socketDesc = setMulticastConnection();
    unsigned char buf[9000];
    unsigned char outMsg[1000];
    char *tmp;
    char servType[] = "_pzp._tcp.local";
    struct sockaddr_in to, dest;
    socklen_t size = sizeof(struct sockaddr_in);
    fd_set fds;
    unsigned int msgSize =0;
    struct timeval t;
    char *machineName = NULL;

    bzero(&to, sizeof(to));
    bzero(&dest, sizeof(dest));
    to.sin_family = AF_INET;
    to.sin_port = htons(5353);
    to.sin_addr.s_addr = inet_addr("224.0.0.251");

    t.tv_sec = 30; // Every 30 sec look for a PZP
    t.tv_usec = 0;

    unsigned short int yes;
    unsigned short int no = 0;
    int i = 0, j =0, psize =0, pos=0;
    /*
    HEADER STRUCTURE
    \ID|FLAG|QCOUNT|ANCOUNT|NSCOUNT|ARCOUNT|
      2  2      2      2       2       2
    */
    memcpy(outMsg,          &no, 2);
    msgSize+=2;
    memcpy(outMsg+msgSize,  &no, 2);
    msgSize+=2;
    yes = htons(1);
    memcpy(outMsg+msgSize,  &yes,2);
    msgSize+=2;
    memcpy(outMsg+msgSize,  &no, 2);
    msgSize+=2;
    memcpy(outMsg+msgSize,  &no, 2);
    msgSize+=2;
    memcpy(outMsg+msgSize,  &no, 2);
    msgSize+=2;

    // Create tmpName of form Len|Str \004_pzp\004_tcp\005local0x00
    tmp = calloc(1, strlen(servType) + 10);// Arbitrary 10 to handle 10 servtype length fields
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
        j++;
    }
    // To add local at the end and append 0x00
    if (tmp) {
        memcpy(outMsg+msgSize, &i, 1);// length of the string;
        msgSize+=1;
        memcpy(outMsg+msgSize, tmp, i+1);// length of the string;
        msgSize+=i+1;
    }
    strncpy(tmp, (char*)outMsg+12, msgSize -12); // store this tmp for future purpose

    /*
    At end of query add: QTYPE|QCLASS
                           2     2
    */
    yes = htons(1);
    memcpy(outMsg+msgSize, &yes, 2);
    msgSize+=2;

    yes = htons(1);
    memcpy(outMsg+msgSize, &yes, 2);
    msgSize+=2;

    // Send Query Request
    sendto(socketDesc, outMsg, msgSize , 0, (struct sockaddr*)&to, sizeof(to));
    // TODO: Send message again on timeout

    while(1) {
        FD_ZERO(&fds);
        FD_SET(socketDesc,&fds);
        select(socketDesc+1,&fds,0,0, &t); // Set time interval to listen on this descriptor

        if(FD_ISSET(socketDesc,&fds)) {
            if ((psize = recvfrom(socketDesc, buf, sizeof(buf), 0, (struct sockaddr*)&dest, &size)) > 0 ) {
                // print out the whole packets
                i = 0;
                printf("\nPacket Size: (%d)\n",psize);
                while (i != psize){
                    printf("  0x%2x", buf[i++]);
                    if (i%20 == 0) printf("\n");
                }
                // byte 3 contains flags that gives indication if it is a response
                if (buf[2] == 132) {///0x84 that means response
                    pos = 12; // By pass header..

                    // Loop to calculate length of response service type.
                    // Format is usually Len|Str \008_machine\004_pzp\004_tcp\005local0x00
                    while(buf[pos] != 0) {
                        pos += 1 + buf[pos]; // 1 for length before string
                    }
                    // Now check whether service type is one we are looking for
                    char *deb = strndup((char*)buf+12, pos-12); // Start after header till above loop found position
                    int diff = 0;

                    pos += 1; // end character
                    // As per DNS spec, our query service type should match but we get machine name in front of the
                    // message. We query \004_pzp\004_tcp\005local0x00 but get machine name appended in front
                    // \008_machine\004_pzp\004_tcp\005local0x00
                    if (strlen(tmp) == strlen(deb)) {
                       // This is correct scenario but machine responds with their machine name in front of service,
                       // so we need to remove that
                    } else if (strlen(tmp) < strlen(deb)){
                        diff = strlen(deb)-strlen(tmp); // Calculate length of machine name
                    }

                    if (strncmp(tmp, (char*)(buf+12+diff), pos-12-diff) == 0){ // Check query string that we sent matches
                                                                               // by removing machine name
                        if (diff > 0) {
                            machineName = calloc(1, diff);
                            machineName= strndup((char*)buf+13, diff-1); // This is not freed here. caller should free it
                        } // Machine name we sent to check peer PZP

                        // just need IP address rest of the information does not matter at the moment
                        while(pos < psize){ // Loop till end of packet
                            if(buf[pos + 1] == 1 ) { // Looking for Only IPV4 address
                                pos += 2 + 2 + 4;   //  Type (2)+ ClassCode (2)+ TTL (4)
                                if (buf[pos+1] == 4) { // Length of ip packet should be 4. We could check rest of the
                                                        // fields such as type, classcode... but omitting at the moment
                                    pos += 2; // Increment length
                                    sprintf(ret, "%d.%d.%d.%d",buf[pos], buf[pos+1], buf[pos+2], buf[pos+3]);
                                    free(deb);
                                    free(tmp);
                                    // Need to free machineName at receiving end
                                    return machineName; // End our search as we already found our PZP
                                }
                            } else {
                                pos += 2 + 2 + 4;   // Type (2)+ ClassCode (2)+ TTL (4)
                                pos += buf[pos+1];  // Length of the message
                                pos += 2;// Length field
                                if(pos < psize) pos+=2;// Offset
                            }
                        }
                        free(deb);
                        break;
                    }
                    free(deb);
//                    if( sendto(socketDesc, outMsg, msgSize , 0, (struct sockaddr*)&to, sizeof(to)) != msgSize){
//                      printf("\n Error Sending Message");
//                    }
                }
           }
        }
    }
    return machineName;
}
/*
int main() {
    char *ipAddress=calloc(1, 8);
    char *machineName = findPzp(ipAddress);
    printf("\n Found Machine Name: %s, %s", machineName, ipAddress);
    free(machineName);
    free(ipAddress);
    return 1;
}
*/