#ifndef GUI_H
#define GUI_H

#include "config.h"

extern Config config;
extern int menu_state;


void drawText(float x, float y, const char *text,int line);

void drawButton(float x, float y, float width, float height, const char *label);

void display_config();

void processDefaultConfig();

void validateAndStoreInput();

void mouse(int button, int state, int x, int y);

void keyboard(unsigned char key, int x, int y);

void init();

void draw_chart();

void draw_table();

void display();

void get_color(int generator_id, float *r, float *g, float *b);

#endif
