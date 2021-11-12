#include "MQTTClient.h"
#include "mqtt.h"
#include "stdio.h"
#include "cfg.h"
#include "conn.h"
#include "log.h"

int mqtt_loop(void)
{
    int fd = 0;
    int i = 0;
    int ret = 8;
    int count = 0;
    int nbytes = 0;

    unsigned char rec_buff[512] = {0};
    unsigned char write_485data[1024] = {0};
    unsigned char write_buff[512] = {0};
    int rc = 0;
    int mqttautoconntime  = 0;

start:
    logw(1, "init  %s \n", cfg.mqttaddr);
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;

    MQTTClient_create(&client, cfg.mqttaddr, cfg.mqttclientid,
                      MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = cfg.mqttinterval;
    conn_opts.username = (const char *)(&cfg.mqttusername);
    conn_opts.password = (const char *)(&cfg.mqttpassword);
    conn_opts.cleansession = 1;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        if (mqttautoconntime == 0) {
            logw(4, "Failed to connect, return code %d\n", rc);
            return -1;
        } else {
            logw(3, "Failed to connect, auto connect time : %d\n", mqttautoconntime);
        }
    }

    MQTTClient_message *m = NULL;
    char *topicName = NULL;
    int topicLen;
    rc = MQTTClient_subscribe(client, cfg.mqttsubscribe, cfg.mqttqos);
    fd = uartOpen((int8_t *)cfg.ttyport, cfg.ttyspeed, 0, cfg.ttytimeout);
    logw(1, "Good rc from subscribe rc was %d  fd was %d \n", rc, fd);
    while (1) {
        topicName = NULL;
        i ++ ;
        rc = MQTTClient_receive(client, &topicName, &topicLen, &m, 5000);
        logw(2, "rece  rc was %d  I:%d \n", rc, i);

        if (rc == -3) {
            if (cfg.mqttautoconn == 1) {
                mqttautoconntime ++;
                if (mqttautoconntime > cfg.mqttautoconnmaxtime) {
                    break;
                }

                MQTTClient_disconnect(client, 10000);
                MQTTClient_destroy(&client);
                sleep(cfg.mqttautoconninteval);
                goto start;

            } else {
                break;

            }

        } else {
            mqttautoconntime = 0;
        }

        if (topicName) {
            if (m->payloadlen > 1024) {
                pubmsg.payload = "out len";
                pubmsg.payloadlen = 7;
                goto mqttsend;
            } else {
                strncpy((char *)write_485data, (const char *)(m->payload), m->payloadlen);
                write_485data[m->payloadlen] = 0;
                logw(1, "Message received on topic %s ,len:%d,%s\n", topicName, m->payloadlen, write_485data);
            }
            if (!strcmp(cfg.ttytype, "str")) {
                remove_blank1((char *)write_485data, m->payloadlen);
                nbytes = strlen((char *)write_485data);
                if (nbytes % 2) {
                    pubmsg.payload = "data format error";
                    pubmsg.payloadlen = 17;
                    logw(3, "date len err %d\n", nbytes);
                    goto mqttsend;
                }
                gl_str2acsll((char *)write_485data, nbytes, write_buff);
                nbytes = nbytes / 2;
            } else {
                nbytes = m->payloadlen;
                strncpy((char *)write_buff, (const char *)write_485data, nbytes);
            }

            tty_write(fd, write_buff, nbytes);

            count = 0;
            ret = 8;
            while (ret == 8) {
                ret = MyuartRxExpires(fd, 200, &rec_buff[0 + count], cfg.ttytimeout);
                count += ret;
            }
            logw(1, "count:%d ret:%d\n", count, ret);


            if (count > 0) {
                char alldata[1024] = {0};
                if (!strcmp(cfg.ttytype, "str")) {
                    gl_hex2str(rec_buff, count, alldata);
                    //  gjson_add_string(rs485_data,"alldata",alldata);
                    pubmsg.payload = &alldata;
                    pubmsg.payloadlen = strlen(alldata);
                } else {
                    pubmsg.payload = &rec_buff;
                    pubmsg.payloadlen = count;
                }
            } else {
                pubmsg.payload = "no data";
                pubmsg.payloadlen = 7;
            }
mqttsend:
            pubmsg.qos = cfg.mqttqos;
            pubmsg.retained = 0;
            //			topicName += 38;
            //			sprintf(cfg.mqttpublish,"%s%s","device/e4956e40b63b/rpc_x300b/response/",topicName);

            rc = MQTTClient_publishMessage(client, cfg.mqttpublish, &pubmsg, &token);
            logw(1, "rece  rc1 was %d  I:%d \n", rc, i);
            logw(1, "Waiting for up to %d seconds for publication of %s\n"
                 "on topic %s for client with ClientID: %s\n", token, cfg.mqtttimeout / 1000, pubmsg.payload, cfg.mqttpublish, cfg.mqttclientid);
            rc = MQTTClient_waitForCompletion(client, token, cfg.mqtttimeout);
            logw(1, "rece  rc3 was %d  I:%d \n", rc, i);
            logw(1, "Message with delivery token %d delivered\n", token);
        }
    }

    usleep(50000);
    MyuartClose(fd);
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}

