OBJECTS=spidev_flash.o
SOURCES=spidev_flash.c 
EXECUTABLE=spidev_flash
CC=arm-uclinuxeabi-gcc
CXX=arm-uclinuxeabi-g++
MIPSE=MIPSEL
EXTRAFLAGS=
CFLAGS=-c  -I. -I../include -D$(MIPSE) -DPROTOCOL=1 -I$(INSTALL_ROOT)/A2F/root/usr/include -g -O2 -mcpu=cortex-m3 -mthumb -L/rcu2/release/linux-cortexm-1.11.0/A2F/root/usr/lib -pthread  
LDFLAGS=-L. -mcpu=cortex-m3 -mthumb -L${INSTALL_ROOT}/A2F/root/usr/lib -pthread


all: $(EXECUTABLE) 

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f spidev_flash

spidev_flash: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

spidev_flash.o: spidev_flash.c
	$(CC) $(CFLAGS) -o spidev_flash.o spidev_flash.c

