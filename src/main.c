#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

#define CL_DEFAULT 1
#define CL_FIELD_MINE 10
#define CL_FIELD_MINE_FLAG 11
#define CL_FIELD_FLAG 12
#define CL_FIELD_CUR 13
#define CL_FIELD_NONE 14
#define CL_FIELD_OPEN 15
#define CL_FIELD_OPEN_1 16
#define CL_FIELD_OPEN_2 17
#define CL_FIELD_OPEN_3 18
#define CL_MENU 20

#define ST_GAME 0
#define ST_LOSE 1
#define ST_WIN 2

#define POS_VALID(x, y) ((x) >= 0 && (x) <= size_x-1 && (y) >= 0 && (y) <= size_y-1)

typedef struct {
    bool mine, flag, open;
} point;

bool quit = false;
unsigned int scr_x, scr_y;
unsigned int size_x, size_y, mines;
unsigned int cur_x, cur_y, st_game;
time_t time_start = NULL, time_end = NULL;
point **field = NULL;

// Generate field
void gen_field() {
    time_start = time(NULL);

    srand((unsigned int) time(0));
    field = calloc(size_x, sizeof(point *));

    for (int i = 0; i < size_x; i++) {
        field[i] = calloc(size_y, sizeof(point));

        for (int j = 0; j < size_y; j++) {
            field[i][j].mine = false;
            field[i][j].flag = false;
            field[i][j].open = false;
        }
    }

    for (int i = 0; i < mines; i++) {
        int x = rand() % size_x;
        int y = rand() % size_y;

        if (x == cur_x || y == cur_y || field[x][y].mine) i--;
        else field[x][y].mine = true;
    }
}

// Count of around mines
int mines_around(int x, int y, bool open) {
    int c = 0;

    // LEFT
    if (POS_VALID(x - 1, y - 1) && (!field[x - 1][y - 1].open || open) && field[x - 1][y - 1].mine) c++;
    if (POS_VALID(x, y - 1) && (!field[x][y - 1].open || open) && field[x][y - 1].mine) c++;
    if (POS_VALID(x + 1, y - 1) && (!field[x + 1][y - 1].open || open) && field[x + 1][y - 1].mine) c++;

    // Center
    if (POS_VALID(x - 1, y) && (!field[x - 1][y].open || open) && field[x - 1][y].mine) c++;
    if (POS_VALID(x + 1, y) && (!field[x + 1][y].open || open) && field[x + 1][y].mine) c++;

    // RIGHT
    if (POS_VALID(x - 1, y + 1) && (!field[x - 1][y + 1].open || open) && field[x - 1][y + 1].mine) c++;
    if (POS_VALID(x, y + 1) && (!field[x][y + 1].open || open) && field[x][y + 1].mine) c++;
    if (POS_VALID(x + 1, y + 1) && (!field[x + 1][y + 1].open || open) && field[x + 1][y + 1].mine) c++;

    return c;
}

// Count of around flags
int flags_around(int x, int y) {
    int c = 0;

    // LEFT
    if (POS_VALID(x - 1, y - 1) && !field[x - 1][y - 1].open && field[x - 1][y - 1].flag) c++;
    if (POS_VALID(x, y - 1) && !field[x][y - 1].open && field[x][y - 1].flag) c++;
    if (POS_VALID(x + 1, y - 1) && !field[x + 1][y - 1].open && field[x + 1][y - 1].flag) c++;

    // Center
    if (POS_VALID(x - 1, y) && !field[x - 1][y].open && field[x - 1][y].flag) c++;
    if (POS_VALID(x + 1, y) && !field[x + 1][y].open && field[x + 1][y].flag) c++;

    // RIGHT
    if (POS_VALID(x - 1, y + 1) && !field[x - 1][y + 1].open && field[x - 1][y + 1].flag) c++;
    if (POS_VALID(x, y + 1) && !field[x][y + 1].open && field[x][y + 1].flag) c++;
    if (POS_VALID(x + 1, y + 1) && !field[x + 1][y + 1].open && field[x + 1][y + 1].flag) c++;

    return c;
}

