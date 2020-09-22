#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/file.h>
#include <json-c/json.h>
#include <gl/gjson.h>
#include <gl/guci2.h>
#include <gl/err_code.h>
#include <sys/stat.h>
#include <dirent.h>
#include <gl/debug.h>
#include <gl/router.h>
#include <gl/shell.h>
#include "cfg.h"

/***
 * @api {get} /rs485/attr/get /rs485/attr/get
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription get rs485 attributes.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */
int get_rs485_attr(json_object * input, json_object * output)
{
	struct uci_context* ctx = guci2_init();
	char device[16] = {0};
	char speed[16] = {0};
	char mode[16] = {0};
	char timeout[16] = {0};
        guci2_get(ctx, "rs485.rs485.device", device);
        guci2_get(ctx, "rs485.rs485.speed", speed);
        guci2_get(ctx, "rs485.rs485.mode", mode);
        guci2_get(ctx, "rs485.rs485.timeout", timeout);

	guci2_free(ctx);

	json_object *rs485_config = json_object_new_object();
	gjson_add_string(rs485_config, "device", device);
	gjson_add_string(rs485_config, "speed", speed);
	gjson_add_string(rs485_config, "mode", mode);
	gjson_add_string(rs485_config, "timeout", timeout);


	gjson_add_object(output, "rs485_config",rs485_config);
	
	return 0;
}

/***
 * @api {post} /rs485/attr/set /rs485/attr/set
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription set rs485 attributes.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */

int set_rs485_attr(json_object * input, json_object * output)
{
	char *speed = gjson_get_string(input, "speed");
	char *device = gjson_get_string(input, "device");
	char *mode = gjson_get_string(input, "mode");
	char *timeout = gjson_get_string(input, "timeout");
	
	struct uci_context* ctx = guci2_init();	
	guci2_set(ctx, "rs485.rs485.device", device);
	guci2_set(ctx, "rs485.rs485.speed",  speed);
	guci2_set(ctx, "rs485.rs485.mode",   mode);
	guci2_set(ctx, "rs485.rs485.timeout",  timeout);
	guci2_commit(ctx, "rs485");

	guci2_free(ctx);

	return 0;
}


char  remove_blank(char *str)
{
    int i=0, is = -1, in = 0;
    for (i=0;str[i] != '\0';++i){
        if (str[i] != ' '){
            if (is != -1){
                    str[is] = str[i];
                    str[i] = ' ';
                if (in == 1){
                    is = i;
                } else {
                    ++is;
                }
            }
        }
	else if (is == -1){
            is = i;
            ++in;
        } else {
            ++in;
        }
    }
    if (is != -1){
        str[is] = '\0';
    }
    return 0;
}
char  remove_blank1(char *str,int count)
{
    int i=0, is = -1, in = 0;
    for (i=0;i<count;++i){
        if (str[i] != ' '){
            if (is != -1){
                    str[is] = str[i];
                    str[i] = ' ';
                if (in == 1){
                    is = i;
                } else {
                    ++is;
                }
            }
        }
        else if (is == -1){
            is = i;
            ++in;
        } else {
            ++in;
        }
    }
    if (is != -1){
        str[is] = '\0';
    }
    return 0;
}
unsigned char my_hex_str_to_i(char *s)
{
	unsigned char i,tmp=0,n;
	int m = 0;

	m=strlen(s);
	for(i=0;i<m;i++)
	{
		if(s[i]>='A'&&s[i]<='F')
			n=s[i]-'A'+10;
		else if(s[i]>='a'&&s[i]<='f')
			n=s[i]-'a'+10;
		else n=s[i]-'0';

		tmp=tmp*16+n;
	}
	return tmp;
}

unsigned char my_hex_str_to_i_l(char *s,unsigned char len,unsigned char offset)
{
        unsigned char i,m,tmp=0,n;

        for(i=0;i<len;i++)
        {
                if(s[i+offset]>='A'&&s[i+offset]<='F')
                        n=s[i+offset]-'A'+10;
                else if(s[i+offset]>='a'&&s[i+offset]<='f')
                        n=s[i+offset]-'a'+10;
                else n=s[i+offset]-'0';

                tmp=tmp*16+n;
        }
        return tmp;
}
/***
 * @api {post} /rs485/ele_meter_vol/get  /rs485/ele_meter_vol/get
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription get rs485 Electric meter Voltage.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */


