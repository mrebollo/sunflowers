/**
 * @file sunflowers.c
 * @brief Juego de deducción Sunflowers (buscaminas inverso) en consola.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


/* ========================================================================== */
/* CONSTANTES Y ESTRUCTURAS DE DATOS                                          */
/* ========================================================================== */

/** @name Constantes del juego */
///@{
#define SIZE 8          ///< Dimensión del tablero (8x8).
#define FLOWERS 18      ///< Total de flores a descubrir.
#define CLUES 26        ///< Pistas fijas visibles desde el inicio.
#define MAX_ERRORS 3    ///< Límite de fallos permitidos.
///@}

/** @name Códigos de escape ANSI para formato en terminal */
///@{
#define ANSI_RESET  "\033[0m"  ///< Restaurar formato por defecto.
#define ANSI_GRAY   "\033[90m" ///< Gris tenue (pistas resueltas y celdas ocultas).
#define ANSI_CYAN   "\033[36m" ///< Cian (cabeceras de fila y columna).
#define ANSI_YELLOW "\033[33m" ///< Amarillo (flores descubiertas).
///@}

/**
 * @brief Contenido real subyacente de cada casilla.
 */
typedef enum {
    EMPTY,  ///< Casilla vacía (sin flor).
    FLOWER  ///< Casilla que contiene una flor.
} content_t;

/**
 * @brief Estado visible e interactivo de la casilla para el jugador.
 */
typedef enum {
    HIDDEN,          ///< Incógnita inicial (se muestra como '.').
    CLUE,            ///< Pista fija visible desde el inicio (número 1 a 8).
    REVEALED_FLOWER, ///< Flor descubierta correctamente ('o').
    REVEALED_EMPTY   ///< Casilla vacía descubierta correctamente ('x').
} state_t;

/**
 * @brief Representación individual de cada celda del tablero.
 */
typedef struct {
    content_t content; ///< Contenido real de la celda (EMPTY o FLOWER).
    int clue_count;    ///< Número de flores en sus 8 casillas vecinas (0-8).
    state_t state;     ///< Estado visual actual para el jugador.
} cell_t;

/**
 * @brief Estadísticas y progreso de la partida en curso.
 */
typedef struct {
    int flowers; ///< Contador de flores descubiertas hasta el momento.
    int errors;  ///< Contador de fallos acumulados cometidos por el jugador.
} stats_t;


/* ========================================================================== */
/* 1. GENERACIÓN E INICIALIZACIÓN DEL TABLERO                                 */
/* ========================================================================== */

/**
 * @brief Inicializa todas las celdas del tablero como vacías y ocultas.
 * @param b Matriz del tablero de juego.
 */
void empty_board(cell_t b[SIZE][SIZE]) {
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            b[r][c].content = EMPTY;
            b[r][c].clue_count = 0;
            b[r][c].state = HIDDEN;
        }
    }
}

/**
 * @brief Coloca aleatoriamente FLOWERS flores sin solapar en el tablero.
 * @param b Matriz del tablero de juego.
 */
void random_flowers(cell_t b[SIZE][SIZE]) {
    int placed = 0;
    while (placed < FLOWERS) {
        int r = rand() % SIZE;
        int c = rand() % SIZE;
        if (b[r][c].content == EMPTY) {
            b[r][c].content = FLOWER;
            placed++;
        }
    }
}

/**
 * @brief Coordinadora: inicializa el tablero y distribuye las flores al azar.
 * @param b Matriz del tablero de juego.
 */
void init_board(cell_t b[SIZE][SIZE]) {
    empty_board(b);
    random_flowers(b);
}


/* ========================================================================== */
/* 2. CÁLCULO Y DISTRIBUCIÓN DE PISTAS                                        */
/* ========================================================================== */

/**
 * @brief Cuenta cuántas flores hay en las 8 casillas vecinas de una celda.
 * @param b Matriz del tablero de juego.
 * @param r Coordenada de fila (0 a SIZE-1).
 * @param c Coordenada de columna (0 a SIZE-1).
 * @return Cantidad de flores adyacentes encontradas (0 a 8).
 */
int count_flowers_around(cell_t b[SIZE][SIZE], int r, int c) {
    int count = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0)
                continue;
            int nr = r + dr;
            int nc = c + dc;
            // Comprobar que el vecino esté dentro de los límites de la matriz
            if (nr >= 0 && nr < SIZE && nc >= 0 && nc < SIZE) {
                if (b[nr][nc].content == FLOWER)
                    count++;
            }
        }
    }
    return count;
}

/**
 * @brief Calcula la cuenta de flores vecinas para todas las casillas vacías.
 * @param b Matriz del tablero de juego.
 */
void count_all_clues(cell_t b[SIZE][SIZE]) {
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            if (b[r][c].content == EMPTY) {
                b[r][c].clue_count = count_flowers_around(b, r, c);
                b[r][c].state = HIDDEN;
            }
        }
    }
}

/**
 * @brief Selecciona aleatoriamente CLUES casillas vacías para mostrarlas como pistas.
 * @param b Matriz del tablero de juego.
 */