// Open all mines
void mine_open_all() {
    for (int i = 0; i < size_x; i++) {
        for (int j = 0; j < size_y; j++) {
            if (field[i][j].mine)
                field[i][j].open = true;
        }
    }
}

// Open mine
void mine_open(int x, int y) {
    if (!field[x][y].open && field[x][y].flag) return;
    field[x][y].open = true;

    // Check
    int c = 0;
    for (int i = 0; i < size_x; i++) {
        for (int j = 0; j < size_y; j++) {
            if ((field[i][j].mine && !field[i][j].open) || (!field[i][j].mine && field[i][j].open)) c++;
        }
    }

    const int mar = mines_around(x, y, false);

    if (field[x][y].mine) {
        mine_open_all();
        st_game = ST_LOSE;
    } else if (c == size_x * size_y) {
        mine_open_all();
        st_game = ST_WIN;
    } else if (mar == 0) {
        if (POS_VALID(x - 1, y - 1) && !field[x - 1][y - 1].open && !field[x - 1][y - 1].mine) mine_open(x - 1, y - 1);
        if (POS_VALID(x, y - 1) && !field[x][y - 1].open && !field[x][y - 1].mine) mine_open(x, y - 1);
        if (POS_VALID(x + 1, y - 1) && !field[x + 1][y - 1].open && !field[x + 1][y - 1].mine) mine_open(x + 1, y - 1);

        if (POS_VALID(x - 1, y) && !field[x - 1][y].open && !field[x - 1][y].mine) mine_open(x - 1, y);
        if (POS_VALID(x + 1, y) && !field[x + 1][y].open && !field[x + 1][y].mine) mine_open(x + 1, y);

        if (POS_VALID(x - 1, y + 1) && !field[x - 1][y + 1].open && !field[x - 1][y + 1].mine) mine_open(x - 1, y + 1);
        if (POS_VALID(x, y + 1) && !field[x][y + 1].open && !field[x][y + 1].mine) mine_open(x, y + 1);
        if (POS_VALID(x + 1, y + 1) && !field[x + 1][y + 1].open && !field[x + 1][y + 1].mine) mine_open(x + 1, y + 1);
    } else if (mar == flags_around(x, y)) {
        if (POS_VALID(x - 1, y - 1) && !field[x - 1][y - 1].open && !field[x - 1][y - 1].flag) mine_open(x - 1, y - 1);
        if (POS_VALID(x, y - 1) && !field[x][y - 1].open && !field[x][y - 1].flag) mine_open(x, y - 1);
        if (POS_VALID(x + 1, y - 1) && !field[x + 1][y - 1].open && !field[x + 1][y - 1].flag) mine_open(x + 1, y - 1);

        if (POS_VALID(x - 1, y) && !field[x - 1][y].open && !field[x - 1][y].flag) mine_open(x - 1, y);
        if (POS_VALID(x + 1, y) && !field[x + 1][y].open && !field[x + 1][y].flag) mine_open(x + 1, y);

        if (POS_VALID(x - 1, y + 1) && !field[x - 1][y + 1].open && !field[x - 1][y + 1].flag) mine_open(x - 1, y + 1);
        if (POS_VALID(x, y + 1) && !field[x][y + 1].open && !field[x][y + 1].flag) mine_open(x, y + 1);
        if (POS_VALID(x + 1, y + 1) && !field[x + 1][y + 1].open && !field[x + 1][y + 1].flag) mine_open(x + 1, y + 1);
    }
}

void render_border(int y1, int x1, int y2, int x2) {
    mvhline(y1, x1, 0, x2 - x1);
    mvhline(y2, x1, 0, x2 - x1);
    mvvline(y1, x1, 0, y2 - y1);
    mvvline(y1, x2, 0, y2 - y1);
    mvaddch(y1, x1, ACS_ULCORNER);
    mvaddch(y2, x1, ACS_LLCORNER);
    mvaddch(y1, x2, ACS_URCORNER);
    mvaddch(y2, x2, ACS_LRCORNER);
}