int get_rs485_ele_meter_vol(json_object * input, json_object * output)
{

        int reg_len = 0;
        int fd  = -1;
        int i  =  0;
        int ret = 8;
        int count = 0;
        int time_out = 0;
        unsigned int f_data = 0;
        unsigned short crc_way = 0xa001;//modbus
        unsigned short crc_v = 0;
        char *device_id = gjson_get_string(input, "device_id");
        unsigned char read_cmd[8] = {0,4,0,0,0,2,0,0};
        unsigned char rec_buff[20] = {0};
        unsigned char fldatastr[40] = {0};
        read_cmd[0] = my_hex_str_to_i(device_id);

        crc_v=gl_crc16 (read_cmd, 6,0xa001);
        read_cmd[6]=crc_v%256;
        read_cmd[7]=crc_v/256;

        cfg_init();

        fd = uartOpen(cfg.ttyport,cfg.ttyspeed,0,cfg.ttytimeout);
        if(fd < 0 )
        {
                return -1;
        }

        while( flock(fd,LOCK_EX|LOCK_NB) != 0 ){//get file lock
                if( ++time_out > 30 ){//time out
                        MyuartClose(fd);
                        return -2;
                }
                usleep(1000);
        }
        MyflushIoBuffer(fd);
        MyuartTxNonBlocking(fd,8,read_cmd);
        while(ret==8){
                ret = MyuartRxExpires(fd,200,&rec_buff[0+count],cfg.ttytimeout);
                count +=ret;
        }
        MyuartClose(fd);

        reg_len = read_cmd[4]*256 + read_cmd[5];
        if((count>0)&&(reg_len>0)){
                f_data = rec_buff[3]*(1<<24)+rec_buff[4]*(1<<16)+rec_buff[5]*256+rec_buff[6];
                gjson_add_double(output, "vol",(*(float*)(&f_data)));

                crc_v=gl_crc16 (rec_buff, count-2,crc_way);
                if(crc_v==(rec_buff[count-2]+rec_buff[count-1]*256)){
                        gjson_add_boolean(output,"crc_ok",1);

                }
                else{
                        gjson_add_boolean(output,"crc_ok",0);

                }

        }
        else{
                gjson_add_string(output,"error","read fail");
        }
        return 0;
}


/***
 * @api {post} /rs485/temp_humi/get  /rs485/temp_humi/get
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription get rs485  Temperature and humidity.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */

int get_rs485_temp_humi(json_object * input, json_object * output)
{

      	int reg_len = 0;
        int fd  = -1;
        int ret = 8;
        int count = 0;
        int time_out = 0;	
	int t_data = 0;
        unsigned short crc_way = 0xa001;//modbus
        unsigned short crc_v = 0;
        char *device_id = gjson_get_string(input, "device_id");
	unsigned char read_cmd[8] = {0,3,0,0,0,2,0,0};
	unsigned char rec_buff[20] = {0};
	read_cmd[0] = my_hex_str_to_i(device_id);

        crc_v=gl_crc16 (read_cmd, 6,0xa001);
        read_cmd[6]=crc_v%256;
        read_cmd[7]=crc_v/256;

        cfg_init();

        fd = uartOpen(cfg.ttyport,cfg.ttyspeed,0,cfg.ttytimeout);
        if(fd < 0 )
        {
                return -1;
        }

        while( flock(fd,LOCK_EX|LOCK_NB) != 0 ){//get file lock
                if( ++time_out > 30 ){//time out
                        MyuartClose(fd);
                        return -2;
                }
                usleep(1000);
        }
        MyflushIoBuffer(fd);
        MyuartTxNonBlocking(fd,8,read_cmd);
        while(ret==8){
                ret = MyuartRxExpires(fd,200,&rec_buff[0+count],cfg.ttytimeout);
                count +=ret;
        }
        MyuartClose(fd);

	reg_len = read_cmd[4]*256 + read_cmd[5];
	if((count>0)&&(reg_len>0)){
		t_data = rec_buff[3]*256+rec_buff[4];
		if(t_data>0x7fff){
			t_data = t_data - 0xffff;
		}
		gjson_add_double(output, "tempdata",(t_data/10.0));
		gjson_add_double(output, "humidata",(rec_buff[5]*256+rec_buff[6])/10.0);

		crc_v=gl_crc16 (rec_buff, count-2,crc_way);
                if(crc_v==(rec_buff[count-2]+rec_buff[count-1]*256)){
                        gjson_add_boolean(output,"crc_ok",1);

                }
                else{
                        gjson_add_boolean(output,"crc_ok",0);

                }

        }
        else{
                gjson_add_string(output,"error","read fail");
        }
        return 0;
}

/***
 * @api {post} /rs485/temp_humi_id/set  /rs485/temp_humi_id/set
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription set rs485  Temperature and humidity ID.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */

