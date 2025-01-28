#!/bin/bash

CONFIG_FILE="simulation_configuration.conf" # Nome del file di configurazione

# Verifica se il file di configurazione esiste
if [ ! -f "$CONFIG_FILE" ]; then
    echo "Errore: il file di configurazione '$CONFIG_FILE' non esiste."
    exit 1
fi

# Funzione per mostrare i parametri attuali
show_config() {
    echo "Parametri attuali:"
    cat "$CONFIG_FILE"
    echo
}

# Funzione per modificare un parametro
modify_param() {
    local param_name=$1
    local new_value=$2

    # Verifica se il parametro esiste nel file
    if grep -q "^$param_name\b" "$CONFIG_FILE"; then
        sed -i "s/^$param_name\s\+.*/$param_name\t$new_value/" "$CONFIG_FILE"
        echo "Parametro '$param_name' aggiornato a '$new_value'."
    else
        echo -e "\e[31mErrore: parametro '$param_name' non trovato nel file di configurazione.\e[0m"
        #echo "Parametri disponibili:"
        #awk '{print $1}' "$CONFIG_FILE"
    fi
}

# Menu principale
while true; do
    show_config

    echo "Menu:"
    echo "1. Modifica un parametro"
    echo "2. Esci"
    read -p "Seleziona un'opzione: " choice

    case $choice in
        1)
            read -p "Inserisci il nome del parametro da modificare: " param
            read -p "Inserisci il nuovo valore per '$param': " value
            modify_param "$param" "$value"
            ;;
        2)
            clear
            exit 0
            ;;
        *)
            echo "Opzione non valida. Riprova."
            ;;
    esac

done
