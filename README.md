# Directory Structure: `progetto-so`

```plaintext
progetto-so/
├── README.md
├── Makefile
├── config_explode.conf
├── config_timout.conf
├── progetto_sso_design.drawio
├── relazione_so.odt
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

# How to run simulation guided
In order to start the simulation you can either use the simulation runner, which guides you in the process of running the simulation, just run
```plaintext
./sim_runner.sh
```
You will be showed this window where you can choose the actions to do.
```plaintext
Menu principale:
1. Eseguire la build
2. Eseguire la clean
3. Modificare i parametri di configurazione
4. Avviare la simulazione
5. Avviare la simulazione - verbose
6. Esci
Seleziona un'opzione: 
```

# Manual 
### Generate executables

Place yourself in the working directory of the project and run
```plaintext
make
```
This will generate executables.

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