int modify_rs485_temp_humi_id(json_object * input, json_object * output)
{
	int reg_len = 0;
        int fd  = -1;
        int ret = 8;
        int count = 0;
        int time_out = 0;
	int i =0;
        unsigned short crc_way = 0xa001;//modbus
        unsigned short crc_v = 0;

	unsigned char read_cmd[16] = {0,0x10,0,2,0,1,2,0,0,0,0};
	unsigned char rec_buff[20] = {0};
	char *device_id = gjson_get_string(input, "device_id");
	char *device_id_m = gjson_get_string(input, "device_id_m");
	read_cmd[0] = my_hex_str_to_i(device_id);
	read_cmd[1] = 0x10;
	read_cmd[8] = my_hex_str_to_i(device_id_m);
        crc_v=gl_crc16 (read_cmd, 9,0xa001);
        read_cmd[9]=crc_v%256;
        read_cmd[10]=crc_v/256;

        cfg_init();

        fd = uartOpen(cfg.ttyport,cfg.ttyspeed,0,cfg.ttytimeout);
        if(fd < 0 )
        {
                return -1;
        }

        while( flock(fd,LOCK_EX|LOCK_NB) != 0 ){//get file lock
                if( ++time_out > 30 ){//time out
                        MyuartClose(fd);
                        return -2;
                }
                usleep(1000);
        }
        MyflushIoBuffer(fd);
        MyuartTxNonBlocking(fd,11,read_cmd);
        while(ret==8){
                ret = MyuartRxExpires(fd,200,&rec_buff[0+count],cfg.ttytimeout);
                count +=ret;
        }
        MyuartClose(fd);

        reg_len = read_cmd[4]*256 + read_cmd[5];
        if((count>0)&&(reg_len>0)){

                char alldata[10] = {0};
                char alldata1[100] = {0};
		memset(alldata1,0,100);
                for(i=0;i<count;i++){
			memset(alldata,0,10);
                        sprintf(alldata,"%02x ",rec_buff[i]);
                        strcat(alldata1,alldata);
                }
                gjson_add_string(output,"alldata",alldata1);

		if(rec_buff[0]==read_cmd[8]){
                        gjson_add_boolean(output,"modify_ok",1);
	
		}
		else{
                        gjson_add_boolean(output,"modify_ok",1);

		}

                crc_v=gl_crc16 (rec_buff, count-2,crc_way);
                if(crc_v==(rec_buff[count-2]+rec_buff[count-1]*256)){
                        gjson_add_boolean(output,"crc_ok",1);

                }
                else{
                        gjson_add_boolean(output,"crc_ok",0);

                }

        }
        else{
                gjson_add_string(output,"error","read fail");
        }
	return 0;
}
/***
 * @api {post} /rs485/data/read /rs485/data/read
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription read rs485 data.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */
int read_rs485_data(json_object * input, json_object * output)
{
char i = 0;
	unsigned char read_cmd[10] = {0};
	unsigned char rec_buff[270] = {0};
	unsigned short crc_way = 0xa001;//modbus
	unsigned short crc_v = 0;
	ttydata_t rs485;
	int reg_len = 0;
	int fd  = -1;
	int ret = 8;
	int count = 0;
	int time_out = 0;
	char *device_id = gjson_get_string(input, "device_id");
	char *func_code = gjson_get_string(input, "func_code");
	char *reg_addr_h = gjson_get_string(input, "reg_addr_h");
	char *reg_addr_l = gjson_get_string(input, "reg_addr_l");
	char *reg_len_l = gjson_get_string(input, "reg_len");//00-7f
//	char *crc16 = gjson_get_string(input, "crc16");
//	crc_way= my_hex_str_to_i_l(crc16,2,0)*256+my_hex_str_to_i_l(crc16,2,2);

	read_cmd[0] = my_hex_str_to_i(device_id);
	read_cmd[1] = my_hex_str_to_i(func_code);
	read_cmd[2] = my_hex_str_to_i(reg_addr_h);
	read_cmd[3] = my_hex_str_to_i(reg_addr_l);
	read_cmd[4] = 0;
	read_cmd[5] = my_hex_str_to_i(reg_len_l);

	crc_v=gl_crc16 (read_cmd, 6,crc_way);
	read_cmd[6]=crc_v%256;
	read_cmd[7]=crc_v/256;
	
//	cfg_init();
//	tty_init(&rs485);
//	tty_open(&rs485);
//	write(rs485.fd,read_cmd,8,1);
//	tty_close(&rs485);

	cfg_init();
	
	fd = uartOpen(cfg.ttyport,cfg.ttyspeed,0,cfg.ttytimeout);
	if(fd < 0 )
	{
		return -1;
	}
	
	while( flock(fd,LOCK_EX|LOCK_NB) != 0 ){//get file lock
		if( ++time_out > 30 ){//time out
			MyuartClose(fd);
			return -2;
		}
		usleep(1000);
	}
	MyflushIoBuffer(fd);
	MyuartTxNonBlocking(fd,8,read_cmd);
	while(ret==8){
		ret = MyuartRxExpires(fd,200,&rec_buff[0+count],cfg.ttytimeout);
		count +=ret;
	}
	MyuartClose(fd);

	json_object *rs485_data = json_object_new_object();

	reg_len = read_cmd[5];
	if((count>0)&&(reg_len>0)){
		char alldata[10] = {0};
		char alldata1[1024] = {0};
		memset(alldata1,0,1024);
		for(i=0;i<count;i++){
			memset(alldata,0,10);
			sprintf(alldata,"%02x ",rec_buff[i]);
			strcat(alldata1,alldata);
		}
		gjson_add_string(rs485_data,"alldata",alldata1);

		crc_v=gl_crc16 (rec_buff, count-2,crc_way);
		if(crc_v==(rec_buff[count-2]+rec_buff[count-1]*256)){
			gjson_add_boolean(rs485_data,"crc_ok",1);

		}
		else{
			gjson_add_boolean(rs485_data,"crc_ok",0);

		}

	}
	else{
		gjson_add_string(rs485_data,"error","read fail");
	}
	gjson_add_object(output, "rs485_data",rs485_data);

	return 0;
}

