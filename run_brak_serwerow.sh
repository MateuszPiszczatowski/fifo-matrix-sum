#!/bin/sh
# =============================================================================
# Test obsluzonego bledu: BRAK SERWEROW (klient nie moze otworzyc FIFO)
# =============================================================================
# Wyzwala obsluzony blad z client.c (FAZA 1):
#   open(fifo_req_i, O_WRONLY) == -1
#   perror("Blad otwarcia FIFO do zapisu. Czy serwery dzialaja?")
# Uruchamiamy klienta z poprawnym N, ale BEZ zadnego serwera.
#
# Oczekiwane: komunikat "...Czy serwery dzialaja?" i kod wyjscia != 0.

gcc client.c -o so_client -lncurses || exit 1

# Usuwamy ewentualne pozostalosci FIFO po wczesniejszych uruchomieniach -
# inaczej open(O_WRONLY) zablokowalby sie (czekajac na czytelnika) zamiast
# zwrocic blad ENOENT.
rm -f fifo_req_* fifo_res_*

printf "\n--- ./so_client 4 (bez uruchomionych serwerow) ---\n"
./so_client 4
printf "(kod wyjscia: %d)\n" "$?"

rm -f so_client
