#ifndef _MODBUS_API_H
#define _MODBUS_API_H


int get_rs485_attr(json_object *input, json_object *output);
int set_rs485_attr(json_object *input, json_object *output);
char  remove_blank(char *str);
char  remove_blank1(char *str, int count);
unsigned char my_hex_str_to_i(char *s);
unsigned char my_hex_str_to_i_l(char *s, unsigned char len, unsigned char offset);
void gl_hex2str(uint8_t *input, int input_len, char *output);
void gl_str2acsll(char  *str_in, int s_len, uint8_t  *acsll_out);
#endif

