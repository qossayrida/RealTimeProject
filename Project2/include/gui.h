#ifndef GUI_H
#define GUI_H

#include "config.h"

extern Config config;
extern int menu_state;


#define NUM_DIRECTORIES 4
#define MAX_FILES 50

typedef struct {
    char files[NUM_DIRECTORIES][MAX_FILES][50]; // File names for each directory
    int file_counts[NUM_DIRECTORIES];           // Number of files in each directory
} DirectoryData;

#define NUM_DIRECTORIES 4
#define MAX_FILES 50

void draw_table();
void draw_chart();
void drawText(float x, float y, const char* text);
void drawButton(float x, float y, float width, float height, const char* label);
void display_config();
void processDefaultConfig();
void validateAndStoreInput();
void mouse(int button, int state, int x, int y);
void keyboard(unsigned char key, int x, int y);
void init();

#endif
