# Parallel matrix summation over named pipes (FIFO)

Two C programs - a **client** and a **server** - that sum an N×N matrix **in parallel** using **named pipes (FIFO)** for inter-process communication. The client generates a random N×N matrix and a separate server process is started for each row; every server sums its row and returns the partial sum, and the client aggregates the grand total. Built for the *Systemy Operacyjne* (Operating Systems) course.

**Detailed description below in English and Polish. - Szczegółowy opis poniżej po angielsku i polsku.**

---

## English

### What it does
- The client generates and prints a random N×N matrix (values 0-9).
- **Phase 1 - scatter:** for each row *i* the client opens the request FIFO `fifo_req_i` and writes first the size *N*, then the *N* integers of row *i*.
- Each **server** (launched with its id) iteratively blocks on its request FIFO, sums the received row, and writes the partial sum back over the response FIFO `fifo_res_i`.
- **Phase 2 - gather:** the client reads every partial sum and prints the grand total.
- `run.sh` builds both programs, launches *N* servers in the background (recording their PIDs), runs the client, then sends `SIGTERM` to the servers and cleans up.

### Concepts demonstrated
- **IPC with named pipes** - `mkfifo` / `open` / `read` / `write`; opening a FIFO blocks until the other end opens, which is used as a client↔server rendezvous.
- **Process orchestration** - background processes, PID tracking and teardown in `run.sh`.
- **Signal handling** - `SIGTERM` / `SIGINT` via `sigaction` and a `volatile sig_atomic_t` flag for a clean shutdown that unlinks the FIFOs.
- **Defensive I/O** - looped reads for partial transfers (`read_all`) and validation of sizes read from the pipe.

### Files
| File | Role |
|------|------|
| `server.c` | Iterative summation server (one row → one partial sum); terminfo colors; clean shutdown on signal. |
| `client.c` | Matrix generation, row scatter, partial-sum gather, grand total. |
| `run.sh` | Build + orchestrate `N=4` servers + the client + teardown. |
| `run_brak_serwerow.sh`, `run_zle_argumenty.sh`, `run_zle_n.sh`, `run_zly_naglowek.sh` | Negative-path scenarios: no servers running, bad arguments, bad `N`, malformed header. |
| `*_description.txt` | Per-component notes. |

### Build & run
Requires Linux, `gcc`, and ncurses/terminfo.

- Quick demo: `sh run.sh` - compiles with `gcc … -lncurses`, runs the demo with `N=4`, then tears everything down.
- Manually: `gcc server.c -o server -lncurses` and `gcc client.c -o client -lncurses`; start `./server 0` … `./server N-1` in the background, then `./client N`.

> The named pipes (`fifo_req_*`, `fifo_res_*`) are created at runtime and removed by each server on exit.

### Notes
- Colored output uses the curses **terminfo** layer (`setupterm` / `tigetstr` / `putp`), not full-screen curses, so it does not disturb normal `printf` output or the background servers.
- The FIFO reads loop over partial reads (`read_all`) and validate the sizes received from the pipe.

---

## Polski

### Co robi
- Klient generuje i wypisuje losową macierz N×N (wartości 0-9).
- **Faza 1 - rozesłanie:** dla każdego wiersza *i* klient otwiera potok żądań `fifo_req_i` i zapisuje najpierw rozmiar *N*, a potem *N* liczb całkowitych wiersza *i*.
- Każdy **serwer** (uruchomiony ze swoim id) iteracyjnie blokuje się na swoim potoku żądań, sumuje otrzymany wiersz i odsyła sumę częściową potokiem odpowiedzi `fifo_res_i`.
- **Faza 2 - zebranie:** klient odczytuje wszystkie sumy częściowe i wypisuje sumę całkowitą.
- `run.sh` kompiluje oba programy, uruchamia *N* serwerów w tle (zapisując ich PID-y), uruchamia klienta, a następnie wysyła `SIGTERM` do serwerów i sprząta.

### Demonstrowane zagadnienia
- **IPC przez potoki nazwane** - `mkfifo` / `open` / `read` / `write`; otwarcie FIFO blokuje do czasu otwarcia drugiego końca, co służy jako spotkanie (rendezvous) klient↔serwer.
- **Orkiestracja procesów** - procesy w tle, śledzenie PID-ów i sprzątanie w `run.sh`.
- **Obsługa sygnałów** - `SIGTERM` / `SIGINT` przez `sigaction` i flagę `volatile sig_atomic_t` dla czystego zamknięcia z usunięciem FIFO.
- **Defensywne I/O** - odczyty w pętli dla transferów częściowych (`read_all`) i walidacja rozmiarów odczytanych z potoku.

### Pliki
| Plik | Rola |
|------|------|
| `server.c` | Iteracyjny serwer sumujący (jeden wiersz → jedna suma częściowa); kolory terminfo; czyste zamknięcie na sygnał. |
| `client.c` | Generowanie macierzy, rozesłanie wierszy, zebranie sum częściowych, suma całkowita. |
| `run.sh` | Budowa + orkiestracja `N=4` serwerów + klient + sprzątanie. |
| `run_brak_serwerow.sh`, `run_zle_argumenty.sh`, `run_zle_n.sh`, `run_zly_naglowek.sh` | Scenariusze negatywne: brak serwerów, złe argumenty, złe `N`, błędny nagłówek. |
| `*_description.txt` | Notatki do poszczególnych komponentów. |

### Budowa i uruchomienie
Wymaga systemu Linux, `gcc` oraz ncurses/terminfo.

- Szybkie demo: `sh run.sh` - kompiluje z `gcc … -lncurses`, uruchamia demo z `N=4`, a potem wszystko sprząta.
- Ręcznie: `gcc server.c -o server -lncurses` i `gcc client.c -o client -lncurses`; uruchom `./server 0` … `./server N-1` w tle, a następnie `./client N`.

> Potoki nazwane (`fifo_req_*`, `fifo_res_*`) tworzone są w trakcie działania i usuwane przez każdy serwer przy wyjściu.

### Uwagi
- Kolorowanie wyjścia korzysta z warstwy **terminfo** biblioteki curses (`setupterm` / `tigetstr` / `putp`), a nie z trybu pełnoekranowego, więc nie zakłóca zwykłego `printf` ani serwerów działających w tle.
- Odczyty z FIFO obsługują częściowe transfery w pętli (`read_all`) i walidują rozmiary odczytane z potoku.
