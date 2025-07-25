# Makefile
CC = gcc
INCLUDES = -I./include
CFLAGS = -Wvla -Wextra -Werror -D_GNU_SOURCE
EXEC = direttore erogatore_ticket utente sportello operatore add_user

ifdef DEBUG
    CFLAGS += -DDEBUG -g
endif

all: $(EXEC)

debug: 
	$(MAKE) DEBUG=1

# compilazione file .c in file .o
src/%.o: src/%.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

# link eseguibili
direttore: src/direttore.o src/simulation_configuration.o src/semapi.o src/simerr.o src/logapi.o src/utils.o src/simulation_stats_api.o
	$(CC) -o $@ $^

erogatore_ticket: src/erogatore_ticket.o src/semapi.o src/simerr.o src/logapi.o src/simulation_configuration.o src/utils.o
	$(CC) -o $@ $^

utente: src/utente.o src/semapi.o src/simerr.o src/logapi.o src/simulation_configuration.o src/simulation_stats_api.o
	$(CC) -o $@ $^

sportello: src/sportello.o src/simerr.o src/logapi.o src/semapi.o src/simulation_configuration.o src/utils.o
	$(CC) -o $@ $^

operatore: src/operatore.o src/simulation_configuration.o src/semapi.o src/simerr.o src/logapi.o src/utils.o src/simulation_stats_api.o
	$(CC) -o $@ $^

add_user: src/add_users.o src/simerr.o
	$(CC) -o $@ $^
clean:
	rm -f $(EXEC) src/*.o *.csv logs.txt
