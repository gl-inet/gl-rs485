#include "crc16.h"

unsigned short gl_crc16 ( unsigned char *arr_buff, unsigned char len,unsigned short crc_v )
{
    unsigned short crc=0xFFFF;
    unsigned char i, j;
    for ( j=0; j <len; j++ ) {
        crc=crc ^*arr_buff++;
        for ( i=0; i<8; i++ ) {
            if( ( crc&0x0001 ) >0 ) {
                crc=crc>>1;
                crc=crc^ crc_v;

            } else
                crc=crc>>1;
        }
    }
    return ( crc );

}

