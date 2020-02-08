#include <stdint.h>

#ifndef GRIDUTILS_H
#define CELL_TYPE int
#include "grid_utils.h"
#endif

extern int HEIGHT, WIDTH;

#ifdef NOTCURSES

#include <notcurses.h>
#include <locale.h>

static notcurses_options ncopt;
static struct notcurses* nc;
static struct ncplane* stdplane;
int colors[7][3] ={
    {.3*256,  0,        0},
    {.5*256,  0,        0},
    {.7*256,  .1*256,   0},
    {.9*256,  .3*256,   0},
    {255,     .5*256,   .1*256},
    {255,     .8*256,   .5*256},
    {255,     255,      255}
};

void printframe(ca_grid* field, char dispch, int maxtemp, int heightrecord)
{
    int PALETTE_SZ = 7;
    char c;
    cell* current;
    int color;
    // On the first run, heightrecord is set to 0, so the whole frame gets drawn. On subsequent
    // frames, only the lines that are below the heightrecord get drawn.
    for (int i = heightrecord; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            color = MIN(PALETTE_SZ, (PALETTE_SZ * IDX(field, i, j) / maxtemp) + 1);
            c = IDX(field, i, j) == 0 ? ' ' : dispch;
            ncplane_at_yx(stdplane, i, j, current);
            cell_init(current);
            cell_load(stdplane, current, &c);
            cell_set_fg_rgb(current, colors[color][0], colors[color][1], colors[color][2]);
            cell_release(stdplane, current);
        }
    }
    notcurses_render(nc);
}

void clearscreen()
{
    ncplane_erase(stdplane);
}

int getchar(){
    return notcurses_getc_nblock(nc, NULL);
}

void get_screen_sz(int* h, int* w)
{
    ncplane_dim_yx(stdplane, h, w);
}

void resize(int h, int w)
{

}

void begin_draw()
{
    setlocale(LC_ALL, "");
    memset(&ncopt, 0, sizeof(ncopt));
    nc = notcurses_init(&ncopt, stdout);
    stdplane = notcurses_stdplane(nc);
}

void end_draw()
{
    notcurses_stop(nc);
}

#else
#include <ncurses.h>

//---------------------------------------[Ncurses functions]----------------------------------------


static int PALETTE_SZ;      //Number of flame colors used
typedef struct colorvalstruct{ short r,g,b; } color_val;
color_val* colors;

int getchar(){
    return getch();
}

void start_ncurses(color_val* colors)
{
    initscr();
    start_color();
    if (COLORS < 256){
        for (int i = 0; i < 8; i++) {
            color_content(i, &colors[i].r, &colors[i].g, &colors[i].b);
        }
        PALETTE_SZ = 7;
        init_color(COLOR_BLACK,    100,   100,   100);
        init_color(COLOR_RED,      300,   0,     0);
        init_color(COLOR_GREEN,    500,   0,     0);
        init_color(COLOR_BLUE,     700,   100,   0);
        init_color(COLOR_YELLOW,   900,   300,   0);
        init_color(COLOR_MAGENTA,  1000,  500,   100);
        init_color(COLOR_CYAN,     1000,  800,   500);
        init_color(COLOR_WHITE,    1000,  1000,  1000);

        init_pair(1,  COLOR_RED,      COLOR_BLACK);
        init_pair(2,  COLOR_GREEN,    COLOR_BLACK);
        init_pair(3,  COLOR_BLUE,     COLOR_BLACK);
        init_pair(4,  COLOR_YELLOW,   COLOR_BLACK);
        init_pair(5,  COLOR_MAGENTA,  COLOR_BLACK);
        init_pair(6,  COLOR_CYAN,     COLOR_BLACK);
        init_pair(7,  COLOR_WHITE,    COLOR_BLACK);
    }
    else {
        // A decent gradient from the X11 256 color palette
        const int x256[] = {
            233,  52,  88, 124,
            160, 166, 202, 208,
            214, 220, 226, 227,
            228, 229, 230, 231};
        PALETTE_SZ = sizeof(x256)/sizeof(int);
        // the first color in the list will be the background, so we start at 1.
        for(size_t i = 1; i < PALETTE_SZ; i++){
            init_pair(i, x256[i], x256[0]);
        }
        PALETTE_SZ -= 1;
    }
    curs_set(0);    //invisible cursor
    timeout(0);     //make getch() non-blocking

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    getmaxyx(stdscr, HEIGHT, WIDTH);
}

void restore_colors(color_val* colors)
{
    for (int i = 0; i < 8; i++) {
        init_color(i, colors[i].r, colors[i].g, colors[i].b);
    }
}

void printframe(ca_grid* field, char dispch, int maxtemp, int heightrecord)
{
    int color;
    // On the first run, heightrecord is set to 0, so the whole frame gets drawn. On subsequent
    // frames, only the lines that are below the heightrecord get drawn.
    for (int i = heightrecord; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            move(i,j);
            color = MIN(PALETTE_SZ, (PALETTE_SZ * IDX(field, i, j) / maxtemp) + 1);
            attron(COLOR_PAIR(color));
            //if the cell is cold, print a space, otherwise print [dispch]
            addch(IDX(field, i, j) == 0 ? ' ' : dispch);
            attroff(COLOR_PAIR(color));
        }
    }
    refresh();
}

void get_screen_sz(int* h, int* w)
{
    int y, x;
    getmaxyx(stdscr, y, x);
    *h = y;
    *w = x;
}

void resize(int h, int w)
{
    resizeterm(h, w);
}

void clearscreen()
{
    endwin();
    refresh();
}

void begin_draw()
{
    colors = malloc(8 * sizeof(color_val));
    start_ncurses(colors);
}

void end_draw()
{
    if(COLORS < 256){
        restore_colors(colors);
    }
    free(colors);
    clear();
    refresh();
    endwin();
}

#endif //#ifdef  NOTCURSES
