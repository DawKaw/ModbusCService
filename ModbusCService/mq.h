#ifndef MQ_H
#define MQ_H

#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include "helperf.h"


#ifndef NULL
    #define NULL ((void *)0)
#endif

#ifndef bool
    #define bool char
#endif

#ifndef true
    #define true	1
#endif

#ifndef false
    #define false	0
#endif

#define UNUSED(x) (void)(x)


typedef struct _data_mqttData_t {
    int mode;           //0-NOT_USED, 1-READ_BIN, 2-READ_WORD, 11-WRITE_BIN, 12-WRITE_WORD
    int addr;           //Register addr
    int nb;             //Number of registers
    int payloadLen;     //MQTT payloadLen
    int retain;         //MQTT retain
    char *topic;        //MQTT topic
    char *payload;      //MQTT payload
} data_mqttData_t;


int init_mqtt(const char *host, int port, int verbose);
void finish_mqtt();
int start_loop_mqtt();
int pub_mqtt(const char *topic, const char *payload, int qos, bool retain);
void setCallBack_mqtt(void (*ptr)(int, const data_mqttData_t*));
int setTopicsLen_mqtt(const int count);
int setTopics_mqtt(const int n, const int mode, const int addr, const int nb, const char *topic, int retain);

#endif // MQ_H
