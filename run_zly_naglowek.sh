#!/bin/sh
# =============================================================================
# Test obsluzonego bledu: ZLY NAGLOWEK (serwer dostaje n <= 0 z potoku)
# =============================================================================
# Wyzwala poprawke z server.c:
#   if (n <= 0) { close(fd_req); break; }
# Zamiast pelnego klienta wysylamy "recznie" naglowek z liczba 0 (4 bajty
# zerowe) do FIFO zadan serwera 0. Serwer odczytuje n = 0, odrzuca go
# i czysto konczy (sprzata FIFO) zamiast wolac malloc(0) czy blednie liczyc.
#
# Oczekiwane: serwer wypisuje swoj komunikat sprzatajacy i konczy sie sam
# (proces znika z listy), zamiast sie zawiesic lub zachowac nieprzewidywalnie.

gcc server.c -o so_server -lncurses || exit 1
rm -f fifo_req_* fifo_res_*

# Start jednego serwera (ID 0) w tle.
./so_server 0 &
SRV=$!
sleep 1

printf "\n--- wysylam naglowek n=0 (4 bajty zerowe) do fifo_req_0 ---\n"
printf '\0\0\0\0' > fifo_req_0

sleep 1
# Serwer powinien juz zakonczyc po wykryciu n<=0; kill tylko na wszelki wypadek.
kill "$SRV" 2>/dev/null

rm -f so_server fifo_req_* fifo_res_*
