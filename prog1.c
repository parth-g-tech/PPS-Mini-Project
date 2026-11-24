/*
 * Tic-Tac-Toe in C
 * Modes: Player vs Player, Player vs Computer (unbeatable minimax AI)
 *
 * C concepts used: structures, typedef, enums, 2D arrays, pointers,
 * recursion (minimax), strings and fgets/sscanf input, function pointers.
 *
 * Compile: gcc -Wall -o tictactoe tictactoe.c
 * Run:     ./tictactoe
 */

#include <stdio.h>
#include <string.h>


#define SIZE 3

typedef enum { EMPTY = ' ', PLAYER_X = 'X', PLAYER_O = 'O' } Cell;
typedef enum { ONGOING, X_WINS, O_WINS, DRAW } Status;

typedef struct {
    Cell board[SIZE][SIZE];
    Cell current;   /* whose turn it is */
    int moves;      /* number of moves played */
} Game;

/* ---------- Game state helpers ---------- */

void init_game(Game *g) {
    for (int r = 0; r < SIZE; r++)
        for (int c = 0; c < SIZE; c++)
            g->board[r][c] = EMPTY;
    g->current = PLAYER_X;
    g->moves = 0;
}

void print_board(const Game *g) {
    printf("\n");
    for (int r = 0; r < SIZE; r++) {
        printf(" ");
        for (int c = 0; c < SIZE; c++) {
            if (g->board[r][c] == EMPTY)
                printf("%d", r * SIZE + c + 1);   /* show cell number */
            else
                printf("%c", g->board[r][c]);
            if (c < SIZE - 1) printf(" | ");
        }
        printf("\n");
        if (r < SIZE - 1) printf("---+---+---\n");
    }
    printf("\n");
}

Status get_status(const Game *g) {
    /* rows and columns */
    for (int i = 0; i < SIZE; i++) {
        if (g->board[i][0] != EMPTY &&
            g->board[i][0] == g->board[i][1] && g->board[i][1] == g->board[i][2])
            return g->board[i][0] == PLAYER_X ? X_WINS : O_WINS;
        if (g->board[0][i] != EMPTY &&
            g->board[0][i] == g->board[1][i] && g->board[1][i] == g->board[2][i])
            return g->board[0][i] == PLAYER_X ? X_WINS : O_WINS;
    }
    /* diagonals */
    if (g->board[1][1] != EMPTY) {
        if ((g->board[0][0] == g->board[1][1] && g->board[1][1] == g->board[2][2]) ||
            (g->board[0][2] == g->board[1][1] && g->board[1][1] == g->board[2][0]))
            return g->board[1][1] == PLAYER_X ? X_WINS : O_WINS;
    }
    return g->moves == SIZE * SIZE ? DRAW : ONGOING;
}

int make_move(Game *g, int cell) {   /* cell is 1..9; returns 1 on success */
    if (cell < 1 || cell > SIZE * SIZE) return 0;
    int r = (cell - 1) / SIZE, c = (cell - 1) % SIZE;
    if (g->board[r][c] != EMPTY) return 0;
    g->board[r][c] = g->current;
    g->moves++;
    g->current = (g->current == PLAYER_X) ? PLAYER_O : PLAYER_X;
    return 1;
}

/* ---------- Computer player (minimax) ---------- */

/* Computer plays O and maximizes; X minimizes. */
static int minimax(Game *g, int depth, int is_max) {
    Status s = get_status(g);
    if (s == O_WINS) return 10 - depth;
    if (s == X_WINS) return depth - 10;
    if (s == DRAW)   return 0;

    int best = is_max ? -100 : 100;
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            if (g->board[r][c] != EMPTY) continue;
            g->board[r][c] = is_max ? PLAYER_O : PLAYER_X;
            g->moves++;
            int score = minimax(g, depth + 1, !is_max);
            g->board[r][c] = EMPTY;   /* undo */
            g->moves--;
            if (is_max ? score > best : score < best) best = score;
        }
    }
    return best;
}

int computer_pick(Game *g) {
    int best_score = -100, best_cell = -1;
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            if (g->board[r][c] != EMPTY) continue;
            g->board[r][c] = PLAYER_O;
            g->moves++;
            int score = minimax(g, 1, 0);
            g->board[r][c] = EMPTY;
            g->moves--;
            if (score > best_score) {
                best_score = score;
                best_cell = r * SIZE + c + 1;
            }
        }
    }
    return best_cell;
}

/* ---------- Move sources (function pointers) ---------- */

typedef int (*MoveFn)(Game *g);

int human_move(Game *g) {
    char line[32];
    int cell;
    while (1) {
        printf("Player %c, choose a cell (1-9): ", (char)g->current);
        if (!fgets(line, sizeof line, stdin)) return -1;   /* EOF */
        if (sscanf(line, "%d", &cell) == 1) {
            int r = (cell - 1) / SIZE, c = (cell - 1) % SIZE;
            if (cell >= 1 && cell <= 9 && g->board[r][c] == EMPTY)
                return cell;
        }
        printf("Invalid move, try again.\n");
    }
}

int ai_move(Game *g) {
    int cell = computer_pick(g);
    printf("Computer (O) chooses %d\n", cell);
    return cell;
}

/* ---------- Main loop ---------- */

int ask_mode(void) {
    char line[32];
    int mode;
    while (1) {
        printf("=== TIC-TAC-TOE ===\n");
        printf("1. Player vs Player\n");
        printf("2. Player vs Computer\n");
        printf("Choose mode: ");
        if (!fgets(line, sizeof line, stdin)) return -1;
        if (sscanf(line, "%d", &mode) == 1 && (mode == 1 || mode == 2))
            return mode;
        printf("Please enter 1 or 2.\n\n");
    }
}

int play_again(void) {
    char line[32];
    printf("Play again? (y/n): ");
    if (!fgets(line, sizeof line, stdin)) return 0;
    return line[0] == 'y' || line[0] == 'Y';
}

int main(void) {
    int mode = ask_mode();
    if (mode < 0) return 0;

    do {
        Game game;
        init_game(&game);

        /* pick who controls X and O */
        MoveFn players[2] = { human_move, mode == 2 ? ai_move : human_move };

        while (get_status(&game) == ONGOING) {
            print_board(&game);
            MoveFn fn = players[game.current == PLAYER_X ? 0 : 1];
            int cell = fn(&game);
            if (cell < 0) return 0;   /* input closed */
            make_move(&game, cell);
        }

        print_board(&game);
        switch (get_status(&game)) {
            case X_WINS: printf("Player X wins!\n"); break;
            case O_WINS: printf("%s wins!\n", mode == 2 ? "Computer (O)" : "Player O"); break;
            default:     printf("It's a draw!\n");
        }
    } while (play_again());

    printf("Thanks for playing!\n");
    return 0;
}