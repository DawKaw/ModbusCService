
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "mb.h"
#include "mq.h"
#include "helperf.h"
#include "asService.h"


// Global variables
bool finishApp = false;
int modbus_to_mqttCnt = 0;
int mqtt_to_modbusCnt = 0;
int modbus_loop_delay = 1000; //ms

int verbose = 2;

char *confFile = 0;
char *serviceUserName = 0;
char *pidFile= 0;

char *modbus_host = 0;
int modbus_port = MODBUS_TCP_DEFAULT_PORT;
int modbus_slave = 1;
char *mqtt_host = 0;
int mqtt_port = 1883;
char *mqtt_script = 0;
char *modbus_script = 0;

int cntModbusArr = 0;
int cntMqttArr = 0;



/**
 * @brief parseLine1 First pass. Parse one line of config file.
 *
 * @param line One line
 */
void parseLine1(char* line){
    //printfDaw("%d %s\n", n, line);

    int cntMax=8;
    int textMax=50;
    char texts[cntMax][textMax];
    memset(texts,0, cntMax * textMax);

    char separator[] = " \t=";
    char *txt;
    txt = strtok(line, separator );
    int cnt=0;
    while(txt != NULL && cnt < cntMax)
    {
        trim(txt);
        if (txt[0]!='#')
            strncpy(texts[cnt], txt, textMax);
        txt = strtok(NULL, separator);
        cnt++;
    }

    strtolower(texts[0]);

    //MODBUS CONFIG

    // Check text is modbus_ip
    if (cnt>=2 && strcmp(texts[0], "modbus_host")==0){
       if (strlen(texts[1])>0){
           modbus_host = (char*)malloc(strlen(texts[1]+1) * sizeof(char));
           strcpy(modbus_host, texts[1]);
           if (verbose>=3)
              printfDaw("  %s=[%s]\n", texts[0], modbus_host);
       }
    }

    // Check text is modbus_port
    if (cnt>=2 && strcmp(texts[0], "modbus_port")==0){
       modbus_port = atoi(texts[1]);
       if (verbose>=3)
          printfDaw("  %s=[%d]\n", texts[0], modbus_port);
    }

    // Check text is modbus_slave
    if (cnt>=2 && strcmp(texts[0], "modbus_slave")==0){
       modbus_slave = atoi(texts[1]);
       if (verbose>=3)
          printfDaw("  %s=[%d]\n", texts[0], modbus_slave);
    }

    // Check text is modbus_loop_delay
    if (cnt>=2 && strcmp(texts[0], "modbus_loop_delay")==0){
       modbus_loop_delay = atoi(texts[1]);
       if (verbose>=3)
          printfDaw("  %s=[%d]\n", texts[0], modbus_loop_delay);
    }


    //MQTT CONFIG

    // Check text is mqtt_host
    if (cnt>=2 && strcmp(texts[0], "mqtt_host")==0){
       if (strlen(texts[1])>0){
           mqtt_host = (char*)malloc(strlen(texts[1]+1) * sizeof(char));
           strcpy(mqtt_host, texts[1]);
           if (verbose>=3)
              printfDaw("  %s=[%s]\n", texts[0], mqtt_host);
       }
    }

    // Check text is mqtt_port
    if (cnt>=2 && strcmp(texts[0], "mqtt_port")==0){
       mqtt_port = atoi(texts[1]);
       if (verbose>=3)
          printfDaw("  %s=[%d]\n", texts[0], mqtt_port);
    }

    // Check text is mqtt_script
    if (cnt>=2 && strcmp(texts[0], "mqtt_script")==0){
       mqtt_script = (char*)malloc(strlen(texts[1]+1) * sizeof(char));
       strcpy(mqtt_script, texts[1]);
       if (verbose>=3)
          printfDaw("  %s=[%s]\n", texts[0], mqtt_script);
    }

    // Check text is modbus_script
    if (cnt>=2 && strcmp(texts[0], "modbus_script")==0){
       modbus_script = (char*)malloc(strlen(texts[1]+1) * sizeof(char));
       strcpy(modbus_script, texts[1]);
       if (verbose>=3)
          printfDaw("  %s=[%s]\n", texts[0], modbus_script);
    }


    // Check text is modbus_to_mqtt
    if (cnt>=5 && strcmp(texts[0], "modbus_to_mqtt")==0)
        modbus_to_mqttCnt++; //Only count

    // Check text is mqtt_to_modbus
    if (cnt>=5 && strcmp(texts[0], "mqtt_to_modbus")==0)
        mqtt_to_modbusCnt++;  //Only count
}


