CC ?= gcc
SRCDIR := src
OBJDIR := obj
MAIN := main

CFLAGS := -Wall -Wextra -O0 -g
LDFLAGS := -ldiscord -lcurl -lcrypto -lpthread -lm

all: $(MAIN)

$(MAIN): $(MAIN).c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm $(MAIN)
