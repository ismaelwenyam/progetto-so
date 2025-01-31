# Directory Structure: `progetto-so`

```plaintext
progetto-so/
├── .gitignore
├── README.md
├── Makefile
├── Dockerfile
├── edit_config.sh
├── sim_runner.sh
├── config_explode.conf
├── config_timout.conf
├── relazione.pdf
├── simulation_configuration.conf
├── include/
│   ├── logapi.h
│   ├── msgapi.h
│   ├── semapi.h
│   ├── service_api.h
│   ├── shmapi.h
│   ├── simerr.h
│   ├── simulation_configuration.h
│   ├── simulation_stats.h
│   ├── sportello_api.h
│   ├── ticket.h
│   └── utils_api.h
└── src/
    ├── add_users.c
    ├── direttore.c
    ├── erogatore_ticket.c
    ├── logapi.c
    ├── operatore.c
    ├── semapi.c
    ├── simerr.c
    ├── simulation_configuration.c
    ├── simulation_stats_api.c
    ├── sportello.c
    ├── utente.c
    └── utils.c
```

# How to run simulation
In order to start the simulation you can either use the simulation runner, which guides you in the process of running the simulation, just run
```plaintext
./sim_runner.sh
```
You will be showed this window where you can choose the actions to do.
```plaintext
Menu principale:
1. Eseguire la compilazione
2. Eseguire la clean
3. Avviare simulazione
4. Modificare i parametri di configurazione
5. Avviare la simulazione - con make clean
6. Avviare la simulazione - verbose con (logs.txt)
7. Mostra risorse ipc
9. Pulire console
8. Esci
Seleziona un'opzione: 
```
* Option 3 simply starts the simulation, after step 1 and 2 have been done.
* Option 4 executes make clean, than make and starts the simulation
* Option 5 executes remove of previous logs.txt file, make clean, than make debug and starts the simulation

## How to add users
In order to add users to the simulation just run 
```plaintext
./add_users 
```
in a new terminal, under project directory.

# Manual 
### Generate executables

Place yourself in the working directory of the project and run
```plaintext
make
```
This will generate executables.
In order to enable logs run 
```plaintext
make debug
```
This run make with -DDEBUG -g flags.

### Edit simulation parameters

You can edit the simulation parameters in simulation_configuration.conf file, by using the script edit_config.sh which will help you doing so avoiding formatting errors.
Just run
```plaintext
./edit_config.sh
```

### Run simulation
Run 
```plaintext
./direttore
```

### Clean directory
In order to clean the directory just run
```plaintext
make clean
```
This command will remove the executables files and object files, but also the csv files.