/**
 * @brief parseLine1 Second pass. Parse one line of config file
 *        searching modbus_to_mqtt and mqtt_to_modbus commands.
 *
 * @param line One line
 */
int parseLine2(char* line){
    int cntMax=8;
    int textMax=50;
    char texts[cntMax][textMax];
    memset(texts,0, cntMax * textMax);

    char separator[] = " \t=";
    char *txt;
    txt = strtok(line, separator );
    int cnt=0;

    while(txt != NULL && cnt < cntMax)
    {
        trim(txt);
        if (txt[0]!='#')
            strncpy(texts[cnt], txt, textMax);
        txt = strtok(NULL, separator);
        cnt++;
    }

    //#                  mode addr nb topic [retain]
    //modbus_to_mqtt READ_BIN 16 16 fvz_in/V2to3 retain

    // Check text is modbus_to_mqtt
    if (cnt>=5 && strcmp(texts[0], "modbus_to_mqtt")==0){
       if (verbose>=3)
          printfDaw("%s %s %s %s %s\n", texts[0], texts[1], texts[2], texts[3], texts[4]);

       strtolower(texts[1]);

       int mode = NOT_USED;
       if (strcmp(texts[1], "read_bin")==0)
           mode = READ_BIN;
       else if (strcmp(texts[1], "read_bin")==0)
           mode = READ_WORD;

       int addr = atoi(texts[2]);
       int nb   = atoi(texts[3]);
       char *topic = texts[4];
       int retainMqtt = 0;

       if (cnt>=6 && strcmp(texts[5], "retain")==0) //last cmd is "retain"?
          retainMqtt = 1;

       if (verbose>=3)
          printfDaw("  [%d] mode:%d addr:%d nb:%d topic:%s retain:%d\n", cntModbusArr, mode, addr, nb, topic, retainMqtt);

       setTopics_modbus(cntModbusArr, mode, addr, nb, topic, retainMqtt);

       cntModbusArr++;
    }


    // Check text is mqtt_to_modbus
    if (cnt>=5 && strcmp(texts[0], "mqtt_to_modbus")==0){
       if (verbose>=3)
          printfDaw("%s %s %s %s %s\n", texts[0], texts[1], texts[2], texts[3], texts[4]);

       strtolower(texts[1]);

       int mode = NOT_USED;
       if (strcmp(texts[1], "write_bin")==0)
           mode = WRITE_BIN;
       else if (strcmp(texts[1], "write_bin")==0)
           mode = WRITE_WORD;

       int addr = atoi(texts[2]);
       int nb   = atoi(texts[3]);
       char *topic = texts[4];
       int retainmodbus = 0;

       if (cnt>=6 && strcmp(texts[5], "retain")==0) //last cmd is "retain"?
          retainmodbus = 1;

       if (verbose>=3)
          printfDaw("  [%d] mode:%d addr:%d nb:%d topic:%s retain:%d\n", cntMqttArr, mode, addr, nb, topic, retainmodbus);

       setTopics_mqtt(cntMqttArr, mode, addr, nb, topic, retainmodbus);

       cntMqttArr++;
    }
    return 0;
}


/**
 * @brief parseConfigFile Parse config file.
 *
 * @param confFile
 */
void parseConfigFile(const char* confFile){
    FILE * fc;
    fc = fopen(confFile, "r");
    if (fc == NULL){
        if (verbose>=1)
           fprintfDaw(stderr, "Can't open config file '%s'\n", confFile);
        exit(EXIT_FAILURE);
    }
    if (verbose>=3)
        printfDaw("Reading config file '%s'\n", confFile);

    int lineLen = 255;
    char line[lineLen];
    //First pass parse
    while(fgets(line, lineLen, fc))
        parseLine1(line);

    if (verbose>=3){
        printfDaw("  modbus_to_mqttCnt: %d\n", modbus_to_mqttCnt);
        printfDaw("  mqtt_to_modbusCnt: %d\n", mqtt_to_modbusCnt);
    }

    fseek(fc, 0, SEEK_SET);
    setTopicsLen_modbus(modbus_to_mqttCnt);
    setTopicsLen_mqtt(mqtt_to_modbusCnt);

    //Second pass parse only for modbus_to_mqtt and mqtt_to_modbus commands
    while(fgets(line, lineLen, fc))
        parseLine2(line);

    fclose(fc);
    if (verbose>=3)
        printfDaw("Finish reading config.\n\n");
}






