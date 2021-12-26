CC ?= gcc
SRC := $(wildcard */*.c)
SETTERS := $(SRC:%.c=%)

CFLAGS := -Wall -Wextra -O0 -g -pthread
LDFLAGS := -ldiscord -lcurl -lcrypto -lm

%: %.c
	$(CC) $(CFLAGS) -o $(notdir $@) $< $(LDFLAGS)

all: $(SETTERS)

echo:
	@ echo SRC: $(SRC)
	@ echo SETTERS: $(SETTERS)

clean:
	rm $(notdir $(SETTERS))
