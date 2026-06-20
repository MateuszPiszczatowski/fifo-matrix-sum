#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <curses.h>   // biblioteka curses - kolory terminala (warstwa terminfo)
#include <term.h>

// Drobny dodatek wizualny: kolorowanie komunikatow przez terminfo z biblioteki curses.
static int g_has_color = 0;
static char *g_setaf, *g_sgr0, *g_bold;

static void ui_init(void) {
    int err;
    if (setupterm(NULL, STDOUT_FILENO, &err) == OK) {
        g_setaf = tigetstr("setaf"); // ustaw kolor tekstu
        g_sgr0  = tigetstr("sgr0");  // reset atrybutow
        g_bold  = tigetstr("bold");
        if (g_setaf && g_sgr0) g_has_color = 1;
    }
}
static void ui_color(int c) { if (g_has_color) putp(tparm(g_setaf, c, 0,0,0,0,0,0,0,0)); }
static void ui_bold(void)   { if (g_has_color && g_bold) putp(g_bold); }
static void ui_reset(void)  { if (g_has_color) putp(g_sgr0); }

// Flaga sterująca pętlą
volatile sig_atomic_t keep_running = 1;

// Funkcja przechwytująca sygnał SIGTERM / SIGINT
void handle_signal(int sig) {
    keep_running = 0;
}

// Odczytuje dokładnie len bajtów z deskryptora fd w pętli.
// Zwraca 0 przy sukcesie, -1 przy błędzie lub zamknięciu potoku.
static int read_all(int fd, void *buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t r = read(fd, (char *)buf + total, len - total);
        if (r <= 0) return -1;
        total += (size_t)r;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uzycie: %s <ID_SERWERA>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    ui_init();

    int server_id = atoi(argv[1]);
    char req_fifo[32], res_fifo[32];

    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask); // Brak blokady innych sygnałów
    sa.sa_flags = 0; // Czyszczenie flag, nie wznawia open jeśli je przerwało - przerwane open zwraca błąd

    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    sprintf(req_fifo, "fifo_req_%d", server_id);
    sprintf(res_fifo, "fifo_res_%d", server_id);

    mkfifo(req_fifo, 0666);
    mkfifo(res_fifo, 0666);

    ui_color(COLOR_BLUE);
    printf("[Serwer %d] Uruchomiony iteracyjnie. Czekam na zadania...", server_id);
    ui_reset(); printf("\n");

    while (keep_running) {
        int fd_req = open(req_fifo, O_RDONLY);

        // Jeśli open() zwróciło błąd (np. zostało przerwane przez sygnał)
        if (fd_req == -1) {
            if (!keep_running) break;
            continue;
        }

        // Odczyt ile danych należy odczytać w następnym kroku
        int n;
        // Częściowy odczyt nagłówka — read() może oddać <4 bajty, więc read_all() pętli aż do kompletu
        if (read_all(fd_req, &n, sizeof(int)) < 0) {
            close(fd_req);
            break;
        }
        // Walidacja n z potoku — zabezpiecza malloc(n*sizeof(int)) przed n<=0
        if (n <= 0) { close(fd_req); break; }

        // Odczyt właściwych danych
        int *row = (int *)malloc(n * sizeof(int));
        if (row == NULL) {
            close(fd_req);
            break;
        }
        if (read_all(fd_req, row, n * sizeof(int)) < 0) {
            free(row);
            close(fd_req);
            break;
        }
        close(fd_req);

        int partial_sum = 0;
        for (int i = 0; i < n; i++) {
            partial_sum += row[i];
        }

        free(row);

        int fd_res = open(res_fifo, O_WRONLY);
        if (fd_res == -1) {
            break;
        }
        if (write(fd_res, &partial_sum, sizeof(int)) < 0) {
            close(fd_res);
            break;
        }
        close(fd_res);
    }

    printf("\n");
    ui_color(COLOR_RED);
    printf("[Serwer %d] Otrzymano sygnal lub zamknieto potok. Zamykam iteracje, sprzatam FIFO i wychodze!", server_id);
    ui_reset(); printf("\n");
    unlink(req_fifo);
    unlink(res_fifo);

    return 0;
}