// ****  Modbus Callback ****
void callbackModbus(int i, const data_modbusData_t *d){
    int lenFactor=2;
    if (d->mode==READ_WORD)
       lenFactor=6;

    int maxLen = (d->rnb*lenFactor)+20;
    char str[maxLen];  memset(str, 0, maxLen);
    char str2[maxLen]; memset(str2, 0, maxLen);
    strcpy(str, "[");
    char val[8];  memset(val, 0, 8);
    char val2[8]; memset(val2, 0, 8);

    for (int i=0; i<d->rnb; i++){
        char comasepar = (i==d->rnb-1)? 0 : ',';
        char spacesepar = (i==d->rnb-1)? 0 : ' ';
        if (d->mode==READ_BIN){
            snprintf(val, 8, "%d%c", d->tab_bits[i], comasepar);
            snprintf(val2, 8, "%d%c", d->tab_bits[i], spacesepar);
        } else {
            snprintf(val, 8, "%d%c", d->tab_registers[i], comasepar);
            snprintf(val2, 8, "%d%c", d->tab_registers[i], spacesepar);
        }
        strncat(str, val, maxLen);
        strncat(str2, val2, maxLen);
    }
    strncat(str, "]", maxLen);
    if (verbose>=4){
        if (mqtt_host==0)
            printfDaw("[%d] Received Modbus message[%s]%s but not sent to mqtt.\n", i, d->topic, str);
        else
            printfDaw("[%d] Sending Modbus->Mqtt[%s]%s\n", i, d->topic, str);
    }

    if (mqtt_host!=0)
        pub_mqtt(d->topic, str, 0, d->retain); //Send data via mqtt.

    // Run script
    if (modbus_script!=0){
        if (getuid()==0){
           if (verbose>=1)
              fprintfDaw(stderr, "Don't run script as root\n");
        } else {
            int maxCmd = strlen(modbus_script)+maxLen+2;
            char strCommand[maxCmd];
            memset(strCommand, 0, maxLen);

            snprintf(strCommand, maxCmd, "%s %s %d %s", modbus_script, d->topic, d->addr, str2);
            if (verbose>=4)
                printfDaw("Running script [%s]\n", strCommand);

            if (verbose>=1){
                if (system(strCommand) != 0)
                    fprintfDaw(stderr, "Run script failed.\n");
            }
        }
    }
}





// ****  Mqtt Callback ****
void callbackMqtt(int topicNr, const data_mqttData_t *d){
    if(d->payloadLen<-0)
        return;

    if (verbose>=4){
        if (modbus_host!=0)
           printfDaw("[%d] Sending Mqtt->modbus[%s]%s\n", topicNr, d->topic, d->payload);
        else
           printfDaw("[%d] Received message from mqtt[%s]%s but not sent to modbus.\n", topicNr, d->topic, d->payload);
    }

    if (modbus_host==0)
        return;

    uint8_t dataB[d->nb];   //bits buffer
    memset(dataB, 0, d->nb);

    uint16_t dataW[d->nb];  //words buffer
    memset(dataW, 0, d->nb);

    if (parseMessage(d->payload, d->payloadLen, verbose))
       return;

    // **** Split text by "," separator, atoi, put 0/1 to data[] array.
    int cnt=0;
    char *pt;
    pt = strtok (d->payload, ",");
    while (pt != NULL) {

      unsigned int a = strtoul(pt, NULL, 10);

      if (d->mode == WRITE_BIN)
          dataB[cnt] = (a>=1) ? 1 : 0;
      else
      if (d->mode == WRITE_WORD)
          dataW[cnt] = a;

      pt = strtok (NULL, ",");
      cnt++;
      if (cnt>=d->nb)
          break;
    }
    // **** end spliting

    if (d->mode == WRITE_BIN)
       writeBits_modbus(d->addr, d->nb, dataB); //Write bits via modbus.
    else  if (d->mode == WRITE_WORD)
       writeWords_modbus(d->addr, d->nb, dataW); //Write bits via modbus.


    // Run script
    if (mqtt_script!=0){

        if (getuid()==0){
           if (verbose>=1)
              fprintfDaw(stderr, "Don't run script as root!\n");
        } else {
            int maxCmd = (d->nb * 9) + strlen(d->topic) + strlen(mqtt_script) + 2;
            char strCommand[maxCmd]; memset(strCommand, 0, maxCmd);

            snprintf(strCommand, maxCmd, "%s %s", mqtt_script, d->topic);
            char val2[8]; memset(val2, 0, 8);
            for (int i=0; i<d->nb; i++){
                if (d->mode == WRITE_BIN)
                    snprintf(val2, 8, " %d", dataB[i]);
                else  if (d->mode == WRITE_WORD)
                    snprintf(val2, 8, " %d", dataW[i]);
                strncat(strCommand, val2, maxCmd);
            }

            if (verbose>=4)
                printfDaw("Running script [%s]\n", strCommand);

            if (verbose>=1){
                if (system(strCommand) != 0)
                    fprintfDaw(stderr, "Run script failed.\n");
            }
        }
    }
}


