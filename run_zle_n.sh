#!/bin/sh
# =============================================================================
# Test obsluzonego bledu: NIEPRAWIDLOWE N w kliencie (walidacja n <= 0)
# =============================================================================
# Wyzwala poprawke z client.c:
#   if (n <= 0) { fprintf(stderr, "Blad: N musi byc dodatnia liczba calkowita"); exit(EXIT_FAILURE); }
# atoi() cicho zwraca 0 dla tekstu i wartosci ujemnych, dlatego klient
# odrzuca takie N, zanim cokolwiek zaalokuje lub policzy.
#
# Oczekiwane: dla kazdego wywolania komunikat
#   "Blad: N musi byc dodatnia liczba calkowita"  oraz kod wyjscia != 0.

gcc client.c -o so_client -lncurses || exit 1

for N in 0 -5 abc; do
    printf "\n--- ./so_client %s ---\n" "$N"
    ./so_client "$N"
    printf "(kod wyjscia: %d)\n" "$?"
done

rm -f so_client
