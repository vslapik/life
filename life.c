#include <assert.h>
#include <curses.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define ARRAY_LEN(a) (sizeof(a) / sizeof(a[0]))

static sig_atomic_t should_exit = false;
static bool board_loop = true;

static void step(int xmax, int ymax, char board[xmax][ymax])
{
    static const int d[][2] = {{-1, -1}, {-1, 0}, {-1, 1},
                               { 0, -1},          {0,  1},
                               { 1, -1}, { 1, 0}, {1,  1}};

    for (int i = 0; i < xmax; i++) {
        for (int j = 0; j < ymax; j++) {
            int c = 0;
            int ii, jj;
            for (int k = 0; k < ARRAY_LEN(d); k++) {
                ii = i + d[k][0];
                jj = j + d[k][1];
                if (board_loop) {
                    ii = (ii + xmax) % xmax;
                    jj = (jj + ymax) % ymax;
                }
                if ((ii >= 0) && (ii < xmax) &&
                    (jj >= 0) && (jj < ymax)) {
                    c += (board[ii][jj] & 0x01);
                }
            }

            if ((c == 3) ||
                (c == 2 && (board[i][j] == 1))) {
                board[i][j] |= 0x02;
            }
        }
    }

    for (int i = 0; i < xmax; i++) {
        for (int j = 0; j < ymax; j++) {
            board[i][j] >>= 1;
        }
    }
}

static void draw_board(int xmax, int ymax, char board[xmax][ymax])
{
    for (int i = 0; i < xmax; i++) {
        for (int j = 0; j < ymax; j++) {
            mvaddch(i, j, board[i][j] ? 'o' : ' '); // move and add a character
        }
    }
    refresh(); // perform actual drawing
}

static void put_glider(int x, int y, int xmax, int ymax,
                       char board[xmax][ymax])
{
    assert(x + 2 < xmax);
    assert(y + 2 < ymax);

    board[x + 2][y + 0] = 1;
    board[x + 0][y + 1] = 1;
    board[x + 2][y + 1] = 1;
    board[x + 1][y + 2] = 1;
    board[x + 2][y + 2] = 1;
}

static void put_gosper_gun(int x, int y, int xmax, int ymax,
                           char board[xmax][ymax])
{
    assert(x + 37 < xmax);
    assert(y + 10 < ymax);

    static const char gg[][2] = {
        {5, 1},  {6, 1},
        {5, 2},  {6, 2},
        {5, 11}, {6, 11}, {7, 11},
        {4, 12}, {8, 12},
        {3, 13}, {9, 13},
        {3, 14}, {9, 14},
        {6, 15},
        {4, 16}, {8, 16},
        {5, 17}, {6, 17}, {7, 17},
        {6, 18},
        {3, 21}, {4, 21}, {5, 21},
        {3, 22}, {4, 22}, {5, 22},
        {2, 23}, {6, 23},
        {1, 25}, {2, 25}, {6, 25}, {7, 25},
        {3, 35}, {4, 35},
        {3, 36}, {4, 36},
    };

    for (int i = 0; i < ARRAY_LEN(gg); i++) {
        board[x + gg[i][0]][y + gg[i][1]] = 1;
    }
}

static void fill_random(int xmax, int ymax, char board[xmax][ymax])
{
    for (int i = 0; i < xmax; i++) {
        for (int j = 0; j < ymax; j++) {
            board[i][j] = (random() % 2);
        }
    }
}

static void finish(int in)
{
    // Resizing is not supported.
    should_exit = true;
}

int main(void)
{
    srand(time(0));

    initscr(); // initialize the curses library
    cbreak(); // take input chars one at a time, no wait for \n
    noecho(); // no echo on input
    clear(); // clear the screen
    curs_set(0); // hide the cursor
    nodelay(stdscr, TRUE); //non-blocking input

    signal(SIGWINCH, finish);
    signal(SIGINT, finish);

    char board[LINES][COLS];
    long long delay = 500000;
    long long d = 0;

    fill_random(LINES, COLS, board);

    while (1) {

        draw_board(LINES, COLS, board);
        step(LINES, COLS, board);

        d = 0;
        do {
            int ch = getch(); // get input if any
            if (ch != ERR) {
                switch (ch) {
                    case 'l':
                        board_loop = !board_loop;
                        break;
                    case 'r':
                        fill_random(LINES, COLS, board);
                        draw_board(LINES, COLS, board);
                        break;
                    case 'q':
                        should_exit = true;
                        break;
                    case 'g':
                        put_glider(0, 0, LINES, COLS, board);
                        draw_board(LINES, COLS, board);
                        break;
                    case 's':
                        put_gosper_gun(0, 0, LINES, COLS, board);
                        draw_board(LINES, COLS, board);
                        break;
                    case 'c':
                        memset(board, 0, sizeof(board));
                        draw_board(LINES, COLS, board);
                        break;
                    case '-':
                        delay *= 2;
                        break;
                    case '+':
                        delay /= 2;
                        delay += 1;
                        break;
                }
                d = delay + 1;
            }
            usleep(delay > 10000 ? 10000 : delay);
            d += 10000;
            if (should_exit) {
                goto exit;
            }
        } while (d < delay);
    }

exit:
    endwin();
    exit(EXIT_SUCCESS);
}
