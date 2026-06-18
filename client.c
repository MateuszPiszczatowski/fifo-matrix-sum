#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <curses.h>   // biblioteka curses - kolory terminala (warstwa terminfo)
#include <term.h>

// Drobny dodatek wizualny: kolorowanie wyjscia przez terminfo z biblioteki curses.
// Uzywamy warstwy terminfo (a nie trybu pelnoekranowego initscr()), bo nie zaklocamy
// dotychczasowego wypisywania przez printf ani serwerow dzialajacych w tle.
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

// Zwalnia rows wierszy macierzy oraz sam wskaźnik tablicy.
static void free_matrix(int **m, int rows) {
    for (int i = 0; i < rows; i++) {
        free(m[i]);
    }
    free(m);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uzycie: %s <Rozmiar_N>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    ui_init();

    int n = atoi(argv[1]);

    // Generowanie i wypisanie losowej macierzy N x N
    int **matrix = (int **)malloc(n * sizeof(int *));
    if (matrix == NULL) {
        fprintf(stderr, "malloc: brak pamieci\n");
        exit(EXIT_FAILURE);
    }
    srand(time(NULL));
    ui_bold(); ui_color(COLOR_GREEN);
    printf("[Klient] Wygenerowana macierz %dx%d:", n, n);
    ui_reset(); printf("\n");
    for (int i = 0; i < n; i++) {
        matrix[i] = (int *)malloc(n * sizeof(int));
        if (matrix[i] == NULL) {
            fprintf(stderr, "malloc: brak pamieci\n");
            free_matrix(matrix, i);
            exit(EXIT_FAILURE);
        }
        ui_color(COLOR_CYAN);
        for (int j = 0; j < n; j++) {
            matrix[i][j] = rand() % 10;
            printf("%d ", matrix[i][j]);
        }
        ui_reset(); printf("\n");
    }

    // FAZA 1: Współbieżne rozesłanie danych do wszystkich serwerów
    char fifo_name[32];
    for (int i = 0; i < n; i++) {
        sprintf(fifo_name, "fifo_req_%d", i);
        int fd_req = open(fifo_name, O_WRONLY); // Odblokowuje serwer 'i'
        if (fd_req == -1) {
            perror("Blad otwarcia FIFO do zapisu. Czy serwery dzialaja?");
            free_matrix(matrix, n);
            exit(EXIT_FAILURE);
        }

        // Wysyłamy rozmiar (N), a potem cały wiersz i-ty
        write(fd_req, &n, sizeof(int));
        write(fd_req, matrix[i], n * sizeof(int));
        close(fd_req);
    }
    ui_color(COLOR_MAGENTA);
    printf("[Klient] Rozeslano wszystkie wiersze. Trwaja obliczenia wspolbiezne...");
    ui_reset(); printf("\n");

    // FAZA 2: Zbieranie sum częściowych
    int total_sum = 0;
    for (int i = 0; i < n; i++) {
        sprintf(fifo_name, "fifo_res_%d", i);
        int fd_res = open(fifo_name, O_RDONLY); // Pozwala serwerowi 'i' wysłać wynik
        if (fd_res == -1) {
            perror("Blad otwarcia FIFO do odczytu");
            free_matrix(matrix, n);
            exit(EXIT_FAILURE);
        }

        int partial_sum;
        if (read(fd_res, &partial_sum, sizeof(int)) <= 0) {
            perror("Blad odczytu z FIFO");
            close(fd_res);
            free_matrix(matrix, n);
            exit(EXIT_FAILURE);
        }
        close(fd_res);

        ui_color(COLOR_YELLOW);
        printf("[Klient] Suma czastkowa z serwera %d = %d", i, partial_sum);
        ui_reset(); printf("\n");
        total_sum += partial_sum;
    }

    printf("\n");
    ui_bold(); ui_color(COLOR_GREEN);
    printf("[Klient] SUMA CALKOWITA WYNOSI: %d", total_sum);
    ui_reset(); printf("\n");

    // Sprzątanie pamięci
    free_matrix(matrix, n);

    return 0;
}
