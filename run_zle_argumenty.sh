#!/bin/sh
# =============================================================================
# Test obsluzonego bledu: ZLA LICZBA ARGUMENTOW (walidacja argc != 2)
# =============================================================================
# Wyzwala obsluzone bledy:
#   client.c -> if (argc != 2) fprintf(stderr, "Uzycie: %s <Rozmiar_N>")
#   server.c -> if (argc != 2) fprintf(stderr, "Uzycie: %s <ID_SERWERA>")
#
# Oczekiwane: komunikaty "Uzycie: ..." i kody wyjscia != 0.

gcc client.c -o so_client -lncurses || exit 1
gcc server.c -o so_server -lncurses || exit 1

printf "\n--- ./so_client (brak argumentu) ---\n"
./so_client; printf "(kod wyjscia: %d)\n" "$?"

printf "\n--- ./so_client 4 8 (za duzo argumentow) ---\n"
./so_client 4 8; printf "(kod wyjscia: %d)\n" "$?"

printf "\n--- ./so_server (brak ID) ---\n"
./so_server; printf "(kod wyjscia: %d)\n" "$?"

rm -f so_client so_server
