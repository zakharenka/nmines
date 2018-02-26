#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define CL_TEXT 1
#define CL_FIELD_MINE 10
#define CL_FIELD_FLAG 11
#define CL_FIELD_SEL 12

// Window
WINDOW *scr;
uint8_t scr_x, scr_y;

// Mines settings
const uint8_t size_x = 8;
const uint8_t size_y = 8;
const uint8_t mines = 10;

typedef struct {
    bool mine, flag, sel;
} point;

point **field;

void gen_field() {
    srand(time(NULL));

    field = calloc(size_x, sizeof(point *));

    for (uint8_t i = 0; i < size_x; i++) {
        field[i] = calloc(size_y, sizeof(point));

        for (uint8_t j = 0; j < size_y; j++) {
            field[i][j].mine = false;
            field[i][j].flag = false;
            field[i][j].sel = false;
        }
    }

    for (uint8_t i = 0; i < mines; i++) {
        uint8_t x = (uint8_t) (rand() % size_x);
        uint8_t y = (uint8_t) (rand() % size_y);

        if (field[x][y].mine) i--;
        field[x][y].mine = true;
    }
}

uint8_t mines_around(uint8_t x, uint8_t y) {
    uint8_t c = 0;

    // LEFT
    if (x > 0 && y > 0 && field[x - 1][y - 1].mine) c++;
    if (y > 0 && field[x][y - 1].mine) c++;
    if (x < size_x - 1 && y > 0 && field[x + 1][y - 1].mine) c++;

    // Center
    if (x > 0 && field[x - 1][y].mine) c++;
    if (x < size_x - 1 && field[x + 1][y].mine) c++;

    // RIGHT
    if (x > 0 && y < size_y - 1 && field[x - 1][y + 1].mine) c++;
    if (y < size_y - 1 && field[x][y + 1].mine) c++;
    if (x < size_x - 1 && y < size_y - 1 && field[x + 1][y + 1].mine) c++;

    return c;
}

void render_field(uint8_t px, uint8_t py) {
    for (uint8_t i = 0; i < size_x; i++) {
        for (uint8_t j = 0; j < size_y; j++) {
            if (field[i][j].mine) {
                attron(COLOR_PAIR(CL_FIELD_MINE));

                mvprintw(px + i, py + j * 2, "* ");
            } else {
                attron(COLOR_PAIR(CL_FIELD_SEL));

                uint8_t mines = mines_around(i, j);
                char str[3] = "\0";
                str[0] = (char) (mines == 0 ? ' ' : (char) (mines + '0'));
                str[1] = ' ';
                mvprintw(px + i, py + j * 2, str);
            }
        }
    }
}

int main(uint8_t argc, char *argv[]) {
    scr = initscr();
    noecho();
    curs_set(FALSE);
    getmaxyx(scr, scr_x, scr_y);

    if (has_colors() == FALSE) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    start_color();

    // Color pairs
    init_pair(CL_TEXT, COLOR_WHITE, COLOR_BLACK);
    init_pair(CL_FIELD_FLAG, COLOR_WHITE, COLOR_YELLOW);
    init_pair(CL_FIELD_MINE, COLOR_WHITE, COLOR_RED);
    init_pair(CL_FIELD_SEL, COLOR_WHITE, COLOR_CYAN);

    gen_field();
    render_field(2, 3);

    printf("%dx%d", scr_x, scr_y);
    refresh();

    sleep(100);

    endwin(); // Restore normal terminal behavior
}