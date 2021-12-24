CC ?= gcc
SRCDIR := src
OBJDIR := obj
MAIN := main

CFLAGS := -pthread -Wall -Wextra -O0 -g
LDFLAGS := -ldiscord -lcurl -lcrypto -lm

all: $(MAIN)

$(MAIN): $(MAIN).c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm $(MAIN)