/***
 * @api {post} /rs485/data/write /rs485/data/write
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription write rs485 data.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */
int write_rs485_data(json_object * input, json_object * output)
{
	int i = 0;
        int fd  = -1;
        int ret = 8;
        int count = 0;
        int time_out = 0;
	int write_len = 0;
	int data_str_len = 0;
        unsigned short crc_way = 0xa001;//modbus
        unsigned short crc_v = 0;
        unsigned char write_cmd[272] = {0};
        unsigned char rec_buff[272] = {0};
        char *device_id = gjson_get_string(input, "device_id");
        char *func_code = gjson_get_string(input, "func_code");
        char *reg_addr_h = gjson_get_string(input, "reg_addr_h");
        char *reg_addr_l = gjson_get_string(input, "reg_addr_l");
        char *reg_len_l = gjson_get_string(input, "reg_len");
        char *data_count = gjson_get_string(input, "data_count");

        char *data = gjson_get_string(input, "data");


        write_cmd[0] = my_hex_str_to_i(device_id);
        write_cmd[1] = my_hex_str_to_i(func_code);
        write_cmd[2] = my_hex_str_to_i(reg_addr_h);
        write_cmd[3] = my_hex_str_to_i(reg_addr_l);
        write_cmd[4] = 0 ;
        write_cmd[5] = my_hex_str_to_i(reg_len_l);//00-7f
        write_cmd[6] = my_hex_str_to_i(data_count);//00-ff

	write_len =7 + write_cmd[6];	

	data_str_len=strlen(data);

	if(data_str_len>512){
		gjson_add_string(output,"error","data len fail");
		return 0;
	}

	if(((data_str_len%2)==1)||((write_cmd[6]*2)!= data_str_len)){
		gjson_add_string(output,"error","data format fail");
		return 0;
	}
	for(i=0;i<write_len;i++){
		write_cmd[7+i] = my_hex_str_to_i_l(data,2,2*i);
	}

        crc_v=gl_crc16 (write_cmd, write_len,0xa001);
        write_cmd[write_len]=crc_v%256;
        write_cmd[write_len+1]=crc_v/256;

        cfg_init();
        fd = uartOpen(cfg.ttyport,cfg.ttyspeed,0,cfg.ttytimeout);
        if(fd < 0 )
        {
                return -1;
        }

        while( flock(fd,LOCK_EX|LOCK_NB) != 0 ){//get file lock
                if( ++time_out > 30 ){//time out
                        MyuartClose(fd);
                        return -2;
                }
                usleep(1000);
        }
        MyflushIoBuffer(fd);
        MyuartTxNonBlocking(fd,2+write_len,write_cmd);
        while(ret==8){
                ret = MyuartRxExpires(fd,200,&rec_buff[0+count],cfg.ttytimeout);
                count +=ret;
        }
        MyuartClose(fd);

	json_object *rs485_data = json_object_new_object();

        if((count>0)&&(write_len>0)){
                char alldata[10] = {0};
                char alldata1[1024] = {0};
		memset(alldata1,0,1024);
                for(i=0;i<count;i++){
			memset(alldata,0,10);
                        sprintf(alldata,"%02x ",rec_buff[i]);
                        strcat(alldata1,alldata);
                }
                gjson_add_string(rs485_data,"alldata",alldata1);

                crc_v=gl_crc16 (rec_buff, count-2,crc_way);
                if(crc_v==(rec_buff[count-2]+rec_buff[count-1]*256)){
                        gjson_add_boolean(rs485_data,"crc_ok",1);

                }
                else{
                        gjson_add_boolean(rs485_data,"crc_ok",0);

                }
        }
        else{
                gjson_add_string(rs485_data,"error","read fail");
        }
        gjson_add_object(output, "rs485_data",rs485_data);

        return 0;
}

