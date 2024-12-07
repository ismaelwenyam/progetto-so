# Makefile
CC = gcc
INCLUDES = -I./include
CFLAGS = -Wvla -Wextra -Werror -D_GNU_SOURCE
EXEC = simulation erogatore_ticket utente

ifdef DEBUG
    CFLAGS += -DDEBUG -g
endif

all: $(EXEC)

# Regole generiche per compilare file .c in file .o
src/%.o: src/%.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

# Link degli eseguibili
simulation: src/direttore.o src/config_sim.o src/semapi.o src/simerr.o
	$(CC) -o $@ $^

erogatore_ticket: src/erogatore_ticket.o src/semapi.o src/simerr.o
	$(CC) -o $@ $^

utente: src/utente.o src/config_sim.o src/semapi.o src/simerr.o
	$(CC) -o $@ $^

clean:
	rm -f $(EXEC) src/*.o
