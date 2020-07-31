CFLAGS=-std=c99 -O2 -c
LDFLAGS=-lm
CC=gcc
FILES=hex.c
OBJECT=hex.o
EXEC=hex

.PHONY: all clear
all: $(OBJECT) $(EXEC)
$(OBJECT): $(FILE)
	$(CC) $(CFLAGS) $(FILES)
$(EXEC):
	$(CC) $(LDFLAGS) $(OBJECT) -o $(EXEC)
clear:
	rm $(OBJECT)
	rm $(EXEC)
