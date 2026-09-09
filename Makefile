libraries = -lpthread -lcurl -ldiscord -lsqlite3 # used by program
CFLAGS = -fstack-protector-strong # -Wall

SRC = $(wildcard src/*.c)

TARGETTEST = test/program
TARGETPROD = build/program

.PHONY: test prod

test:
	gcc $(SRC) $(libraries) $(CFLAGS) -o $(TARGETTEST)

prod:
	mkdir build
	gcc $(SRC) $(libraries) $(CFLAGS) -o $(TARGETPROD)
	cp src/copystructure.db build/data.db
	cp src/configexample.json build/config.json