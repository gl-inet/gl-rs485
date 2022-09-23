include $(TOPDIR)/rules.mk

ifdef CONFIG_TARGET_ath79
       GLUTIL = -lglutil_ath79
else ifdef CONFIG_TARGET_x86_64
       GLUTIL = -lglutil_x86_64
endif


LDFLAGS += -L. -I include -Xlinker "-("  -Bstatic $(GLUTIL) -Xlinker "-)" -ljson-c -luci -lpaho-mqtt3cs

all :  gl-rs485 librs485api


librs485api:
	$(CC) $(CFLAGS)  -Wall cfg.c log.c conn.c crc16.c main.c mqtt.c queue.c sock.c state.c uart_posix.c modbus_api.c -fPIC -o librs485api.so  $(LDFLAGS)
gl-rs485 :
	$(CC) $(CFLAGS)  cfg.c log.c conn.c crc16.c main.c mqtt.c queue.c sock.c state.c uart_posix.c modbus_api.c $^ -o $@  $(LDFLAGS)
clean:
	rm -fr $(PROC)  gl-rs485 librs485api
