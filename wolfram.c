#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef char byte;

int width;
byte* next;
char* gen;

void wolfram(byte* world, const byte rule){
    int l,c,r;
    int current = 0;
    for(int i = 0; i < width; i++){
        l = world[(i-1) % width];
        c = world[i];
        r = world[(i+1) % width];
        current = (l<<2) | (c<<1) | r;
        next[i] = ((1<<current) & rule) > 0 ? 1 : 0;
    }

    for(int i = 0; i < width; i++){
        world[i] = next[i];
    }
}

void printit(byte* ary){
    for(int i = 0; i < width; i++){
        if(ary[i] == 0) gen[i] = ' ';
        else gen[i] = '#';
    }
    puts(gen);
}

int main(int argc, char** argv){
    srand(time(NULL));
    byte rule = atoi(argv[1]);
    width = 80;
    byte* world = malloc(width*sizeof(byte));
    next = malloc(width*sizeof(byte));
    gen = malloc((width+1) * sizeof(char));
    gen[width] = '\0';
    for(int i = 0; i < width; i++){
        world[i] = rand()%2;
    }
    for(int i = 0; i > -1;){
        printit(world);
        wolfram(world, rule);
    }
    free(gen);
    free(next);
    free(world);
}

