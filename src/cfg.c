
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
#ifdef LOG
  cfg.dbglvl = 2;
  strncpy(cfg.logname, LOGNAME, INTBUFSIZE);
#endif
	struct uci_context* ctx = guci2_init();

	char device[16] = {0};
	char speed[16] = {0};
	char mode[16] = {0};
	char timeout[16] = {0};
	char mode_s[16] = {0};
	char timeout_s[16] = {0};
	char port[16] = {0};
	char addr[16] = {0};

	guci2_get(ctx, "rs485.rs485.device", device);
	guci2_get(ctx, "rs485.rs485.speed", speed);
	guci2_get(ctx, "rs485.rs485.mode", mode);
	guci2_get(ctx, "rs485.rs485.timeout", timeout);

	guci2_get(ctx, "rs485.socket.timeout", timeout_s);
	guci2_get(ctx, "rs485.socket.mode", mode_s);
	guci2_get(ctx, "rs485.socket.port", port);
	guci2_get(ctx, "rs485.socket.address", addr);

  	strncpy(cfg.ttyport, device, sizeof(device));
  	strncpy(cfg.ttymode, mode, sizeof(mode));

  	strncpy(cfg.serveraddr, addr, sizeof(addr));
  	strncpy(cfg.connmode, mode_s, sizeof(mode_s));

	cfg.ttyspeed = strToNumber(speed);
	cfg.ttytimeout = strToNumber(timeout);

	cfg.conntimeout = strToNumber(timeout_s);
	cfg.serverport  = strToNumber(port);

	guci2_free(ctx);

#ifdef TRXCTL
  cfg.trxcntl = TRX_ADDC;
  *cfg.trxcntl_file = '\0';
#endif
  cfg.maxconn = DEFAULT_MAXCONN;
}