// **** Callback of system signals ****
void sig_handler(int signum){

  if (signum == SIGINT){
     finishApp = true;
     romovePidfiles(verbose);
     if (verbose>=1)
        printfDaw("Signal SIGINT\n");
  } else if (signum == SIGHUP){
    if (verbose>=1)
        printfDaw("Signal SIGHUP\n");
  } else {
    if (verbose>=1)
       printfDaw("Signal %d\n", signum);
  }
}


// **** Main exec function ****
int main(int argc, char *argv[]){

    int  opt;
    while ((opt=getopt(argc, argv, "c:Vv:s:p:")) != -1){

        switch (opt) {

           case 'c':
                confFile = optarg;
                break;

           case 'v':
                verbose = atoi(optarg);
                break;

           case 'V':
                printfDaw("Version: %s\n", VERSION);
                exit(EXIT_SUCCESS);
                break;

           case 's':
                serviceUserName = optarg;
                break;

           case 'p':
                pidFile = optarg;
                break;

           default: /* '?' */
                printfDaw("Usage: %s -c config file name.\n", argv[0]);
                printfDaw("    [-c config] Config file name.\n");
                printfDaw("    [-v level] Verbose level:\n");
                printfDaw("       0 - silence\n");
                printfDaw("       1 - only faults\n");
                printfDaw("       2 - faults and a few notice (default)\n");
                printfDaw("       3 - more notice\n");
                printfDaw("       4 and more full verbose.\n");
                printfDaw("    [-V version] Print version.\n");
                printfDaw("    [-s user] Run as service as user.\n");
                printfDaw("    [-p pifFile] Save PID to file name.\n");
                printfDaw("\n");
                printfDaw("Example:\n  %s -c modbuscs.conf\n", argv[0]);
                printfDaw("  %s -c modbuscs.conf -s modbuserv\n", argv[0]);
                exit(EXIT_FAILURE);
           break;
        }
    }


    if (confFile==0){
       if (verbose>=1)
          fprintfDaw(stderr, "Config file name parameter (-c) is missing.\n");
       return EXIT_FAILURE;
    }


    // Run programm as service in background.
    if (serviceUserName!=0) {
       asservice(serviceUserName, argv[0], pidFile, verbose);
    }

    if (verbose>=2)
        printfDaw("Starting %s version: %s\n", argv[0], VERSION);

    signal(SIGINT, sig_handler); // Register signal handler SIGINT
    //signal(SIGHUP, sig_handler); // Register signal handler SIGHUP





    parseConfigFile(confFile);

    //if command in config file "modbus_to_mqttCnt" is missing then zero modbus_host.
    if (modbus_host!=0 && modbus_to_mqttCnt==0){
        free(modbus_host);
        modbus_host=0;
    }

    //Modbus init only when modbus is configured correctly in config file.
    if (modbus_host!=0){
        init_modbus(modbus_host, modbus_port, modbus_slave, verbose);
        setCallBack_modbus(&callbackModbus);
    }

    //if command in config file "mqtt_to_modbus" is missing then zero mqtt_host.
    if (mqtt_host!=0 && mqtt_to_modbusCnt==0){
        free(mqtt_host);
        mqtt_host=0;
    }

    //Mqtt init only when mqtt is configured correctly in config file.
    if (mqtt_host!=0){
        init_mqtt(mqtt_host, mqtt_port, verbose);
        setCallBack_mqtt(&callbackMqtt);
        start_loop_mqtt();
    }



    // **** Main loop ****
    int read_status = 1;
    while (!finishApp) {
        if (modbus_host!=0){    //Host for modbus is set?

            if (read_status == 0) //Last state of read modbus ok is?
               read_status = init_modbus(modbus_host, modbus_port, modbus_slave, verbose); //No. Try renew connection.

            if (read_status != 0) //If connection is ok then read data via modbus;
               read_status = read_modbus(); //Read data via modbus.
        }
        usleep((modbus_loop_delay*1000)-(100*1000));  //Delay
    }

    if (verbose>=1)
        printfDaw("Terminate application %s\n", argv[0]);

    setTopicsLen_modbus(0);
    finish_modbus();

    setTopicsLen_mqtt(0);
    finish_mqtt();

    if (modbus_host!=0)
        free(modbus_host);
    if (mqtt_host!=0)
        free(mqtt_host);
    if (modbus_script!=0)
        free(modbus_script);
    if (mqtt_script!=0)
        free(mqtt_script);

    return 0;
}