/***
 * @api {get} /rs485/dlt645/contact_get /rs485/dlt645/contact_get
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription read  dlt645 contact addr.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */
int get_dlt645_contact_addr(json_object * input, json_object * output)
{
	int i = 0;
        int fd  = -1;
        int ret = 8;
        int count = 0;
        int time_out = 0;
	unsigned char data_cs = 0;

	unsigned char read_cmd[12] = {0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x13,0x00,0xDF,0x16};
	unsigned char rec_buff[50] = {0};
        cfg_init();

        fd = uartOpen(cfg.ttyport,cfg.ttyspeed,0,cfg.ttytimeout);
        if(fd < 0 )
        {
                return -1;
        }

        while( flock(fd,LOCK_EX|LOCK_NB) != 0 ){//get file lock
                if( ++time_out > 30 ){//time out
                        MyuartClose(fd);
                        return -2;
                }
                usleep(1000);
        }
        MyflushIoBuffer(fd);
        MyuartTxNonBlocking(fd,12,read_cmd);
        while(ret==8){
                ret = MyuartRxExpires(fd,200,&rec_buff[0+count],cfg.ttytimeout);
                count +=ret;
        }
        MyuartClose(fd);

        json_object *rs485_data = json_object_new_object();

        if(count>0){
                char alldata[10] = {0};
                char addr_data[20] = {0};
                char alldata1[1024] = {0};
		memset(alldata1,0,1024);
                for(i=0;i<count;i++){
			memset(alldata,0,10);
                        sprintf(alldata,"%02x ",rec_buff[i]);
                        strcat(alldata1,alldata);
                }
                for(i=1;i<7;i++){
                        sprintf(alldata,"%02x",rec_buff[i]);
                        strcat(addr_data,alldata);
                }
                gjson_add_string(rs485_data,"alldata",alldata1);
                gjson_add_string(rs485_data,"addr_data",addr_data);
		
                for(i=0;i<count-2;i++){
			data_cs += rec_buff[i];
                }


                if(data_cs==(rec_buff[count-2])){
                        gjson_add_boolean(rs485_data,"cs_ok",1);
                }
                else{
                       gjson_add_boolean(rs485_data,"cs_ok",0);

                }

        }
        else{
                gjson_add_string(rs485_data,"error","read fail");
        }
        gjson_add_object(output, "rs485_data",rs485_data);

        return 0;

}

/***
 * @api {post} /rs485/dlt645/data_read /rs485/dlt645/data_read
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription read  dlt645 data.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */
int read_dlt645_data(json_object * input, json_object * output)
{
        int i = 0;
        int fd  = -1;
        int ret = 8;
        int count = 0;
        int time_out = 0;
        int datalen = 0;
        unsigned char data_cs = 0;

        unsigned char read_cmd[20] = {0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0};
        unsigned char rec_buff[50] = {0};
	
	char *con_addr = gjson_get_string(input, "con_addr");
	char *ctl_code = gjson_get_string(input, "ctl_code");
	char *data_len = gjson_get_string(input, "data_len");


        for(i=0;i<6;i++){
                read_cmd[1+i] = my_hex_str_to_i_l(con_addr,2,2*i);
        }
	read_cmd[8] = my_hex_str_to_i(ctl_code);
	datalen = read_cmd[9] = my_hex_str_to_i(data_len);

	if(datalen > 0){
		char *data_iden = gjson_get_string(input, "data_iden");
		for(i=0;i<4;i++){
			read_cmd[10+i] = my_hex_str_to_i_l(data_iden,2,2*i);
		}
	}

	if(datalen > 4){
		char *data = gjson_get_string(input, "data");
		for(i=0;i<datalen-4;i++){
			read_cmd[14+i] = my_hex_str_to_i_l(data,2,2*i);
		}
	}

	for(i=0;i<datalen + 10;i++){
		read_cmd[10+datalen] += read_cmd[i];
	}

	read_cmd[11+datalen]  = 0x16;

        cfg_init();

        fd = uartOpen(cfg.ttyport,cfg.ttyspeed,0,cfg.ttytimeout);
        if(fd < 0 )
        {
                return -1;
        }

        while( flock(fd,LOCK_EX|LOCK_NB) != 0 ){//get file lock
                if( ++time_out > 30 ){//time out
                        MyuartClose(fd);
                        return -2;
                }
                usleep(1000);
        }
        MyflushIoBuffer(fd);
        MyuartTxNonBlocking(fd,12+datalen,read_cmd);
        while(ret==8){
                ret = MyuartRxExpires(fd,200,&rec_buff[0+count],cfg.ttytimeout);
                count +=ret;
        }
        MyuartClose(fd);

        json_object *rs485_data = json_object_new_object();

        if(count>0){
                char rec_ctl_code[4] = {0};
                char rec_data_len[4] = {0};
                char rec_data_iden[10] = {0};
                char alldata[10] = {0};
                char rec_data[40] = {0};
                char addr_data[20] = {0};
                char alldata1[1024] = {0};
		memset(alldata1,0,1024);
                for(i=0;i<count;i++){
			memset(alldata,0,10);
                        sprintf(alldata,"%02x ",rec_buff[i]);
                        strcat(alldata1,alldata);
                }
                for(i=1;i<7;i++){
			memset(alldata,0,10);
                        sprintf(alldata,"%02x",rec_buff[i]);
                        strcat(addr_data,alldata);
                }

		sprintf(rec_ctl_code,"%02x",rec_buff[8]);
		sprintf(rec_data_len,"%02x",rec_buff[9]);


                gjson_add_string(rs485_data,"alldata",alldata1);
                gjson_add_string(rs485_data,"con_addr",addr_data);
                gjson_add_string(rs485_data,"ctl_code",rec_ctl_code);
                gjson_add_string(rs485_data,"data_len",rec_data_len);

                for(i=0;i<count-2;i++){
                        data_cs += rec_buff[i];
                }

		datalen = rec_buff[9];		
		if(datalen > 0){
			for(i=0;i<4;i++){
				rec_buff[13-i] -=0x33;
				sprintf(alldata,"%02x",rec_buff[13-i]);
				strcat(rec_data_iden,alldata);
			}
			gjson_add_string(rs485_data,"data_iden",rec_data_iden);
		}

		if(datalen > 4){
			for(i=0;i<datalen-4;i++){
				rec_buff[datalen+9-i] -=0x33;
				sprintf(alldata,"%02x",rec_buff[datalen+9-i]);
				strcat(rec_data,alldata);
			}
			gjson_add_string(rs485_data,"data",rec_data);
		}

                if(data_cs==(rec_buff[count-2])){
                        gjson_add_boolean(rs485_data,"cs_ok",1);
                }
                else{
                       gjson_add_boolean(rs485_data,"cs_ok",0);

                }

        }
        else{
                gjson_add_string(rs485_data,"error","read fail");
        }
        gjson_add_object(output, "rs485_data",rs485_data);

        return 0;

}

