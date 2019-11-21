#include <assert.h>
#include <curses.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define LOOP
#define ARRAY_LEN(a) (sizeof(a) / sizeof(a[0]))

static sig_atomic_t should_exit = false;

static int step(int xmax, int ymax, char board[][xmax])
{
    static const int d[][2] = {{-1, -1}, {-1, 0}, {-1, 1},
                               { 0, -1},          {0,  1},
                               { 1, -1}, { 1, 0}, {1,  1}};

    for (int i = 0; i < ymax; i++) {
        for (int j = 0; j < xmax; j++) {
            int c = 0;
            int ii;
            int jj;
            for (int k = 0; k < ARRAY_LEN(d); k++) {
                ii = i + d[k][0];
                jj = j + d[k][1];
#ifdef LOOP
                ii = (ii + ymax) % ymax;
                jj = (jj + xmax) % xmax;
#endif
                if ((ii >= 0) && (ii < ymax) && (jj >= 0) && (jj < xmax)) {
                    c += (board[ii][jj] & 0x01);
                }
            }

            if (c == 3) {
                board[i][j] |= 0x02;
            } else if (c == 2 && (board[i][j] == 1)) {
                board[i][j] |= 0x02;
            }
        }
    }

    for (int i = 0; i < ymax; i++) {
        for (int j = 0; j < xmax; j++) {
            board[i][j] >>= 1;
        }
    }
}

static void put_glider(int x, int y, int xmax, int ymax, char board[][xmax])
{
    assert(y + 2 < ymax);
    assert(x + 2 < xmax);

    board[y + 2][x + 0] = 1;
    board[y + 2][x + 1] = 1;
    board[y + 2][x + 2] = 1;
    board[y + 1][x + 2] = 1;
    board[y + 0][x + 1] = 1;
}

void sig_winch(int in)
{
    // Resizing is not supported.
    should_exit = true;
}

int main(void)
{
    initscr();
    cbreak();
    noecho();
    clear();
    curs_set(0);
    nodelay(stdscr, TRUE);
    signal(SIGWINCH, sig_winch);

    char board[LINES][COLS];

    for (int i = 0; i < LINES; i++) {
        for (int j = 0; j < COLS; j++) {
            board[i][j] = (random() % 2);
        }
    }

    while (1) {
        step(COLS, LINES, board);
        refresh();
        for (int i = 0; i < LINES; i++) {
            for (int j = 0; j < COLS; j++) {
                if (should_exit) {
                    goto exit;
                }
                mvaddch(i, j, board[i][j] ? 'o' : ' ');
            }
        }
        int ch = getch();
        if (ch != ERR) {
            put_glider(0, 0, COLS, LINES, board);
        }
//        usleep(20000);
        usleep(2000);
    }

exit:

    getch();
    endwin();

    exit(0);
}
