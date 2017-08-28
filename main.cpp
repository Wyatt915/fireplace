#include <cmath>
#include <iostream>
#include <ncurses.h>
#include <stdlib.h>
#include <ctime>
#include <unistd.h>
#include <vector>

int SCW, SCH, maxtemp;

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
    int sum = (max * (max+1))/2; //Thanks, Gauss!
    int r = rand() % sum;
    for(int i = 1; i < maxtemp; i++){
        if(r < max) return i;
        r -= i;
    }
}

inline float dist(float x, float y){
    return (sqrt((x*x) + (y*y)) / 90) + 0.1;
}

void nextframe(int** field, int** count){
    for(int i = 1; i < SCH + 1; i++){
        for(int j = 0; j < SCW; j++){
            float avg = 0;
            //int temp = rand() % maxtemp * 4;
            int counter = 0;
            for(int xoff = -7; xoff <= 7; xoff++){
                for(int yoff = 1; yoff <= 7; yoff++){
                    int y = i + yoff;
                    int x = j + xoff;
                    if(x < 0)         avg += 0;
                    else if(x >= SCW) avg += 0;
                    else if(y >= SCH)  avg += field[SCH][x];
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

//sets the heating element at the bottom of the screen
void hotplate(int** field){
    int t;
    for(int i = 0; i < SCW; i++){
        t = maxtemp * (sin(float(i)/10.0) + 2) /3;
        field[SCH][i] = t > maxtemp ? maxtemp : t;
    }
}
int time_in_ms(){
    return std::clock() / (double)(CLOCKS_PER_SEC / 1000);
}

void flames(){
    int** field = init(SCH + 1, SCW);
    int** count = init(SCH, SCW);
    hotplate(field);
    char c, disp;
    while((c = getch()) != 'q'){
        if((time_in_ms() % 100 ) <= 10){
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
            nextframe(field, count);
            refresh();
            //sleep(1);
        }
    }
    refresh();
    deallocate(field, SCH + 1);
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
    maxtemp = 64;
    srand(time(NULL));
    initscr();
    cbreak();
    timeout(0);
    noecho();
    keypad(stdscr, TRUE); 
    getmaxyx(stdscr, SCH, SCW);
    set_colors();
    flames();
    endwin();
    return 0;
}
