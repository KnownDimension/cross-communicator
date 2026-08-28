libraries = -lpthread -lcurl -ldiscord -lsqlite3 # used by program
CFLAGS = -fstack-protector-strong -Wall

SRC = $(wildcard src/*.c)

TARGET = test/program

prod:
	gcc $(SRC) $(libraries) $(CFLAGS) -o $(TARGET)