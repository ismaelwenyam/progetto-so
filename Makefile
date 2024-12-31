# Makefile
CC = gcc
INCLUDES = -I./include
CFLAGS = -Wvla -Wextra -Werror -D_GNU_SOURCE
EXEC = direttore erogatore_ticket utente sportello operatore

ifdef DEBUG
    CFLAGS += -DDEBUG -g
endif

all: $(EXEC)

# Regole generiche per compilare file .c in file .o
src/%.o: src/%.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

# Link degli eseguibili
direttore: src/direttore.o src/simulation_configuration.o src/semapi.o src/simerr.o src/logapi.o src/sportello_api.o
	$(CC) -o $@ $^

erogatore_ticket: src/erogatore_ticket.o src/semapi.o src/simerr.o src/logapi.o src/simulation_configuration.o src/sportello_api.o
	$(CC) -o $@ $^

utente: src/utente.o src/simulation_configuration.o src/semapi.o src/simerr.o src/logapi.o src/sportello_api.o
	$(CC) -o $@ $^

erogatore_ticket: src/erogatore_ticket.o src/semapi.o src/simerr.o src/logapi.o src/sportello_api.o
	$(CC) -o $@ $^

sportello: src/sportello.o src/simerr.o src/logapi.o src/semapi.o src/simulation_configuration.o src/sportello_api.o
	$(CC) -o $@ $^

operatore: src/operatore.o src/simulation_configuration.o src/semapi.o src/simerr.o src/logapi.o src/sportello_api.o
	$(CC) -o $@ $^

clean:
	rm -f $(EXEC) src/*.o