void select_visible_clues(cell_t b[SIZE][SIZE]) {
    int placed = 0;
    while (placed < CLUES) {
        int r = rand() % SIZE;
        int c = rand() % SIZE;
        if (b[r][c].content == EMPTY && b[r][c].state == HIDDEN) {
            b[r][c].state = CLUE;
            placed++;
        }
    }
}

/**
 * @brief Coordinadora: calcula todas las pistas y activa las que serán visibles.
 * @param b Matriz del tablero de juego.
 */
void compute_clues(cell_t b[SIZE][SIZE]) {
    count_all_clues(b);
    select_visible_clues(b);
}


/* ========================================================================== */
/* 3. RENDERIZADO Y VISUALIZACIÓN EN TERMINAL                                 */
/* ========================================================================== */

/**
 * @brief Muestra el tablero con la solución completa de flores (modo depuración/fin).
 * @param b Matriz del tablero de juego.
 */
void print_flowers_debug(cell_t b[SIZE][SIZE]) {
    char symbol[] = {'.', '*'}; // 0 = EMPTY, 1 = FLOWER
    printf("Tablero de solucion (* = flor, . = vacio):\n\n  ");

    for (int c = 0; c < SIZE; c++) {
        printf("%d ", c);
    }
    printf("\n");

    for (int r = 0; r < SIZE; r++) {
        printf("%d ", r);
        for (int c = 0; c < SIZE; c++)
            printf("%c ", symbol[b[r][c].content]);
        printf("\n");
    }
}

/**
 * @brief Comprueba si una pista ya tiene todas sus flores vecinas descubiertas.
 * @param b Matriz del tablero de juego.
 * @param r Coordenada de fila (0 a SIZE-1).
 * @param c Coordenada de columna (0 a SIZE-1).
 * @return true si todas las flores adyacentes han sido descubiertas, false en caso contrario.
 */
bool is_clue_satisfied(cell_t b[SIZE][SIZE], int r, int c) {
    int found = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0)
                continue;
            int nr = r + dr;
            int nc = c + dc;
            if (nr >= 0 && nr < SIZE && nc >= 0 && nc < SIZE) {
                if (b[nr][nc].state == REVEALED_FLOWER)
                    found++;
            }
        }
    }
    return found == b[r][c].clue_count;
}

/**
 * @brief Imprime en terminal una celda individual según su estado y formato ANSI.
 * @param b Matriz del tablero de juego.
 * @param r Coordenada de fila (0 a SIZE-1).
 * @param c Coordenada de columna (0 a SIZE-1).
 */
void print_cell(cell_t b[SIZE][SIZE], int r, int c) {
    switch (b[r][c].state) {
    case CLUE:
        if (is_clue_satisfied(b, r, c)) {
            // Pista resuelta: en gris atenuado
            printf(ANSI_GRAY "%d " ANSI_RESET, b[r][c].clue_count);
        } else {
            // Pista activa normal
            printf("%d ", b[r][c].clue_count);
        }
        break;
    case REVEALED_FLOWER:
        // Flor descubierta: en amarillo
        printf(ANSI_YELLOW "o " ANSI_RESET);
        break;
    case REVEALED_EMPTY:
        // Casilla vacía descubierta
        printf("x ");
        break;
    case HIDDEN:
    default:
        // Casilla incógnita
        printf(ANSI_GRAY ". " ANSI_RESET);
        break;
    }
}

/**
 * @brief Coordinadora: dibuja el tablero completo con cabeceras y estadísticas.
 * @param b Matriz del tablero de juego.
 * @param stats Puntero a las estadísticas de la partida (puede ser NULL).
 */
void print_board(cell_t b[SIZE][SIZE], stats_t *stats) {
    // Cabecera de columnas
    printf("\n   ");
    for (int c = 0; c < SIZE; c++) {
        printf(ANSI_CYAN "%d " ANSI_RESET, c);
    }
    printf("\n");

    // Contenido del tablero con índice de fila
    for (int r = 0; r < SIZE; r++) {
        printf(ANSI_CYAN "%d  " ANSI_RESET, r);
        for (int c = 0; c < SIZE; c++) {
            print_cell(b, r, c);
        }
        printf("\n");
    }

    // Resumen de estado de la partida
    if (stats) {
        printf("\nFlores encontradas: %d/%d  |  Fallos: %d/%d\n", stats->flowers,
               FLOWERS, stats->errors, MAX_ERRORS);
    }
}


/* ========================================================================== */
/* 4. MECÁNICAS Y LÓGICA DE JUGADA                                            */
/* ========================================================================== */

/**
 * @brief Valida si los datos y la celda elegida para la jugada son admisibles.
 * @param b Matriz del tablero de juego.
 * @param r Coordenada de fila.
 * @param c Coordenada de columna.
 * @param action Tipo de marca ('o' para flor, 'x' para vacía).
 * @return true si la jugada es válida, false en caso contrario.
 */