/***
 * @api {post} /rs485/terminal/send_read /rs485/terminal/send_read
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription  termiinal send read .
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */
int terminal_send_read(json_object * input, json_object * output)
{
	char *str_hex = gjson_get_string(input, "show_type");
	char s_send = gjson_get_boolean(input, "show_send");
	char s_time = gjson_get_boolean(input, "show_date");
	char *s_data = gjson_get_string(input, "data");


        int fd = 0;
        int ret = 8;
        int count = 0;
        int i = 0;
        int nbytes = 0;

	json_object *rs485_data = json_object_new_object();

	if(s_send){
                gjson_add_string(rs485_data,"send",s_data);
	}

	cfg_init();
        fd = uartOpen(cfg.ttyport,cfg.ttyspeed,0,cfg.ttytimeout);


        if(!strcmp(str_hex,"hex")){
		remove_blank(s_data);
		nbytes = strlen(s_data);
                if(nbytes%2){
                        printf("date len err\n");
                        return -1;
                }
                for(i=0;i<nbytes/2;i++){
                        s_data[i] = my_hex_str_to_i_l(s_data,2,2*i);;
                }
                tty_write(fd,s_data,nbytes/2);
        }
        else{
		nbytes = strlen(s_data);
                tty_write(fd,s_data,nbytes);
        }
        unsigned  char rec_buff[1024] = {0};
        while(ret==8){
                ret = MyuartRxExpires(fd,200,&rec_buff[0+count],cfg.ttytimeout);
                count +=ret;
        }

        usleep(1000);
        MyuartClose(fd);

        char alldata[10] = {0};
        char alldata1[1024] = {0};
	memset(alldata1,0,1024);
        if(!strcmp(str_hex,"hex")){
		for(i=0;i<count;i++){
			memset(alldata,0,10);
			sprintf(alldata,"%02x",rec_buff[i]);
			strcat(alldata1,alldata);
		}
		gjson_add_string(rs485_data,"alldata",alldata1);
        }
        else{
                printf("rec:%s\n",rec_buff);
                gjson_add_string(rs485_data,"alldata",rec_buff);
        }
	if(s_time){
		char *date_rc =  getShellCommandReturnDynamic("date '+%Y-%m-%d %H:%M:%S'");
                gjson_add_string(rs485_data,"date",date_rc);
	}
        gjson_add_object(output, "rs485_data",rs485_data);

	return 0;

}


/***
 * @api {get} /rs485/mqtt/get  /rs485/mqtt/get
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription get rs485-mrtt config.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */
int get_mqtt_config(json_object * input, json_object * output)
{
        struct uci_context* ctx = guci2_init();

        char port_m[16] = {0};
        char addr_m[48] = {0};
        char timeout_m[16] = {0};
        char qos_m[16] = {0};
        char autoconn_m[16] = {0};
        char autoconnmaxtime_m[16] = {0};
        char autoconninteval_m[16] = {0};
        char interval_m[16] = {0};
        char username_m[32] = {0};
        char password_m[32] = {0};
        char clientid_m[32] = {0};
        char publish_m[32] = {0};
        char subscribe_m[32] = {0};

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


        guci2_free(ctx);

        json_object *rs485_mqtt_config = json_object_new_object();

        gjson_add_string(rs485_mqtt_config, "port", port_m);
        gjson_add_string(rs485_mqtt_config, "address", addr_m);
        gjson_add_string(rs485_mqtt_config, "timeout", timeout_m);
        gjson_add_string(rs485_mqtt_config, "interval", interval_m);
        gjson_add_string(rs485_mqtt_config, "username", username_m);
        gjson_add_string(rs485_mqtt_config, "password", password_m);
        gjson_add_string(rs485_mqtt_config, "qos", qos_m);
        gjson_add_string(rs485_mqtt_config, "clientid", clientid_m);
        gjson_add_string(rs485_mqtt_config, "publish", publish_m);
        gjson_add_string(rs485_mqtt_config, "subscribe", subscribe_m);
        gjson_add_string(rs485_mqtt_config, "autoconn", autoconn_m);
        gjson_add_string(rs485_mqtt_config, "autoconninteval",autoconninteval_m);
        gjson_add_string(rs485_mqtt_config, "autoconnmaxtime", autoconnmaxtime_m);

        gjson_add_object(output, "rs485_mqtt_config",rs485_mqtt_config);

        return 0;
}


