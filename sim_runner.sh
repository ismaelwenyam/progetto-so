#!/bin/bash

EDIT_CONFIG_SCRIPT="edit_config.sh" # Nome dello script per modificare i parametri

# Funzione per eseguire la build
run_build() {
    echo "Eseguendo la build..."
    make
}

# Funzione per eseguire la clean
run_clean() {
    echo "Eseguendo la pulizia..."
    make clean
}

# Funzione per modificare i parametri di configurazione
edit_config() {
    if [ -f "$EDIT_CONFIG_SCRIPT" ]; then
        bash "$EDIT_CONFIG_SCRIPT"
    else
        echo -e "\e[31mErrore: lo script '$EDIT_CONFIG_SCRIPT' non esiste.\e[0m"
    fi
}

# Funzione per avviare la simulazione
run_simulation() {
    run_clean
    echo "Eseguendo la build e avviando la simulazione..."
    make && ./direttore
}

# Funzione per avviare la simulazione con logs
run_simulation_verbose() {
    run_clean
    echo "Eseguendo la build e avviando la simulazione con logs..."
    make debug && ./direttore
}

# Menu principale
while true; do
    echo "Menu principale:"
    echo "1. Eseguire la build"
    echo "2. Eseguire la clean"
    echo "3. Modificare i parametri di configurazione"
    echo "4. Avviare la simulazione"
    echo "5. Avviare la simulazione - verbose"
    echo "6. Esci"
    read -p "Seleziona un'opzione: " choice

    case $choice in
        1)
            run_build
            ;;
        2)
            run_clean
            ;;
        3)
            edit_config
            ;;
        4)
            run_simulation
            ;;
        5)
            run_simulation_verbose
            ;;
        6)
            clear
            exit 0
            ;;
        *)
            echo -e "\e[31mOpzione non valida. Riprova.\e[0m"
            ;;
    esac

done