bool is_valid_move(cell_t b[SIZE][SIZE], int r, int c, char action) {
    bool is_flower = (action == 'o' || action == 'O');
    bool is_empty = (action == 'x' || action == 'X');

    if (!is_flower && !is_empty) {
        printf("Accion no valida. Usa 'o' para flor o 'x' para vacia.\n");
        return false;
    }
    if (r < 0 || r >= SIZE || c < 0 || c >= SIZE) {
        printf("Coordenadas fuera del tablero (0-%d).\n", SIZE - 1);
        return false;
    }
    if (b[r][c].state != HIDDEN && b[r][c].state != REVEALED_EMPTY) {
        printf("Esa casilla ya no es una incognita.\n");
        return false;
    }
    return true;
}

/**
 * @brief Procesa el intento de marcar una celda como flor ('o').
 * @param b Matriz del tablero de juego.
 * @param stats Puntero a las estadísticas de la partida.
 * @param r Coordenada de fila.
 * @param c Coordenada de columna.
 */
void mark_flower(cell_t b[SIZE][SIZE], stats_t *stats, int r, int c) {
    if (b[r][c].content == FLOWER) {
        b[r][c].state = REVEALED_FLOWER;
        stats->flowers++;
        printf("¡Acierto! Hay una flor en (%d, %d).\n", r, c);
    } else {
        stats->errors++;
        printf("¡Fallo! No habia flor en (%d, %d).\n", r, c);
    }
}

/**
 * @brief Procesa el intento de marcar o desmarcar una celda como vacía ('x').
 * @param b Matriz del tablero de juego.
 * @param stats Puntero a las estadísticas de la partida.
 * @param r Coordenada de fila.
 * @param c Coordenada de columna.
 */
void mark_empty(cell_t b[SIZE][SIZE], stats_t *stats, int r, int c) {
    // Si ya estaba marcada con 'x', se desmarca y vuelve a ser incógnita
    if (b[r][c].state == REVEALED_EMPTY) {
        b[r][c].state = HIDDEN;
        printf("Casilla (%d, %d) desmarcada.\n", r, c);
        return;
    }

    if (b[r][c].content == EMPTY) {
        b[r][c].state = REVEALED_EMPTY;
        printf("¡Acierto! Casilla vacia en (%d, %d).\n", r, c);
    } else {
        stats->errors++;
        printf("¡Fallo! Habia una flor en (%d, %d).\n", r, c);
    }
}

/**
 * @brief Coordinadora: valida y ejecuta la jugada elegida por el usuario.
 * @param b Matriz del tablero de juego.
 * @param stats Puntero a las estadísticas de la partida.
 * @param r Coordenada de fila elegida.
 * @param c Coordenada de columna elegida.
 * @param action Tipo de marca ('o' para flor, 'x' para vacía).
 * @return true si la jugada fue válida y procesada, false si los datos eran incorrectos.
 */
bool make_move(cell_t b[SIZE][SIZE], stats_t *stats, int r, int c,
               char action) {
    if (!is_valid_move(b, r, c, action))
        return false;

    if (action == 'o' || action == 'O')
        mark_flower(b, stats, r, c);
    else
        mark_empty(b, stats, r, c);

    return true;
}


/* ========================================================================== */
/* 5. BUCLE PRINCIPAL DE JUEGO Y ENTRADA                                      */
/* ========================================================================== */

/**
 * @brief Gestiona el bucle interactivo de la partida hasta la victoria o derrota.
 * @param b Matriz del tablero de juego.
 * @param stats Puntero a las estadísticas de la partida.
 */
void play(cell_t b[SIZE][SIZE], stats_t *stats) {
    char line[64];
    int r, c;
    char action;

    // Bucle principal del juego
    while (stats->errors < MAX_ERRORS && stats->flowers < FLOWERS) {
        print_board(b, stats);
        printf("\nIntroduce jugada (fila col o/x): ");
        if (!fgets(line, sizeof(line), stdin))
            break;

        if (sscanf(line, "%d %d %c", &r, &c, &action) != 3) {
            printf("Entrada invalida. Ejemplo de uso: 3 4 o\n");
            continue;
        }
        make_move(b, stats, r, c, action);
    }
}

/**
 * @brief Punto de entrada principal del programa.
 * @return 0 al finalizar la ejecución.
 */
int main(void) {
    cell_t board[SIZE][SIZE];
    stats_t stats = {0, 0};

    // Inicialización del juego y generación del tablero
    srand((unsigned int)time(NULL));
    init_board(board);
    compute_clues(board);

    // Mensaje de bienvenida e inicio de partida
    printf("=== SUNFLOWERS ===\n");
    printf("Descubre las %d flores. Maximo %d fallos permitidos.\n", FLOWERS,
           MAX_ERRORS);
    printf("Formato de jugada: <fila> <columna> <o/x> (ej: 2 4 o) o 'Ctrl+C' para salir.\n");
    play(board, &stats);

    // Fin de la partida: evaluación de victoria o derrota
    print_board(board, &stats);
    if (stats.flowers == FLOWERS) {
        printf("\n¡ENHORABUENA! ¡Has descubierto todas las flores!\n");
    } else {
        printf("\n¡HAS PERDIDO! Has alcanzado los %d fallos permitidos.\n\n",
               MAX_ERRORS);
        print_flowers_debug(board);
    }

    return 0;
}
