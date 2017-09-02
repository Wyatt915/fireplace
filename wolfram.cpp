#include <iostream>
#include <stdlib.h>
#include <ctime>

int width;

void wolfram(int* world, const int rule){
    int l,c,r;
    int* next = new int[width];
    int current;
    for(int i = 0; i < width; i++){
        l = i-1 < 0 ? 0 : world[i-1];
        c = world[i];
        r = i+1 >= width ? 0 : world[i+1];
        current = (l<<2) | (c<<1) | r;
        next[i] = ((1<<current) & rule) > 0 ? 1 : 0;
    }

    for(int i = 0; i < width; i++){
        world[i] = next[i];
    }
    delete[] next;
}

void printit(int* ary){
    for(int i = 0; i < width; i++){
        if(ary[i] == 0) std::cout << ' ';
        else std::cout << '#';
    }
    std::cout << std::endl;
}

int main(int argc, char** argv){
    srand(time(NULL));
    int rule = std::stoi(argv[1]);
    width = 80;
    int* world = new int[width] ();
    for(int i = 0; i < width; i++){
        world[i] = rand()%2;
    }
    for(int i = 0; i > -1; i++){
        printit(world);
        wolfram(world, rule);
    }
}

