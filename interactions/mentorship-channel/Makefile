CC ?= gcc
SRCDIR := src
OBJDIR := obj
MAIN := cmd-set menu-set

CFLAGS := -Wall -Wextra -O0 -g
LDFLAGS := -ldiscord -lcurl -lcrypto -lpthread -lm

all: $(MAIN)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm $(MAIN)