/***
 * @api {post} /rs485/mqtt/set  /rs485/mqtt/set
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription set rs485_mqtt config.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */

int set_mqtt_config(json_object * input, json_object * output)
{

        char *port_m = gjson_get_string(input, "port");
        char *addr_m = gjson_get_string(input, "address");
        char *timeout_m = gjson_get_string(input, "timeout");
        char *interval_m = gjson_get_string(input, "interval");
        char *username_m = gjson_get_string(input, "username");
        char *password_m = gjson_get_string(input, "password");
        char *qos_m = gjson_get_string(input, "qos");
        char *clientid_m = gjson_get_string(input, "clientid");
        char *publish_m = gjson_get_string(input, "publish");
        char *subscribe_m = gjson_get_string(input, "subscribe");
        char *autoconn_m = gjson_get_string(input, "autoconn");
        char *autoconninteval_m = gjson_get_string(input, "autoconninteval");
        char *autoconnmaxtime_m = gjson_get_string(input, "autoconnmaxtime");

        struct uci_context* ctx = guci2_init();

        guci2_set(ctx, "rs485.mqtt.port", port_m);
        guci2_set(ctx, "rs485.mqtt.address", addr_m);
        guci2_set(ctx, "rs485.mqtt.timeout", timeout_m);
        guci2_set(ctx, "rs485.mqtt.interval", interval_m);
        guci2_set(ctx, "rs485.mqtt.username", username_m);
        guci2_set(ctx, "rs485.mqtt.password", password_m);
        guci2_set(ctx, "rs485.mqtt.qos", qos_m);
        guci2_set(ctx, "rs485.mqtt.clientid", clientid_m);
        guci2_set(ctx, "rs485.mqtt.publish", publish_m);
        guci2_set(ctx, "rs485.mqtt.subscribe", subscribe_m);
        guci2_set(ctx, "rs485.mqtt.autoconn", autoconn_m);
        guci2_set(ctx, "rs485.mqtt.autoconninteval",autoconninteval_m);
        guci2_set(ctx, "rs485.mqtt.autoconnmaxtime", autoconnmaxtime_m);


        guci2_commit(ctx, "rs485");

        guci2_free(ctx);

        return 0;
}

/***
 * @api {get}  /rs485/socket/get /rs485/socket/get
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription get rs485_socket config.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */
int get_socket_config(json_object * input, json_object * output)
{
        struct uci_context* ctx = guci2_init();
        char addr[64] = {0};
        char port[16] = {0};
        char mode_s[16] = {0};
        char timeout_s[16] = {0};

        guci2_get(ctx, "rs485.socket.timeout", timeout_s);
        guci2_get(ctx, "rs485.socket.mode", mode_s);
        guci2_get(ctx, "rs485.socket.port", port);
        guci2_get(ctx, "rs485.socket.address", addr);

        guci2_free(ctx);

        json_object *rs485_socket_config = json_object_new_object();
        gjson_add_string(rs485_socket_config, "address", addr);
        gjson_add_string(rs485_socket_config, "port", port);
        gjson_add_string(rs485_socket_config, "mode", mode_s);
        gjson_add_string(rs485_socket_config, "timeout", timeout_s);


        gjson_add_object(output, "rs485_socket_config",rs485_socket_config);

        return 0;
}

/***
 * @api {post}   /rs485/socket/set  /rs485/socket/set
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription set rs485_cocket config.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */

int set_socket_config(json_object * input, json_object * output)
{
        char *addr = gjson_get_string(input, "address");
        char *port = gjson_get_string(input, "port");
        char *mode_s = gjson_get_string(input, "mode");
        char *timeout_s = gjson_get_string(input, "timeout");

        struct uci_context* ctx = guci2_init();

        guci2_set(ctx, "rs485.socket.timeout", timeout_s);
        guci2_set(ctx, "rs485.socket.mode", mode_s);
        guci2_set(ctx, "rs485.socket.port", port);
        guci2_set(ctx, "rs485.socket.address", addr);

        guci2_commit(ctx, "rs485");

        guci2_free(ctx);

        return 0;
}


/***
 * @api {get}  /rs485/socket/start /rs485/socket/start
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription start rs485 to socket.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */
int rs485_socket_start(json_object * input, json_object * output)
{

	char *rc =  getShellCommandReturnDynamic("gl-rs485 -B  socket");

        struct uci_context* ctx = guci2_init();

        guci2_set(ctx, "rs485.socket.status", "1");

        guci2_commit(ctx, "rs485");

        guci2_free(ctx);

	if(rc==NULL){
		gjson_add_string(output,"rs485_socket_start","ok");
	}else{
		gjson_add_string(output,"rs485_socket_start","error");
	}

	return 0;
}


