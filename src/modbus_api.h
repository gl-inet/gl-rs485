#ifndef _MODBUS_API_H
#define _MODBUS_API_H


int get_rs485_attr(json_object * input, json_object * output);
int set_rs485_attr(json_object * input, json_object * output);
char  remove_blank(char *str);
char  remove_blank1(char *str,int count);
unsigned char my_hex_str_to_i(char *s);
unsigned char my_hex_str_to_i_l(char *s,unsigned char len,unsigned char offset);

#endif
