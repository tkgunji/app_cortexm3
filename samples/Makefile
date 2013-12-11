SRCS = func.c
OBJS = $(SRCS:.c=.o)
TARGET = libfunc.so
CXX = arm-uclinuxeabi-gcc
#CXX = arm-uclibc-g++
#CXXFLAGS = -O3 -Wall -fPIC -I$(INSTALL_ROOT)/A2F/root/usr/include
CXXFLAGS = -O3 -Wall -fPIC 
#LDFLAGS = -shared -mcpu=cortex-m3 -mthumb -L${INSTALL_ROOT}/A2F/root/usr/lib -pthread
LDFLAGS = -shared 
$LIBS = 

all: $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LIBS) -o $(TARGET)
	@echo done.

$(OBJS): $(SRCS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(SRCS) -c

clean:
	rm -rf *.o core.* *# *~ $(TARGET)