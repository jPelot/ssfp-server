CC     = gcc
CFLAGS = -g
RM     = rm -f

HEADERS = socket.h strarray.h intarray.h voidarray.h ssfp_server.h
OBJECTS = socket.o strarray.o intarray.o voidarray.o ssfp-server.o

LDFLAGS = -L/usr/local/ssl/lib -L/user/lib/mysql
LDLIBS = -lssl -lcrypto -lmysqlclient

default: all

all: ssfp-server

%.o: %.c $(HEADERS)
	gcc -c $< -o $@

ssfp-server: main.o $(OBJECTS)
	$(CC) main.o $(OBJECTS) -o ssfp-server $(LDFLAGS) $(LDLIBS)

clean:
	-rm -f *.o
