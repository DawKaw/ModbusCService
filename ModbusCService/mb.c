#include "mb.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>  //For errno - the error number
#include <netdb.h>	//hostent
#include <arpa/inet.h>

#ifndef NULL
    #define NULL ((void *)0)
#endif

modbus_t *ctx = 0;
int countErr = 0;

data_modbusData_t *arrayBuff;
int arrayBuffCnt = 0;
void (*ptrCallBack_modbus)(int, const data_modbusData_t*) = 0;
int onlyOne = 0;
int verb;


int hostname_to_ip(const char *hostname, char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ( (he = gethostbyname( hostname ) ) == NULL){
        // get the host info
        herror("gethostbyname");
        return 1;
    }

    addr_list = (struct in_addr **) he->h_addr_list;

    for(i = 0; addr_list[i] != NULL; i++){
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
    return 1;
}


/**
 * @brief init_modbus Initalize modbus connection.
 * @param ip_address Address to modbus slave (server).
 * @param port Port to modbus slave (server).
 * @return Return 0 if success full.
 */
int init_modbus(const char *host, int port, int slave, int verbose)
{
    verb = verbose;
    if (onlyOne==0 && verb>=4){
        printfDaw("libmodbus ver: %s\n", LIBMODBUS_VERSION_STRING);
        onlyOne=1;
    }

    char ip[100];
    if (strlen(host)>0 && hostname_to_ip(host, ip) != 0){
        if (verb>=1)
            fprintfDaw(stderr, "Can't resolve modbus hostname [%s]!\n", host);
        return 0;
    }

    if (verb>=2)
        printfDaw("Resolve modbus hostname [%s] to [%s]\n", host, ip);

    ctx = modbus_new_tcp(ip, port);
    if (modbus_connect(ctx) == -1) {
        if (verb>=1)
            fprintfDaw(stderr, "Connection failed: %s\n", modbus_strerror(errno));
         modbus_free(ctx);
         ctx=0;
         return 0;
    }
    modbus_set_slave(ctx, slave);
    return 1;
}

/**
 * @brief finish_modbus Finish connection and library.
 */
void finish_modbus()
{
    modbus_close(ctx);
    modbus_free(ctx);
    ctx=0;
}

/**
 * @brief chkConn Check number of data exchange faults.
 *          If exceed MAX_CONN_ERR, break connection.
 *
 * @param ret A value representing the connection status.
 * @return Void.
 */
void chkConn(int ret){
  if (ret<0){
      countErr++;
      modbus_flush(ctx);
  } else {
      countErr=0;
      return;
  }

  if (countErr>=MAX_CONN_ERR){
      if (verb>=1)
         printfDaw("countErr reach max: %d\nReconnecting.", countErr);
      countErr=0;
      finish_modbus();
  }
}

/**
 * @brief readBits_modbus Read data as bits.
 * @param addr Address of Register to read.
 * @param nb Nuber or registers.
 * @param dest Array destination.
 * @return
 */
int readBits_modbus(int addr, int nb, uint8_t *dest){
    return modbus_read_bits(ctx, addr, nb, dest); //Read bits
}

/**
 * @brief writeBits_modbus Write data as bits.
 * @param addr Address of Register to write.
 * @param nb Nuber or registers.
 * @param data Array source.
 * @return
 */
int writeBits_modbus(int addr, int nb, const uint8_t *data){
    return modbus_write_bits(ctx, addr, nb, data); //Write bits
}

/**
 * @brief readWords_modbus Read data as Words.
 * @param addr Address of Register to read.
 * @param nb Nuber or registers.
 * @param dest Array destination.
 * @return
 */
int readWords_modbus(int addr, int nb, uint16_t *dest){
    return modbus_read_registers(ctx, addr, nb, dest); //Read register
}

/**
 * @brief writeWords_modbus Write data as Words.
 * @param addr Address of Register to read.
 * @param nb Nuber or registers.
 * @param data Array source.
 * @return
 */
int writeWords_modbus(int addr, int nb, const uint16_t *data){ //Write register
    return modbus_write_registers(ctx, addr, nb, data);
}


/**
 * @brief zeroBuff  Clear arrayBuff structure.
 */
void zeroBuff(){
    for (int i=0; i<arrayBuffCnt; i++) {
      data_modbusData_t *d = &arrayBuff[i];
      d->mode = NOT_USED;
      d->addr = 0;
      d->nb = 0;
      d->rnb = -1;
      d->tab_bits = 0;
      d->tab_registers = 0;
      d->tab_bits_old = 0;
      d->tab_registers_old = 0;
      d->topic = 0;
      d->retain = 0;
    }
}

/**
 * @brief freeBufTab Free memory of tab_bits, tab_registers,tab_bits_old
 *      tab_registers_old, topic in arrayBuff structure.
 */
void freeBufTab(){
    for (int i=0; i<arrayBuffCnt; i++) {
        data_modbusData_t *d = &arrayBuff[i];
        if (d->tab_bits!=0)         {free(d->tab_bits);          d->tab_bits=0;}
        if (d->tab_registers!=0)    {free(d->tab_registers);     d->tab_registers=0;}
        if (d->tab_bits_old!=0)     {free(d->tab_bits_old);      d->tab_bits_old=0;}
        if (d->tab_registers_old!=0){free(d->tab_registers_old); d->tab_registers_old=0;}
        if (d->topic!=0)            {free(d->topic);             d->topic=0;}
    }
}

/**
 * @brief setArrayBuffLen_modbus Set arrayBuff length.
 * @param count Nuber of modbus structures.
 * @return Return current nuber of topics. 0 if error.
 */
int setTopicsLen_modbus(const int count){

    arrayBuffCnt = 0;
    if (arrayBuff>0) {
        freeBufTab();
        free(arrayBuff);
    }

    if (count<=0)
        return 0;

    arrayBuffCnt = count;
    arrayBuff = (data_modbusData_t*)malloc(count * sizeof(data_modbusData_t));
    if (arrayBuff == 0) {
        if (verb>=1)
            fprintfDaw(stderr, "Memory for arrayBuff not allocated.\n");
        arrayBuffCnt = 0;
    }

    zeroBuff();
    return arrayBuffCnt;
}

/**
 * @brief setArrayBuff_modbus Set data of arrayTopics structure.
 * @param n Index of arrayTopics structure.
 * @param mode Mode READ_BIN or READ_WORD.
 * @param addr Address of Register to read.
 * @param nb Nuber or registers.
 * @param topic Topic as text.
 * @return Return 0 i successfull.
 */
int setTopics_modbus(const int n, const int mode, const int addr, const int nb, const char *topic, int retain){
    if (n>arrayBuffCnt-1){
        if (verb>=1)
            fprintfDaw(stderr, "On call setArrayBuff_modbus parametr n(%d) exceed arrayBuffCnt(%d)\n", n, arrayBuffCnt);
        return 1;
    }

    data_modbusData_t *d = &arrayBuff[n];
    unsigned int len = (unsigned int)strlen(topic);

    d->mode = mode; //0-NOT_USED, 1-READ_BIN, 2-READ_WORD, 11-WRITE_BIN, 12-WRITE_WORD
    d->addr = addr;
    d->nb = nb;
    d->retain = retain;
    d->topic = (char*)malloc((len+1) * sizeof(char));
    memcpy(d->topic, topic, len+1);

    if (d->mode==READ_BIN || d->mode==WRITE_BIN){
        d->tab_bits     = (uint8_t*)malloc(d->nb * sizeof(uint8_t));
        d->tab_bits_old = (uint8_t*)malloc(d->nb * sizeof(uint8_t));
        memset(d->tab_bits_old, 7, d->nb * sizeof(uint8_t));
    }
    else if (d->mode==READ_WORD || d->mode==WRITE_WORD){
        d->tab_registers     = (uint16_t*)malloc(d->nb * sizeof(uint16_t));
        d->tab_registers_old = (uint16_t*)malloc(d->nb * sizeof(uint16_t));
        memset(d->tab_registers_old, 7, d->nb * sizeof(uint16_t));
    }
    return 0;
}

/**
 * @brief setCallBack_modbus Set callback function handle.
 */
void setCallBack_modbus(void (*ptr)(int, const data_modbusData_t*)){
  ptrCallBack_modbus = ptr;
}

/**
 * @brief checkSameData Compare data in tab_bits_old and tab_bits.
 * @param d Data as data_modbusData_t structura.
 * @return If the same, return 0;
 */
int checkSameData(data_modbusData_t *d){
    int ret = 0;

    if (d->mode==READ_BIN){
        ret = memcmp(d->tab_bits_old, d->tab_bits, d->nb * sizeof(uint8_t));
    }  else
      if (d->mode==READ_WORD){
        ret = memcmp(d->tab_registers_old, d->tab_registers, d->nb * sizeof(uint16_t));
    }

    if (d->mode==READ_BIN)
        memcpy(d->tab_bits_old, d->tab_bits, d->nb * sizeof(uint8_t));

    if (d->mode==READ_WORD)
        memcpy(d->tab_registers_old, d->tab_registers, d->nb * sizeof(uint16_t));
    return ret;
}

/**
 * @brief read_modbus Read registers accordinf arrayBuff.
 * @return Void.
 */
int read_modbus()
{
    int ret;
    for (int i=0; i<arrayBuffCnt; i++){
        data_modbusData_t *d = &arrayBuff[i];

        if (d->mode==READ_BIN){
           ret = readBits_modbus(d->addr, d->nb, d->tab_bits); //Read bits
        }
        else if (d->mode==READ_WORD){
           ret = readWords_modbus(d->addr, d->nb, d->tab_registers); //Read register
        }

        if (d->rnb==-1 && verb>=4)
            printfDaw("[%d] Reading modbus registers %d-%d as %s\n", i, d->addr, (d->addr + d->nb-1), (d->mode==READ_BIN)? "binary" : "word");
        d->rnb = ret;
        chkConn(ret);

        if (ret == d->nb){
            if (checkSameData(d)!=0){ //Exclude repeated values
                if (ptrCallBack_modbus>0)
                    ptrCallBack_modbus(i, d);
            }

        } else {
            if (countErr>0 && verb>=1)
                fprintfDaw(stderr, "[%d] Modbud read error : %s\n", i, modbus_strerror(errno));
        }
        usleep(100*1000);
    }
    return (ctx==0) ? 0 : 1;
}
