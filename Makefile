CFLAGS=-std=c99 -O2 -c -Wall -Wextra
CC=gcc
FILES=hex.c
OBJECT=hex.o
EXEC=hex

.PHONY: all clear install
all: $(OBJECT) $(EXEC)
$(OBJECT): $(FILE)
	$(CC) $(CFLAGS) $(FILES)
$(EXEC): $(OBJECT)
	$(CC) $(OBJECT) -o $(EXEC)
clean:
	rm $(OBJECT)
	rm $(EXEC)
install: $(EXEC)
	cp $(EXEC) /usr/bin/$(EXEC)
