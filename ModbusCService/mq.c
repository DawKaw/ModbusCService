#include "mq.h"
#include <string.h>

struct mosquitto *mosq = NULL;

data_mqttData_t *arrayTopics;
int  arrayTopicsCnt = 0;
void (*ptrCallBack_mqtt)(int, const data_mqttData_t*) = 0;
int verb;

/**
 * @brief findTopicNr Find index of topic (text) in arrayTopicsCnt.
 *
 * @param topic Topic as text.
 * @return Index of topic.
 */
int findTopicNr(const char* topic){
    for (int i=0; i<arrayTopicsCnt; i++) {
        data_mqttData_t *t = &arrayTopics[i];
        //printfDaw("topic:[%s] payload:[%s]\n", t->topic, t->payload);
        int r = strcmp(t->topic, topic);
        if (r==0)
            return i;
    }
    return -1;
}

/**
 * @brief my_message_callback Callback of libmosquitto called when subscribed
 *      messages arrived.
 *
 * @param mosq Internal libmosquitto structure.
 * @param userdata
 * @param message Arrived message structure.
 */
void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    UNUSED(mosq);
    UNUSED(userdata);
/*
    if(message->payloadlen){
         printfDaw("message: %s %s\n", message->topic, (char*)message->payload);
    } else {
         printfDaw("message: %s (null)\n", message->topic);
    }
    fflush(stdout);
*/

    //Find index of arrived topic.
    int topicNr = findTopicNr(message->topic);
    //printfDaw("topicNr:%d  \n", topicNr);
    if (topicNr<0) return;

    //Get pointer to properly arrayTopics according arrived topic.
    data_mqttData_t *t = &arrayTopics[topicNr];

    if (t->payload!=0) free(t->payload); //Free memory of previous payload

    //Allocate a new amount of memory and copy payload to it.
    t->payload = (char*)malloc((message->payloadlen+1) * sizeof(char));
    memcpy(t->payload, message->payload, message->payloadlen+1);

    t->payloadLen = message->payloadlen; //Save length of payload
    //printfDaw("topic:[%s] payload:[%s]\n", t->topic, t->payload);

    if (ptrCallBack_mqtt!=0) //Call callback
        ptrCallBack_mqtt(topicNr, t);
}

/**
 * @brief my_connect_callback Callback of libmosquitto called when connection
 *          to mqtt server is successful.
 *
 * @param mosq Internal libmosquitto structure.
 * @param userdata
 * @param result The return code of the connection response.
 */
void my_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
    UNUSED(userdata);
    if(!result){
        for (int i=0; i<arrayTopicsCnt; i++) {
            data_mqttData_t *d = &arrayTopics[i];
            if (d!=0) {
                                //  mosq, mid, sub, qos
                mosquitto_subscribe(mosq, NULL,  d->topic, 0);  //Subscribe to broker information topics on successful connect
                if (verb>=2)
                   printfDaw("[%d] Subscribe topic:[%s]\n", i, d->topic);
            }
        }
    } else {
       if (verb>=1)
          fprintfDaw(stderr, "Connect failed\n");
    }
}


/**
 * @brief init_mqtt Initalize mqtt connection and callbacks.
 *
 * @param host Address to mqtt server.
 * @param port Port to mqtt server.
 * @return Return 0 if success full.
 */
int init_mqtt(const char *host, int port, int verbose)
{
    verb = verbose;
    int major, minor, revision;
    int ret = mosquitto_lib_version(&major, &minor, &revision);
    if (ret>0){
        if (verb>=4)
            printfDaw("libmosquitto ver: %d.%d.%d\n", major, minor, revision) ;
    } else {
        if (verb>=1)
            printfDaw("Initialize mosquitto_lib error %d\n", ret);
        return 1;
    }


    mosquitto_lib_init();
    //                     id, clean_session, obj
    mosq = mosquitto_new(NULL, true, NULL);
    if(!mosq){
         if (verb>=1)
            fprintfDaw(stderr, "Error: Out of memory.\n");
         return 2;
    }
    mosquitto_username_pw_set(mosq, "webaccess", "asdfasdf");
    mosquitto_connect_callback_set(mosq, my_connect_callback);
    mosquitto_message_callback_set(mosq, my_message_callback);

                      //*mosq,*host, port, keepalive);
    if(mosquitto_connect(mosq, host, port, 60)){
         if (verb>=1)
            fprintfDaw(stderr, "Unable to connect to mqtt server '%s:%d'.\n", host, port);
         return 3;
    }
    return 0;
}

