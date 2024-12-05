#Makefile
CC = gcc
INCLUDES = -I./include
CFLAGS = -Wvla -Wextra -Werror -D_GNU_SOURCE
EXEC = simulation erogatore_ticket

ifdef DEBUG
    CFLAGS += -DDEBUG -g
endif

all: $(EXEC)

src/direttore.o: src/direttore.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

src/erogatore_ticket.o: src/erogatore_ticket.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

src/semapi.o: src/semapi.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

src/simerr.o: src/simerr.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

simulation: src/direttore.o src/semapi.o src/simerr.o
	$(CC) -o $@ $^

erogatore_ticket: src/erogatore_ticket.o src/semapi.o src/simerr.o
	$(CC) -o $@ $^

clean:
	rm -f $(EXEC) *.swp src/.*o
