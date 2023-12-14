#ifndef MB_H
#define MB_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus/modbus.h>
#include "helperf.h"

#define MAX_CONN_ERR 4
#define NOT_USED 0
#define READ_BIN 1
#define READ_WORD 2
#define WRITE_BIN 11
#define WRITE_WORD 12


typedef struct _data_modbusData_t {
    int mode;           //0-NOT_USED, 1-READ_BIN, 2-READ_WORD, 11-WRITE_BIN, 12-WRITE_WORD
    int addr;           //Register addr
    int nb;             //Number of registers
    int rnb;            //Number of received registers
    int retain;         //Retain topic in mqtt;
    uint8_t *tab_bits;           //Buffer for bits
    uint16_t *tab_registers;     //Buffer for words
    uint8_t *tab_bits_old;       //Buffer for bits old value
    uint16_t *tab_registers_old; //Buffer for words old value
    char *topic;        //MQTT topic
} data_modbusData_t;




int init_modbus(const char *host, int port, int slave, int verbose);
void finish_modbus();

int readBits_modbus(int addr, int nb, uint8_t *dest);//Read bits
int writeBits_modbus(int addr, int nb, const uint8_t *data); //Write bits

int readWords_modbus(int addr, int nb, uint16_t *dest); //Read register as words
int writeWords_modbus(int addr, int nb, const uint16_t *data);

int read_modbus();
//int write_modbus(const char *topic, const char *payload, int payloadLen, int topicNr);

int setTopicsLen_modbus(const int count);
int setTopics_modbus(const int n, const int mode, const int addr, const int nb, const char *topic, int retain);
void setCallBack_modbus(void (*ptr)(int, const data_modbusData_t*));

#endif // MB_H
