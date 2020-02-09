/***************************************************************************************************
*                                                                                                  *
*       oooooooooooo  o8o                                oooo                                      *
*       `888'     `8  `"'                                `888                                      *
*        888         oooo  oooo d8b  .ooooo.  oo.ooooo.   888   .oooo.    .ooooo.   .ooooo.        *
*        888oooo8    `888  `888""8P d88' `88b  888' `88b  888  `P  )88b  d88' `"Y8 d88' `88b       *
*        888    "     888   888     888ooo888  888   888  888   .oP"888  888       888ooo888       *
*        888          888   888     888    .o  888   888  888  d8(  888  888   .o8 888    .o       *
*       o888o        o888o d888b    `Y8bod8P'  888bod8P' o888o `Y888""8o `Y8bod8P' `Y8bod8P'       *
*                                              888                                                 *
*                                             o888o                                                *
*                                                                                                  *
*                                                                                                  *
*   File:      main.c                                                                              *
*   Author:    Wyatt Sheffield                                                                     *
*                                                                                                  *
*   Lights a cozy fire in your terminal. Goes well with coffee.                                    *
*                                                                                                  *
*                   Copyright (c) 2017 Wyatt Sheffield and GitHub contributors                     *
*                                                                                                  *
***************************************************************************************************/

#include <time.h>
#include <signal.h>
#include <stdlib.h> //random
#include <unistd.h> //usleep, getopt
#include <stdint.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h> //Sleep
#endif

//the type of cell MUST be defined before including grid_utils.
#define CELL_TYPE int
#include "grid_utils.h"
#include "draw.h"


//----------------------------------------[Global variables]----------------------------------------

int WIDTH, HEIGHT;   // size of the terminal
static int heightrecord=0;  // highest point the flames reached last frame
static volatile sig_atomic_t sig_caught = 0;

//---------------------------------------[Memory Management]----------------------------------------

// Most of the interesting stuff is going on at the bottom of the screen. We will make sure all
// those goodies get preserved on a window resize by flipping the screen upside down and then
// copying like normal.
void flip_grid(ca_grid** grid, size_t rows, size_t cols){
    CELL_TYPE* temp = malloc(cols * sizeof(int));
    for(size_t i = 0; i < rows/2; i++){
        for (size_t j = 0; j < cols; j++){
            temp[j] = IDX((*grid), rows-i-1, j);
            IDX((*grid), rows-i-1, j) = IDX((*grid), i, j);
        }
        for (size_t j = 0; j < cols; j++){
            IDX((*grid), i, j) = temp[j];
        }
    }
}

void resize_array(uint8_t** ary, size_t old, size_t new){
    size_t n = MIN(old, new);
    uint8_t* temp = calloc(new, sizeof(uint8_t));
    for (size_t i = 0; i < n; i++){
        temp[i] = (*ary)[i];
    }
    free(*ary);
    *ary = temp;
}


//-----------------------------------[Cellular Automata Helpers]------------------------------------

//As a cell cools it has a higher chance of cooling again on the next frame.
int cooldown(int heat) {
    if (heat == 0) return 0;
    int r = (rand() % (heat*heat*heat));
    if (r == 0) heat--;
    return heat;
}

void cleargrid(ca_grid* grid, int h)
{
    for (int i = h; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            IDX(grid, i, j) = 0;
        }
    }
}

void warm(uint8_t* heater, uint8_t* hotplate, int maxtemp)
{
    for (size_t i = 0; i < WIDTH; i++) {
        hotplate[i] /= 2;
    }
    for (size_t i = 0; i < WIDTH; i++) {
        hotplate[i] += heater[i] * maxtemp;
    }
}

//---------------------------------------[Cellular Automata]----------------------------------------

void nextframe(ca_grid* field, ca_grid* count, uint8_t* hotplate)
{
    cleargrid(count, heightrecord);
    int rowsum = 0;
    int h = heightrecord - 3;
    h = MAX(h, 1);  //we can ignore the vast majority of cold cells
                    //and skip down to the bottom of the window
    for (int i = h; i <= HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            float avg = 0;
            //int temp = rand() % maxtemp * 4;
            int counter = 0;

            //the search space around the current cell is as follows
            //    .......
            //    ...X...
            //    .......
            //    .......
            //    .......
            for (int xoff = -3; xoff <= 3; xoff++) {
                for (int yoff = -1; yoff <= 3; yoff++) {
                    int y = i + yoff;
                    y = MAX(y,0); //if y is less than zero, clamp it to zero.
                    int x = j + xoff;
                    //if the search has gone beyond the left or right, no heat is added
                    if (x < 0 || x >= WIDTH) avg += 0;
                    //if the search goes below the screen, add the hotplate value.
                    //the hotplate has infinite depth.
                    else if (y >= HEIGHT)  avg += hotplate[x];
                    else avg += IDX(field, y, x);
                    counter++;
                }
            }
            avg /= counter;
            //see if the cell cools or not
            //we add the value at (i-1) so that an upward motion will be created.
            IDX(count, i-1, j) = cooldown(avg);
            rowsum += IDX(count, i-1, j);
        }
        if (rowsum > 0 && i < heightrecord) heightrecord = i;
        rowsum = 0;
    }

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            IDX(field, i, j) = IDX(count, i, j);
        }
    }
}

