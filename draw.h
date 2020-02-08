#ifndef DRAW_H
#define DRAW_H

extern int WIDTH, HEIGHT;

void printframe(struct ca_grid* field, char dispch, int maxtemp, int heightrecord);
void clearscreen();
void begin_draw();
void end_draw();
void get_screen_sz(int* h, int* w);
void resize(int h, int w);
int getchar();
#endif //#ifndef DRAW_H
