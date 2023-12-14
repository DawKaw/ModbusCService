#include "helperf.h"
#include <syslog.h>

void trim(char* txt){
    int l = strlen(txt)-1;
    if (txt[l]=='\n' || txt[l]==' ' || txt[l]=='\t')
        txt[l]=0;
}


// **** Helper fo remove char from text ****
void removeChar(char *str, char garbage) {
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}


void strtolower(char *str){
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = tolower(*src);
        dst++;
    }
    *dst = '\0';

}


int parseMessage(char *payload, int len, int verbose){
    if (payload[0]!='[' || payload[len-1]!=']'){
        if (verbose>0)
           printf("<-Parse mqtt message error. '[' or ']' is missing.\n");
        fflush(stdout);
        return 1;
    }
    removeChar(payload, '[');
    removeChar(payload, ']');
    return 0;
}


void printfDaw(const char *format, ...){
    va_list args;
    va_start(args, format);

    if (serviceEnable!=0)
        vsyslog(LOG_INFO, format, args);
    else
        vprintf(format, args);
}

void setServiceEnable(int val){
    serviceEnable = val;
}

int fprintfDaw(FILE *stream, const char *format, ...){
    va_list args;
    va_start(args, format);

    if (serviceEnable!=0)
       vsyslog(LOG_INFO, format, args);
    else
       return vfprintf(stream, format, args);

    return 0;
}
