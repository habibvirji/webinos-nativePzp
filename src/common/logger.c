#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "../webinos.h"

void logPrint(char *file, int line, char *fmt, ...){
    va_list list;
    char *r, *p;
    int i;

    char buffer[80];
    time_t rawtime;
    struct tm *timeinfo;
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer,80,"%d.%m.%Y %I:%M:%S",timeinfo);

    va_start(list, fmt);
    fprintf(stderr, "\n[%s] info %s(%d): ", buffer, file, line);
    for (p = fmt; *p; ++p) {
       if ( *p != '%' ) {
           fputc(*p, stderr);
       } else {
          switch ( *++p ){
              case 's':{
                  r = va_arg( list, char * );
                  fprintf(stderr,"%s", r);
                  continue;
              }
              case 'd':{
                  i = va_arg( list, int );
                  fprintf(stderr,"%d", i);
                  continue;
              }
              default:
                 fputc(*p, stderr);
          }
      }
    }
    va_end( list );
}