// Render mines field
void render() {
    clear();
    bkgd(COLOR_PAIR(CL_DEFAULT));
    getmaxyx(stdscr, scr_x, scr_y);

    const int px = scr_x / 2 - size_x / 2 - 2, py = scr_y / 2 - size_y - 2;

    // Count of flags
    int flags = 0;
    if (field != NULL)
        for (int i = 0; i < size_x; i++)
            for (int j = 0; j < size_y; j++)
                if ((!field[i][j].open || (field[i][j].open && field[i][j].mine)) && field[i][j].flag)
                    flags++;

    render_border(px - 1, py - 1, px + size_x, py + size_y * 2);

    for (int i = 0; i < size_x; i++) {
        for (int j = 0; j < size_y; j++) {
            bool cur = cur_x == i && cur_y == j;

            if (field == NULL) { // Null field
                attron(COLOR_PAIR(cur ? CL_FIELD_CUR : CL_FIELD_NONE));
                mvprintw(px + i, py + j * 2, "  ");
                continue;
            } else if (field[i][j].open && field[i][j].mine) { // Open mine
                attron(COLOR_PAIR(cur ? CL_FIELD_CUR : (field[i][j].flag ? CL_FIELD_MINE_FLAG : CL_FIELD_MINE)));
                mvprintw(px + i, py + j * 2, "* ");
            } else if (field[i][j].open) { // Open square
                int mines = mines_around(i, j, true);

                if (mines == 0) attron(COLOR_PAIR(cur ? CL_FIELD_CUR : CL_FIELD_OPEN));
                if (mines == 1) attron(COLOR_PAIR(cur ? CL_FIELD_CUR : CL_FIELD_OPEN_1));
                if (mines == 2) attron(COLOR_PAIR(cur ? CL_FIELD_CUR : CL_FIELD_OPEN_2));
                if (mines >= 3) attron(COLOR_PAIR(cur ? CL_FIELD_CUR : CL_FIELD_OPEN_3));

                char str[3] = "\0";
                str[0] = (char) (mines == 0 ? ' ' : (char) (mines + '0'));
                str[1] = ' ';
                mvprintw(px + i, py + j * 2, str);
            } else if (field[i][j].flag) { // Flag
                attron(COLOR_PAIR(cur ? CL_FIELD_CUR : CL_FIELD_FLAG));
                mvprintw(px + i, py + j * 2, "FF");
            } else { // Closed square
                attron(COLOR_PAIR(cur ? CL_FIELD_CUR : CL_FIELD_NONE));
                mvprintw(px + i, py + j * 2, "  ");
            }
        }

        attron(COLOR_PAIR(CL_DEFAULT));
    }

    mvprintw(px - 1, py, "%03d", time_start == NULL ? 0 : time(NULL)-time_start);
    mvprintw(px - 1, py + size_y * 2 - 2, "%02d", mines - flags);

    // Status message
    render_border(px + size_x + 1, py - 11, px + size_x + 3, py + size_y * 2 + 10);
    char *message;
    switch (st_game) {
        case ST_LOSE:
            message = "You lose. Press any key to menu";
            break;

        case ST_WIN:
            message = "You win! Press any key to menu";
            break;

        default:
            message = "flag(f) open(space) quit(q) menu(m)";
            break;
    }
    mvprintw(px + size_x + 2, py - 9, "%-35s", message);

    refresh();
}

// Play game
void start_game() {
    st_game = 0, cur_x = 0, cur_y = 0, field = NULL, time_start = NULL;
    render();
    timeout(100);

    // Game doing
    do {
        switch (getch()) {
            case 65: // UP
                if (cur_x > 0) cur_x--;
                break;

            case 66: // DOWN
                if (cur_x < size_x - 1) cur_x++;
                break;

            case 67: // RIGHT
                if (cur_y < size_y - 1) cur_y++;
                break;

            case 68: // LEFT
                if (cur_y > 0) cur_y--;
                break;

            case 32: // SPACE
                if (field == NULL) gen_field();
                mine_open(cur_x, cur_y);
                break;

            case 'f': // FLAG
                if (field == NULL) break;
                field[cur_x][cur_y].flag = !field[cur_x][cur_y].flag;
                break;

            case 'q': // QUIT
                quit = true;
                return;

            case 'm': // MENU
                return;

            default:
                break;
        }

        render();
    } while (st_game == ST_GAME);

    timeout(-1);
    getch();
}

