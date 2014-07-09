OBJECTS=mtddev_flash.o
SOURCES=mtddev_flash.c 
EXECUTABLE=mtddev_flash
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
	rm -f mtddev_flash

mtddev_flash: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

mtddev_flash.o: mtddev_flash.c
	$(CC) $(CFLAGS) -o mtddev_flash.o mtddev_flash.c

