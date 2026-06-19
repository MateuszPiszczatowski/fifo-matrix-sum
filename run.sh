#!/bin/sh

# Plik tymczasowy do przechowywania identyfikatorów procesów
PID_FILE="server_pids.txt"

# Sprzątanie pliku PID po ewentualnych poprzednich, nieudanych uruchomieniach
rm -f "$PID_FILE"

# Kompilacja programów
gcc server.c -o so_server -lncurses
gcc client.c -o so_client -lncurses

N=4 # Rozmiar tablicy i liczba serwerów

echo "--- Uruchamianie $N serwerow w tle ---"
i=0
while [ "$i" -lt "$N" ]; do
    ./so_server "$i" & echo $! >> "$PID_FILE"
    i=$((i + 1))
done

# Danie systemowi ułamka sekundy na utworzenie wszystkich kolejek FIFO
sleep 1 

printf "\n--- Uruchamianie Klienta ---\n"
./so_client "$N"

printf "\n--- Zamykanie srodowiska ---\n"

# Zamykanie serwerów na podstawie ich PID
if [ -f "$PID_FILE" ]; then
    while read -r pid; do
        # Wysłanie sygnału SIGTERM do procesu
        kill "$pid" 2>/dev/null
    done < "$PID_FILE"
    
    rm -f "$PID_FILE"
fi

# Usuwanie plików wykonywalnych
rm -f so_server so_client