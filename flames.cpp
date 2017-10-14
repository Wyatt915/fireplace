#include <ctime>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

int SCW, SCH, maxtemp, gen, framerate;

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
    if(max == 0) return 0;
    int r = (rand() % max);
    if(r == 0) max--;
    return max;
}

float hotplate_temp_at(int* hotplate, int x){
    float total = 0;
    for(int i = -9; i <= 9; i++){
        int j = x + i;
        if(j <0 || j >= SCW){
            total +=0;
        }
        else{
            total += hotplate[j];
        }
    }
    total = (total * maxtemp) / 9.0;
    return total > maxtemp ? maxtemp : total;
}

void nextframe(int** field, int** count, int* hotplate){
    for(int i = 1; i <= SCH; i++){
        for(int j = 0; j < SCW; j++){
            float avg = 0;
            //int temp = rand() % maxtemp * 4;
            int counter = 0;
            for(int xoff = -3; xoff <= 3; xoff++){
                for(int yoff = -1; yoff <= 3; yoff++){
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
        l = world[(i-1)%SCW];
        c = world[i];
        r = world[(i+1)%SCW];
        current = (l<<2) | (c<<1) | r;
        next[i] = ((1<<current) & rule) > 0 ? 1 : 0;
    }

    for(int i = 0; i < SCW; i++){
        world[i] = next[i];
    }
    delete[] next;
}

void animate(int** field, int** count, int* hotplate){
    char disp;
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
    wolfram(hotplate, 60);
    refresh();
}

unsigned long int time_in_ms(){
    return std::clock() / (double)(CLOCKS_PER_SEC / 1000);
}

void flames(){
    int** field = init(SCH, SCW);
    int** count = init(SCH, SCW);
    int* hotplate = new int[SCW];
    
    for(int i = 0; i < SCW; i++){
        hotplate[i] = rand() % 2;
    }
    
    char c = 0;
    
    unsigned long int prevframe = time_in_ms();

    while((c = getch()) != 'q'){
        if(time_in_ms() >= prevframe + framerate){
            animate(field, count, hotplate);
            prevframe = time_in_ms();
        }
    }

    refresh();
    delete[] hotplate;
    deallocate(field, SCH);
    deallocate(count, SCH);
}

void set_colors(){
    start_color();
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

int main(int argc, char** argv){
    gen = 0;
    maxtemp = 10;
    srand(time(NULL));
    initscr();
    curs_set(0);
    timeout(0);
    cbreak();
    noecho();
    keypad(stdscr, TRUE); 
    getmaxyx(stdscr, SCH, SCW);
    set_colors();
    framerate = 1000 / 20;
    flames();
    endwin();
    return 0;
}
