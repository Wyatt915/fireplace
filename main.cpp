#include <cmath>
#include <iostream>
#include <ncurses.h>
#include <stdlib.h>
#include <ctime>
#include <unistd.h>
#include <vector>
#include <string>

int SCW, SCH, maxtemp, gen;

int** init(int y, int x){
    int** out = new int*[y];
    for(int i = 0; i < y; i++){
        out[i] = new int[x];
    }

    for(int i = 0; i < y; i++){
        for(int j = 0; j < x; j++){
            out[i][j] = 0;
        }
    }
    return out;
}

void deallocate(int** in, int rows){
    for(int i = 0; i < rows; i++){
        delete[] in[i];
    }
    delete[] in;
}

int weightedprob(int max){
    int r = max - (rand() % 2);
    return r < 0 ? 0 : r;
}

float dist(float x, float y){
    float d = sqrt((x*x) + (y*y));
    return d  == 0 ? 0.5 : d;
}

float hotplate_temp_at(int* hotplate, int x){
    float total = 0;
    for(int i = -4; i <=4; i++){
        int j = x + i;
        if(j <0 || j >= SCW){
            total +=0;
        }
        else{
            total += hotplate[j];
        }
    }
    return (total * maxtemp) / 9.0;
}

void nextframe(int** field, int** count, int* hotplate){
    for(int i = 1; i <= SCH; i++){
        for(int j = 0; j < SCW; j++){
            float avg = 0;
            //int temp = rand() % maxtemp * 4;
            int counter = 0;
            for(int xoff = -7; xoff <= 7; xoff++){
                for(int yoff = 1; yoff < 7; yoff++){
                    int y = i + yoff;
                    y = y < 0 ? 0 : y;
                    int x = j + xoff;
                    if(x < 0)         avg += 0;
                    else if(x >= SCW) avg += 0;
                    else if(y >= SCH)  avg += hotplate_temp_at(hotplate,x);
                    else avg += field[y][x];
                    counter++;
                }
            }
            avg /= counter;
            count[i - 1][j] = weightedprob(avg);
        }
    }
    
    for(int i = 0; i < SCH; i++){
        for(int j = 0; j < SCW; j++){
            field[i][j] = count[i][j];
        }
    }
}

void wolfram(int* world, const int rule){
    int l,c,r;
    int* next = new int[SCW];
    int current;
    for(int i = 0; i < SCW; i++){
        l = i-1 < 0 ? 0 : world[i-1];
        c = world[i];
        r = i+1 >= SCW ? 0 : world[i+1];
        current = (l<<2) | (c<<1) | r;
        next[i] = ((1<<current) & rule) > 0 ? 1 : 0;
    }

    for(int i = 0; i < SCW; i++){
        world[i] = next[i];
    }
    delete[] next;
}

int time_in_ms(){
    return std::clock() / (double)(CLOCKS_PER_SEC / 1000);
}

//clamps the framerate, returns true only every n milliseconds
bool advance(int n){
#ifndef STEP
    return (time_in_ms() % n) <= 10;
#else
    return true;
#endif
}

void flames(){
    int** field = init(SCH, SCW);
    int** count = init(SCH, SCW);
    int* hotplate = new int[SCW];
    
    for(int i = 0; i < SCW; i++){
        hotplate[i] = rand() % 2;
    }
    
    char c, disp;
    while((c = getch()) != 'q'){
        if(advance(100)){
            gen++;
            for(int i = 0; i < SCH; i++){
                for(int j = 0; j < SCW; j++){
                    move(i,j);
                    disp = field[i][j] == 0 ? ' ' : '@';
                    int color = (7 * field[i][j] / maxtemp) + 1;
                    attron(COLOR_PAIR(color));
                    addch(disp);
                    attroff(COLOR_PAIR(color));
                }
            }
            nextframe(field, count, hotplate);
            wolfram(hotplate, 90);
            mvaddstr(0,0, std::to_string(gen).c_str());
            refresh();
        }
    }
    refresh();
    deallocate(field, SCH);
    deallocate(count, SCH);
}

void set_colors(){
    start_color();
    //init_color(COLOR_BLACK,    0,     0,     0);
    init_color(COLOR_RED,      100,   0,     0);
    init_color(COLOR_GREEN,    300,   0,     0);
    init_color(COLOR_BLUE,     500,   100,   0);
    init_color(COLOR_YELLOW,   700,   300,   0);
    init_color(COLOR_MAGENTA,  900,   500,   100);
    init_color(COLOR_CYAN,     1000,  800,   500);
    init_color(COLOR_WHITE,    1000,  1000,  1000);

    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);
}

int main(int argc, char** argv){
    gen = 0;
    maxtemp = 16;
    srand(time(NULL));
    initscr();
    curs_set(0);
    cbreak();
    #ifndef STEP
    timeout(0);
    #endif
    noecho();
    keypad(stdscr, TRUE); 
    getmaxyx(stdscr, SCH, SCW);
    set_colors();
    flames();
    endwin();
    return 0;
}
