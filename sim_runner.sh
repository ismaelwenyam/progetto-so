#!/bin/bash

EDIT_CONFIG_SCRIPT="edit_config.sh"

run_build() {
    echo "Eseguendo la build..."
    make
}

run_clean() {
    echo "Eseguendo la pulizia..."
    make clean
}

edit_config() {
    if [ -f "$EDIT_CONFIG_SCRIPT" ]; then
        bash "$EDIT_CONFIG_SCRIPT"
    else
        echo -e "\e[31mErrore: lo script '$EDIT_CONFIG_SCRIPT' non esiste.\e[0m"
    fi
}

run_simulation() {
    ./direttore
}

run_simulation_clean() {
    run_clean
    echo "Eseguendo la build e avviando la simulazione..."
    make && clear && ./direttore
}

run_simulation_verbose() {
    run_clean
    echo "Eseguendo la build e avviando la simulazione con logs..."
    make debug && clear && ./direttore
}

show_ipc_resources () {
    ipcs
}

clear_console () {
    clear
}

while true; do
    echo "Menu principale:"
    echo "1. Eseguire la compilazione"
    echo "2. Eseguire la clean"
    echo "3. Avviare la simulazione"
    echo "4. Modificare i parametri di configurazione"
    echo "5. Avviare la simulazione - con make clean"
    echo "6. Avviare la simulazione - verbose con (logs.txt)"
    echo "7. Mostra risorse ipc"
    echo "8. Pulire console"
    echo "9. Esci"
    read -p "Seleziona un'opzione: " choice

    case $choice in
        1)
            run_build
            ;;
        2)
            run_clean
            ;;
        3)
            run_simulation
            ;;
        4)
            edit_config
            ;;
        5)
            run_simulation_clean
            ;;
        6)
            run_simulation_verbose
            ;;
        7)
            show_ipc_resources
            ;;
        8)
            clear_console
            ;;
        9)
            clear
            exit 0
            ;;
        *)
            echo -e "\e[31mOpzione non valida. Riprova.\e[0m"
            ;;
    esac

done