//Wolfram's Elementary cellular atomaton
//https://en.wikipedia.org/wiki/Elementary_cellular_automaton
void wolfram(uint8_t* world, const uint8_t rule)
{
    uint8_t* next = malloc(WIDTH * sizeof(uint8_t));
    size_t l,c,r;
    size_t lidx, ridx;
    uint8_t current;
    for (int i = 0; i < WIDTH; i++) {
        lidx = i > 0 ? i - 1 : WIDTH - 1;
        ridx = (i + 1) % WIDTH;
        l = world[lidx];
        c = world[i];
        r = world[ridx];
        current = (l<<2) | (c<<1) | r;
        next[i] = (rule>>current) & 0b1;
    }

    for (int i = 0; i < WIDTH; i++) {
        world[i] = next[i];
    }
    free(next);
}

//----------------------------------------[Draw and Animate]----------------------------------------

void flames(char dispch, uint8_t wolfrule, int maxtemp, int frameperiod)
{
    ca_grid* field = new_grid(HEIGHT, WIDTH); //The cells that will be displayed
    ca_grid* count = new_grid(HEIGHT, WIDTH); //A grid of cells used to tally neighbors for CA purposes

    // these special cells provide "heat" at the bottom of the screen.
    // The heater heats the hotplate. The hotplate will cool without heat.
    uint8_t* heater = malloc(WIDTH * sizeof(uint8_t));
    uint8_t* hotplate = malloc(WIDTH * sizeof(uint8_t));

    for (size_t i = 0; i < WIDTH; i++) {
        heater[i] = rand() % 2;
        hotplate[i] = 0;
    }

    int c = 0;
draw:
    while (sig_caught == 0 && (c = getchar()) != 'q') {
        if(c == 'k') maxtemp++;
        if(c == 'j' && maxtemp > 1) maxtemp--;
        wolfram(heater, wolfrule);
        // In about 1 in 30 frames, flip a random cell in the heater from 0 to 1 and vice versa
        if (!(rand() % 30)) { heater[rand()%WIDTH] ^= 0x1; }
        warm(heater, hotplate, maxtemp);
        printframe(field, dispch, maxtemp, heightrecord);
        nextframe(field, count, hotplate);
        #ifdef _WIN32
            Sleep(frameperiod);
        #elif __linux__
            usleep(frameperiod);
        #endif
    }

    // Resizing logic
    if (sig_caught == SIGWINCH){
        heightrecord = 0;
        clearscreen();
        size_t old_h = HEIGHT; size_t old_w = WIDTH;
        get_screen_sz(&HEIGHT, &WIDTH);
        resize(HEIGHT, WIDTH);
        resize_array(&heater, old_w, WIDTH);
        resize_array(&hotplate, old_w, WIDTH);
        // We flip the screen upside-down so that the bottom (where the flames are) gets copied
        // first.
        flip_grid(&field, old_h, old_w); //flip_grid(&count, old_h, old_w);
        resize_grid(&field, old_h, old_w, HEIGHT, WIDTH);
        resize_grid(&count, old_h, old_w, HEIGHT, WIDTH);
        // Don't forget to flip things right-side up!
        flip_grid(&field, HEIGHT, WIDTH); flip_grid(&count, HEIGHT, WIDTH);
        sig_caught = 0;
        goto draw;
    }

    free(hotplate); free(heater);
    free_grid(field); free_grid(count);
}

//----------------------------------------------[Help]----------------------------------------------

void printhelp(const char progname[])
{
    const char fmtstr[] =
        "\nUsage: %s [options]\n"\
        "\t-c character\tAn ASCII character to draw the flames. Default is '@'.\n"\
        "\t-h\t\tPrint this message.\n"\
        "\t-f framerate\tSet the framerate in frames/sec. Default is 20.\n"\
        "\t\t\tA framerate of zero will make frames spit out as soon as they are ready.\n"\
        "\t-t temp\t\tSet the maximum temperature of the flames. Default is 10.\n"\
        "\t\t\tA higher temp means taller flames. Press the up/down arrows\n"\
        "\t\t\tto change the temperature at any time.\n\n"\
        "Press ^C or q at any time to douse the flames.\n\n";
    fprintf(stderr, fmtstr, progname);
}

//--------------------------------------------[Signals]---------------------------------------------

void sig_handler(int signum)
{
    sig_caught = signum;
}

//----------------------------------------------[Main]----------------------------------------------


int main(int argc, char** argv)
{
    signal(SIGINT, sig_handler);
    signal(SIGWINCH, sig_handler);

    srand(time(NULL));

    const int persecond = 1000000;
    int frameperiod = persecond / 20;
    int maxtemp = 10;
    int dispch = '@';
    //Use Rule 60 to make flames flicker nicely.
    int wolfrule = 60;

    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "c:hf:t:w:")) != -1) {
        switch (c) {
            case 'c':
                dispch = optarg[0];
                break;
            case 'h':
                printhelp(argv[0]);
                return 0;
            case 'f':
                if (atoi(optarg) < 1) frameperiod = 0;
                else frameperiod = persecond / atoi(optarg);
                break;
            case 't':
                maxtemp = atoi(optarg);
                break;
            case 'w':
                wolfrule = atoi(optarg);
                break;
            case '?':
                fprintf(stderr, "\nYou've really bunged this one up. Here, this may help:\n");
                printhelp(argv[0]);
                return 1;
            default:
                fprintf(stderr, "What the hell?!");
                return 2;
        }
    }
    begin_draw();
    flames(dispch, wolfrule, maxtemp, frameperiod);
    end_draw();
    return 0;
}
