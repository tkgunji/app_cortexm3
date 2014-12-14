CC=arm-uclinuxeabi-gcc
CXX=arm-uclinuxeabi-g++
MIPSE=MIPSEL
EXTRAFLAGS=
CFLAGS=-c  -I. -I../include -D$(MIPSE) -DPROTOCOL=1 -I$(INSTALL_ROOT)/A2F/root/usr/include -g -O2 -mcpu=cortex-m3 -mthumb -L/rcu2/release/linux-cortexm-1.11.0/A2F/root/usr/lib -pthread  
LDFLAGS=-L. -mcpu=cortex-m3 -mthumb -L${INSTALL_ROOT}/A2F/root/usr/lib -pthread


all: app mem

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f app

app: app.o
	$(CC) $(LDFLAGS) app.o -o $@

app.o: app.c
	$(CC) $(CFLAGS) -o app.o app.c

mem: mem.o
	$(CC) $(LDFLAGS) mem.o -o $@

mem.o: mem.c
	$(CC) $(CFLAGS) -o mem.o mem.c