int main(int argc, char *argv[]) {
    initscr();
    noecho();
    curs_set(FALSE);

    if (has_colors() == FALSE) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    start_color();

    // Color pairs
    init_pair(CL_DEFAULT, COLOR_WHITE, COLOR_BLACK);
    init_pair(CL_FIELD_FLAG, COLOR_WHITE, COLOR_YELLOW);
    init_pair(CL_FIELD_MINE, COLOR_WHITE, COLOR_RED);
    init_pair(CL_FIELD_MINE_FLAG, COLOR_WHITE, COLOR_GREEN);
    init_pair(CL_FIELD_CUR, COLOR_BLACK, COLOR_CYAN);
    init_pair(CL_FIELD_NONE, COLOR_WHITE, COLOR_BLACK);
    init_pair(CL_FIELD_OPEN, COLOR_BLACK, COLOR_WHITE);
    init_pair(CL_FIELD_OPEN_1, COLOR_BLUE, COLOR_WHITE);
    init_pair(CL_FIELD_OPEN_2, COLOR_GREEN, COLOR_WHITE);
    init_pair(CL_FIELD_OPEN_3, COLOR_RED, COLOR_WHITE);
    init_pair(CL_MENU, COLOR_WHITE, COLOR_BLUE);
    bkgd(COLOR_PAIR(CL_DEFAULT));

    // Main menu
    int menu_pos = 0, menu_size = 5;
    timeout(100);
    while (!quit) {
        clear();
        bkgd(COLOR_PAIR(CL_DEFAULT));
        getmaxyx(stdscr, scr_x, scr_y);

        const int menu_x = 5, menu_y = 15;
        const int px = scr_x / 2 - menu_x / 2 - 2, py = scr_y / 2 - menu_y - 2;

        render_border(px - 1, py - 1, px + menu_x + 1, py + menu_y + 1);
        mvprintw(px - 1, py + 2, "nMines 0.1.0");

        attron(A_BOLD);
        mvprintw(px, py, "   Difficulty   ");
        attroff(A_BOLD);

        if (menu_pos == 0) attron(COLOR_PAIR(CL_MENU));
        mvprintw(px + 1, py, "   Easy (8x8)   ");
        attron(COLOR_PAIR(CL_DEFAULT));

        if (menu_pos == 1) attron(COLOR_PAIR(CL_MENU));
        mvprintw(px + 2, py, " Medium (16x16) ");
        attron(COLOR_PAIR(CL_DEFAULT));

        if (menu_pos == 2) attron(COLOR_PAIR(CL_MENU));
        mvprintw(px + 3, py, "  Hard (32x16)  ");
        attron(COLOR_PAIR(CL_DEFAULT));

        if (menu_pos == 3) attron(COLOR_PAIR(CL_MENU));
        mvprintw(px + 4, py, " Expert (32x16) ");
        attron(COLOR_PAIR(CL_DEFAULT));

        if (menu_pos == 4) attron(COLOR_PAIR(CL_MENU));
        mvprintw(px + 5, py, "      Exit      ");
        attron(COLOR_PAIR(CL_DEFAULT));

        refresh();

        // Switches
        switch (getch()) {
            case 65: // UP
                if (menu_pos > 0) menu_pos--;
                break;

            case 66: // DOWN
                if (menu_pos < menu_size - 1) menu_pos++;
                break;

            case 32: // SPACE
                if (menu_pos == 4) { quit = true; break; }
                else if (menu_pos == 0) size_x = 8, size_y = 8, mines = 10;
                else if (menu_pos == 1) size_x = 16, size_y = 16, mines = 40;
                else if (menu_pos == 2) size_x = 16, size_y = 32, mines = 70;
                else if (menu_pos == 3) size_x = 16, size_y = 32, mines = 99;
                start_game();
                break;

            default:
                break;
        }
    }

    curs_set(TRUE);
    endwin();
    return 0;
}
