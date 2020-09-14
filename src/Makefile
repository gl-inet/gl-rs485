LDFLAGS = -ljson-c -lglutil

all :  librs485api  gl-rs485



librs485api:
	$(CC) $(CFLAGS) $(LDFLAGS)  -Wall cfg.c  conn.c  crc16.c  log.c  main.c   queue.c   sock.c  state.c  uart_posix.c   modbus_api.c  -shared -fPIC -o librs485api.so $(LDFLAGS)
gl-rs485 :
	$(CC) $(CFLAGS) $(LDFLAGS)  $^ -o $@ -L. -lrs485api
clean:
	rm -fr $(PROC)  gl-rs485 librs485api
