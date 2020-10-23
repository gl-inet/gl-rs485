
#include "cfg.h"
#include <string.h>
#include <gl/gjson.h>
#include <gl/shell.h>
#include <gl/guci2.h>

cfg_t cfg;

int powTen(int n)
{
	int result=1;
	int i;
        for(i=1;i<=n;i++)
	{
		result*=10;
            
	}
        return result;
}       
int strToNumber(char *numbers)
{
	int length=strlen(numbers);
	int sum=0;
	int i;
	for(i=0;i<length;i++)
	{
		if(numbers[i]>='0'&&numbers[i]<='9')
			sum+=((int)numbers[i]-48)*powTen(length-i-1);
		else
		{
			sum=sum/10;
			break;
		
		}
	
	}
	return sum;

}
void
cfg_init(void)
{
	struct uci_context* ctx = guci2_init();

	char speed[8] = {0};
	char mode[8] = {0};
	char logv[8] = {0};
	char timeout[8] = {0};
	char mode_s[8] = {0};
	char timeout_s[8] = {0};
	char port[8] = {0};
	char port_m[8] = {0};
	char timeout_m[8] = {0};
	char qos_m[8] = {0};
	char autoconn_m[8] = {0};
	char autoconnmaxtime_m[8] = {0};
	char autoconninteval_m[8] = {0};
	char interval_m[8] = {0};
	char device[INTBUFSIZE+1] = {0};
	char addr[INTBUFSIZE+1] = {0};
	char addr_m[INTBUFSIZE+1] = {0};
	char username_m[INTBUFSIZE+1] = {0};
	char password_m[INTBUFSIZE+1] = {0};
	char clientid_m[INTBUFSIZE+1] = {0};
	char publish_m[INTBUFSIZE+1] = {0};
	char subscribe_m[INTBUFSIZE+1] = {0};

	guci2_get(ctx, "rs485.rs485.device", device);
	guci2_get(ctx, "rs485.rs485.speed", speed);
	guci2_get(ctx, "rs485.rs485.mode", mode);
	guci2_get(ctx, "rs485.rs485.timeout", timeout);
	guci2_get(ctx, "rs485.rs485.log", logv);

	guci2_get(ctx, "rs485.socket.timeout", timeout_s);
	guci2_get(ctx, "rs485.socket.mode", mode_s);
	guci2_get(ctx, "rs485.socket.port", port);
	guci2_get(ctx, "rs485.socket.address", addr);

	guci2_get(ctx, "rs485.mqtt.port", port_m);
	guci2_get(ctx, "rs485.mqtt.address", addr_m);
	guci2_get(ctx, "rs485.mqtt.timeout", timeout_m);
	guci2_get(ctx, "rs485.mqtt.interval", interval_m);
	guci2_get(ctx, "rs485.mqtt.username", username_m);
	guci2_get(ctx, "rs485.mqtt.password", password_m);
	guci2_get(ctx, "rs485.mqtt.qos", qos_m);
	guci2_get(ctx, "rs485.mqtt.clientid", clientid_m);
	guci2_get(ctx, "rs485.mqtt.publish", publish_m);
	guci2_get(ctx, "rs485.mqtt.subscribe", subscribe_m);
	guci2_get(ctx, "rs485.mqtt.autoconn", autoconn_m);
	guci2_get(ctx, "rs485.mqtt.autoconninteval",autoconninteval_m);
	guci2_get(ctx, "rs485.mqtt.autoconnmaxtime", autoconnmaxtime_m);

  	strncpy(cfg.ttyport, device, sizeof(device));
	strncpy(cfg.logname, LOGNAME, INTBUFSIZE);

  	strncpy(cfg.ttymode, mode, sizeof(mode));

  	strncpy(cfg.serveraddr, addr, sizeof(addr));
  	strncpy(cfg.connmode, mode_s, sizeof(mode_s));
	
	sprintf(cfg.mqttaddr,"tcp://%s:%s",addr_m,port_m);
  	strncpy(cfg.mqttusername, username_m, sizeof(username_m));
  	strncpy(cfg.mqttpassword, password_m, sizeof(password_m));
  	strncpy(cfg.mqttclientid, clientid_m, sizeof(clientid_m));
  	strncpy(cfg.mqttpublish, publish_m, sizeof(publish_m));
  	strncpy(cfg.mqttsubscribe, subscribe_m, sizeof(subscribe_m));

	cfg.ttyspeed = strToNumber(speed);
	if(cfg.ttyspeed>115200){
		cfg.ttyspeed =115200;
	}
	cfg.ttytimeout = strToNumber(timeout);
	cfg.dbglvl = strToNumber(logv);
	if(cfg.dbglvl>5){
		cfg.dbglvl =5;
	}

	cfg.conntimeout = strToNumber(timeout_s);
	cfg.serverport  = strToNumber(port);

	cfg.mqttinterval  = strToNumber(interval_m);
	cfg.mqtttimeout  = strToNumber(timeout_m);
	cfg.mqttqos  = strToNumber(qos_m);
	cfg.mqttautoconn  = strToNumber(autoconn_m);
	cfg.mqttautoconnmaxtime  = strToNumber(autoconnmaxtime_m);
	cfg.mqttautoconninteval  = strToNumber(autoconninteval_m);

	guci2_free(ctx);

#ifdef TRXCTL
  cfg.trxcntl = TRX_ADDC;
  *cfg.trxcntl_file = '\0';
#endif
  cfg.maxconn = DEFAULT_MAXCONN;
}