/**
 * @brief finish_mqtt Finish connection and library.
 */
void finish_mqtt()
{
    mosquitto_loop_stop(mosq, true);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
}

/**
 * @brief start_loop_mqtt Start internal thread to receive messages.
 * @return
 */
int start_loop_mqtt()
{
    mosquitto_loop_start(mosq);
}

/**
 * @brief pub_mqtt Send message (public) with topic and payload on server.
 *
 * @param topic Topic as text.
 * @param payload Payload as text.
 * @param qos Qos of message.
 * @param retain Retain of message.
 * @return
 */
int pub_mqtt(const char *topic, const char *payload, int qos, bool retain)
{
    unsigned int payloadlen = strlen(payload);
    return mosquitto_publish(mosq, NULL, topic, payloadlen, payload, qos, retain);
}

/**
 * @brief setCallBack_mqtt Set callback function handle.
 */
void setCallBack_mqtt(void (*ptr)(int, const data_mqttData_t*)){
    ptrCallBack_mqtt = ptr;
}

/**
 * @brief zeroBuff_mqtt Clear arrayTopics structure.
 */
void zeroBuff_mqtt(){
    for (int i=0; i<arrayTopicsCnt; i++) {
      data_mqttData_t *t = &arrayTopics[i];
      if (t->topic>0){
          //fprintf(stderr, "debig [%d]\n", arrayTopicsCnt);
          free(t->topic);
      }
    }
}

/**
 * @brief freeBufTab_mqtt Free memory of topic, payload in arrayTopics structure.
 */
void freeBufTab_mqtt(){
    for (int i=0; i<arrayTopicsCnt; i++) {
        data_mqttData_t *d = &arrayTopics[i];
        if (d->topic!=0)   {free(d->topic);   d->topic=0;}
        if (d->payload!=0) {free(d->payload); d->payload=0;}
    }
}

/**
 * @brief setTopicsLen_mqtt Set arrayTopics length.
 * @param count Nuber of topics.
 * @return Return current nuber of topics. 0 if error.
 */
int setTopicsLen_mqtt(const int count){

    arrayTopicsCnt = 0;
    if (arrayTopics>0) {
        freeBufTab_mqtt();
        free(arrayTopics);
    }

    if (count<=0)
        return 0;

    arrayTopicsCnt = count;
    arrayTopics = (data_mqttData_t*)malloc(count * sizeof(data_mqttData_t));
    if (arrayTopics == 0) {
        if (verb>=1)
            fprintfDaw(stderr, "Memory for arrayBuff not allocated.\n");
        arrayTopicsCnt = 0;
    }

    zeroBuff_mqtt();

    return arrayTopicsCnt;
}

/**
 * @brief setTopics_mqtt Set data of arrayTopics structure.
 * @param n Index of topic.
 * @param mode Mode WRITE_BIN or WRITE_WORD.
 * @param addr Address of Register to write.
 * @param nb Nuber or registers.
 * @param topic Topic as text.
 * @return Return 0 i successfull.
 */
int setTopics_mqtt(const int n, const int mode, const int addr, const int nb, const char *topic, int retain){
    if (n>arrayTopicsCnt-1){
       if (verb>=1)
          fprintfDaw(stderr, "On call setTopics_mqtt parametr n(%d) exceed arrayTopicsCnt(%d)\n", n, arrayTopicsCnt);
       return 1;
    }

    unsigned int len = (unsigned int)strlen(topic);
    data_mqttData_t *t = &arrayTopics[n];
    t->mode = mode;
    t->addr = addr;
    t->nb = nb;
    t->topic = (char*)malloc((len+1) * sizeof(char));
    t->payload = 0;
    t->payloadLen = 0;
    t->retain = retain;
    memcpy(t->topic, topic, len+1);

    //printfDaw("Topoc[%d]:[%s]\n", 0, arrayTopics[0]->topic);
    //printfDaw("Topoc[%d]:[%s]\n", 1, arrayTopics[1]->topic);
    return 0;
}
