#include <ctime>
#include <ncurses.h>
#include <stdlib.h> //random
#include <unistd.h> //getopt
#include <iostream>

//----------------------------------------[Global variables]----------------------------------------

int WIDTH, HEIGHT;  //Set by ncurses
int maxtemp;        //maximum flame temperature
int framerate;      //framerate
char dispch;        //the character used to draw the flames
int heightrecord;   //max height reached by the flames

//------------------------------[Memory Management and Initialization]------------------------------

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

void start_ncurses(){
    initscr();
    start_color();
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
    curs_set(0);
    timeout(0);
    cbreak();
    noecho();
    keypad(stdscr, TRUE); 
    getmaxyx(stdscr, HEIGHT, WIDTH);
    heightrecord = HEIGHT;
}

//---------------------------------------[Cellular Automata]---------------------------------------

//As a cell cools it has a higher chance of cooling again on the next frame.
int cooldown(int max){
    if(max == 0) return 0;
    int r = (rand() % max);
    if(r == 0) max--;
    return max;
}

float hotplate_temp_at(int* hotplate, int x){
    float total = 0;
    for(int i = -9; i <= 9; i++){
        int j = x + i;
        if(j < 0 || j >= WIDTH){
            total +=0;
        }
        else{
            total += hotplate[j];
        }
    }
    total = (total * maxtemp) / 9.0;
    return total > maxtemp ? maxtemp : total;
}

void cleargrid(int** grid){
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            grid[i][j] = 0;
        }
    }
}

void nextframe(int** field, int** count, int* hotplate){
    cleargrid(count);
    int rowsum = 0;
    int h = heightrecord - 3;
    h = h < 1 ? 1 : h;  //we can ignore the vast majority of cold cells
                        //and skip down to the bottom of the window
    for(int i = h; i <= HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            float avg = 0;
            //int temp = rand() % maxtemp * 4;
            int counter = 0;

            //the search space around the current cell is as follows
            //    .......
            //    ...X...
            //    .......
            //    .......
            //    .......
            for(int xoff = -3; xoff <= 3; xoff++){
                for(int yoff = -1; yoff <= 3; yoff++){
                    int y = i + yoff;
                    y = y < 0 ? 0 : y; //if y is less than zero, clamp it to zero.
                    int x = j + xoff;
                    //if the search has gon beyond the left or right, no heat is added
                    if(x < 0 || x >= WIDTH) avg += 0;
                    //if the search goes below the screen, add the hotplate value.
                    //the hotplate has infinite depth.
                    else if(y >= HEIGHT)  avg += hotplate_temp_at(hotplate,x);
                    else avg += field[y][x];
                    counter++;
                }
            }
            avg /= counter;
            //see if the cell cools or not
            //we add the value at (i-1) so that an upward motion will be created.
            count[i - 1][j] = cooldown(avg);
            rowsum += count[i-1][j];
        }
        if(rowsum > 0 && i < heightrecord) heightrecord = i;
        rowsum = 0;
    }
    
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            field[i][j] = count[i][j];
        }
    }
}

//Wolfram's Elementary cellular atomaton
void wolfram(int* world, const int rule){
    int l,c,r;
    int lidx, ridx;
    int* next = new int[WIDTH];
    int current;
    for(int i = 0; i < WIDTH; i++){
        lidx = i > 0 ? i - 1 : WIDTH - 1;
        ridx = (i + 1) % WIDTH;
        l = world[lidx];
        c = world[i];
        r = world[ridx];
        current = (l<<2) | (c<<1) | r;
        next[i] = ((1<<current) & rule) > 0 ? 1 : 0;
    }

    for(int i = 0; i < WIDTH; i++){
        world[i] = next[i];
    }
    delete[] next;
}

void animate(int** field, int** count, int* hotplate){
    char disp;
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            move(i,j);
            //if the cell is cold, print a space, otherwise print [dispch]
            disp = field[i][j] == 0 ? ' ' : dispch;
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

//-------------------------------------------[Main Loop]-------------------------------------------

unsigned long int time_in_ms(){
    return std::clock() / (double)(CLOCKS_PER_SEC / 1000);
}

void flames(){
    int** field = init(HEIGHT, WIDTH); //The cells that will be displayed
    int** count = init(HEIGHT, WIDTH); //A grid of cells used to tally neighbors for CA purposes
    int* hotplate = new int[WIDTH]; //these special cells provide "heat" at the bottom of the screen.
    
    for(int i = 0; i < WIDTH; i++){
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
    deallocate(field, HEIGHT);
    deallocate(count, HEIGHT);
}

void printhelp(char progname[]){
    std::cout << "\nUsage: " << progname << " [options]\n"
        << "\t-c character\tAn ASCII character to draw the flames. Default is '@'.\n"
        << "\t-h\t\tPrint this message.\n"
        << "\t-f framerate\tSet the framerate in frames/sec. Default is 20.\n"
        << "\t\t\tA framerate of zero will make frames spit out as soon as they are ready.\n"
        << "\t-t temp\t\tSet the maximum temperature of the flames. Default is 10.\n"
        << "\t\t\tA higher temp means taller flames.\n"
        << "\n"
        << "Press q at any time to douse the flames.\n\n";
}

int main(int argc, char** argv){
    srand(time(NULL));
    framerate = 1000 / 20;
    maxtemp = 10;
    dispch = '@';
    
    int c;
    opterr = 0;
    while((c = getopt(argc, argv, "c:hf:t:")) != -1){
        switch (c){
            case 'c':
                dispch = optarg[0];
                break;
            case 'h':
                printhelp(argv[0]);
                return 1;
            case 'f':
                if(atoi(optarg) < 1) framerate = 0;
                else framerate = 1000 / atoi(optarg);
                break;
            case 't':
                maxtemp = atoi(optarg);
                break;
            case '?':
                std::cout << "\nYou've really bunged this one up. Here, this may help:\n";
                printhelp(argv[0]);
                return 1;
            default:
                std::cerr << "What the hell?!";
                return 2;
        }
    }
    
    start_ncurses();
    flames();
    endwin();
    return 0;
}