/***
 * @api {get}  /rs485/socket/stop /rs485/socket/stop
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription stop rs485 to socket.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */
int rs485_socket_stop(json_object * input, json_object * output)
{

	char *rc =  getShellCommandReturnDynamic("pgrep rs485 | xargs  kill -9");
	struct uci_context* ctx = guci2_init();

	guci2_set(ctx, "rs485.socket.status", "0");
	guci2_set(ctx, "rs485.mqtt.status", "0");

	guci2_commit(ctx, "rs485");

	guci2_free(ctx);

	if(rc==NULL){
		gjson_add_string(output,"rs485_stop","ok");
	}else{
		gjson_add_string(output,"rs485_stop","error");
	}
        return 0;
}


/***
 * @api {get}  /rs485/socket/status /rs485/socket/status
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription get rs485 to socket status.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */
int get_rs485_socket_status(json_object * input, json_object * output)
{
        struct uci_context* ctx = guci2_init();
        char status[16] = {0};

        guci2_get(ctx, "rs485.socket.status", status);

        guci2_free(ctx);


        gjson_add_string(output,"rs485_socket_status",status);

        return 0;
}



/***
 * @api {get}  /rs485/mqtt/start /rs485/mqtt/start
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription start rs485 to mqtt.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */
int rs485_mqtt_start(json_object * input, json_object * output)
{

	char *rc =  getShellCommandReturnDynamic("gl-rs485 -B mqtt");
        struct uci_context* ctx = guci2_init();

        guci2_set(ctx, "rs485.mqtt.status", "1");

        guci2_commit(ctx, "rs485");

        guci2_free(ctx);
	if(rc==NULL){
		gjson_add_string(output,"rs485_mqtt_start","ok");
	}else{
		gjson_add_string(output,"rs485_mqtt_start","error");
	}
        return 0;
}


/***
 * @api {get}  /rs485/mqtt/stop /rs485/mqtt/stop
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription stop rs485 to mqtt.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */
int rs485_mqtt_stop(json_object * input, json_object * output)
{
	char *rc =  getShellCommandReturnDynamic("pgrep rs485 | xargs  kill -9");

	struct uci_context* ctx = guci2_init();

	guci2_set(ctx, "rs485.socket.status", "0");
	guci2_set(ctx, "rs485.mqtt.status", "0");

	guci2_commit(ctx, "rs485");

	guci2_free(ctx);

	if(rc==NULL){
		gjson_add_string(output,"rs485_stop","ok");
	}else{
		gjson_add_string(output,"rs485_stop","error");
	}
        return 0;
}


/***
 * @api {get}  /rs485/mqtt/status /rs485/mqtt/status
 * @apiGroup rs485
 * @apiVersion 1.0.0
 * @apiDescription get rs485 to mqtt status.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */
int get_rs485_mqtt_status(json_object * input, json_object * output)
{
	struct uci_context* ctx = guci2_init();
        char status[16] = {0};

        guci2_get(ctx, "rs485.mqtt.status", status);

        guci2_free(ctx);


	gjson_add_string(output,"rs485_mqtt_status",status);
        return 0;
}


/** The implementation of the GetAPIFunctions function **/
#include <gl/glapibase.h>

static api_info_t gl_lstCgiApiFuctionInfo[] = {
//you can simply add or remove entities from here
	map("/rs485/attr/get", "get", get_rs485_attr),
	map("/rs485/attr/set", "post", set_rs485_attr),
	map("/rs485/data/read", "post", read_rs485_data),
	map("/rs485/data/write", "post", write_rs485_data),
	map("/rs485/temp_humi/get", "post", get_rs485_temp_humi),
	map("/rs485/temp_humi_id/set", "post", modify_rs485_temp_humi_id),
	map("/rs485/ele_meter_vol/get", "post", get_rs485_ele_meter_vol),
	map("/rs485/dlt645/contact_get", "get", get_dlt645_contact_addr),
	map("/rs485/dlt645/data_read", "post", read_dlt645_data),
	map("/rs485/terminal/send_read", "post", terminal_send_read),
	map("/rs485/socket/get", "get", get_socket_config),
	map("/rs485/socket/set", "post", set_socket_config),
	map("/rs485/socket/start", "get", rs485_socket_start),
	map("/rs485/socket/stop", "get", rs485_socket_stop),
	map("/rs485/socket/status", "get", get_rs485_socket_status),
	map("/rs485/mqtt/get", "get", get_mqtt_config),
	map("/rs485/mqtt/set", "post", set_mqtt_config),
	map("/rs485/mqtt/start", "get", rs485_mqtt_start),
	map("/rs485/mqtt/stop", "get", rs485_mqtt_stop),
	map("/rs485/mqtt/status", "get", get_rs485_mqtt_status),
};

api_info_t *get_api_entity(int *pLen)
{
	(*pLen) = sizeof(gl_lstCgiApiFuctionInfo) / sizeof(gl_lstCgiApiFuctionInfo[0]);
	return gl_lstCgiApiFuctionInfo;
}


