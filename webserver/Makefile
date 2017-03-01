CC = arm-none-linux-gnueabi-gcc
EXTRA_LIBS += -static -lpthread

EXEC = httpd
OBJS = httpd.o copy.o camera.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) -o $@ $(OBJS) $(EXTRA_LIBS)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o
