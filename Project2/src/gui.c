#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"

Config config;
int menu_state = 0;         // 0: Main menu, 1: Custom input menu, 2: Display final configuration
char input_buffers[5][50] = {"", "", "", "", ""}; // Buffers for user inputs
int show_alert = 0;    // Alert flag for validation errors
int active_field = 0; // Tracks which input field is currently active for keyboard input


const char* prompts[] = {
        "Number of File Generators (1-100): ",
        "Number of File Calculators (1-100): ",
        "Number of Rows (1-100000): ",
        "Number of Columns (1-1000): ",
        "Miss Percentage (0-100): "
};

int* config_values[] = {
        &config.num_file_generators,
        &config.num_file_calculators,
        &config.default_rows,
        &config.default_columns,
        &config.miss_percentage
};

int min_values[] = {1, 1, 1, 1, 0};
int max_values[] = {100, 100, 100000, 1000, 100};



void init() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
}

void validateAndStoreInput() {
    show_alert = 0;

    for (int i = 0; i < 5; i++) {
        int value = atoi(input_buffers[i]);
        if (value >= min_values[i] && value <= max_values[i]) {
            *config_values[i] = value;
        } else {
            show_alert = 1; // Validation failed
            break;
        }
    }

    if (!show_alert) {
        menu_state = 2; // All inputs are valid; move to final configuration
    }

    glutPostRedisplay();
}

void processDefaultConfig() {
    if (load_config("config.txt", &config) != 0) {
        fprintf(stderr, "Failed to load configuration.\n");
        exit(1);
    }
    menu_state = 2; // Move to final configuration display
    glutPostRedisplay();
}



void mouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        // Convert mouse coordinates to OpenGL coordinates
        float mx = (float)x / 960.0f - 1.0f; // Scale for 1920px width
        float my = 1.0f - (float)y / 540.0f; // Scale for 1080px height

        if (menu_state == 0) { // Main Menu buttons
            if (mx >= -0.6f && mx <= 0.2f && my <= 0.6f && my >= 0.4f) {
                processDefaultConfig(); // Default Config
            } else if (mx >= -0.6f && mx <= 0.2f && my <= 0.3f && my >= 0.1f) {
                processDefaultConfig();
                menu_state = 1; // Custom Input Menu
                glutPostRedisplay();
            }
        }else if (menu_state == 1) {
            for (int i = 0; i < 5; i++) {
                float icon_x = -0.9f;
                float icon_y = 0.72f - i * 0.2f;
                float icon_size = 0.05f;

                // Check if mouse click is within the bounds of the icon
                if (mx >= icon_x && mx <= icon_x + icon_size && my <= icon_y && my >= icon_y - icon_size) {
                    active_field = i; // Set active field to the clicked icon
                    glutPostRedisplay();
                    return;
                }
            }
        }

    }
}


void drawText(float x, float y, const char* text) {
    glColor3f(1.0f, 1.0f, 1.0f); // Default color
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}


void drawButton(float x, float y, float width, float height, const char* label) {
    glColor3f(0.2f, 0.2f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y - height);
    glVertex2f(x, y - height);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(x + 0.02f, y - 0.07f, label);
}

void keyboard(unsigned char key, int x, int y) {
    if (menu_state == 1) { // Custom Input Menu
        if (key == '\r' || key == '\n') { // Enter key
            // Validate current input
            if (strlen(input_buffers[active_field]) > 0) { // Ensure the buffer is not empty
                int value = atoi(input_buffers[active_field]);
                if (value >= min_values[active_field] && value <= max_values[active_field]) {
                    *config_values[active_field] = value; // Store the validated value
                    active_field++; // Move to the next input field
                    show_alert = 0; // Reset alert
                    if (active_field == 5) { // All inputs collected
                        glutMouseFunc(NULL);
                        glutKeyboardFunc(NULL);
                        menu_state = 2; // Move to final configuration display
                    }
                } else {
                    show_alert = 1; // Invalid input; show alert
                }
            } else {
                show_alert = 1; // Buffer is empty; show alert
            }
        } else if (key == '\b') { // Backspace
            size_t len = strlen(input_buffers[active_field]);
            if (len > 0) input_buffers[active_field][len - 1] = '\0';
        } else if (key >= '0' && key <= '9') { // Numeric input
            size_t len = strlen(input_buffers[active_field]);
            if (len < sizeof(input_buffers[active_field]) - 1) {
                input_buffers[active_field][len] = key;
                input_buffers[active_field][len + 1] = '\0';
            }
        }
        glutPostRedisplay();
    }
}


void display_config() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (menu_state == 0) { // Main Menu
        drawText(-0.9f, 0.8f, "Choose an option:");
        drawButton(-0.6f, 0.6f, 0.8f, 0.2f, "Use default values from config.txt");
        drawButton(-0.6f, 0.3f, 0.8f, 0.2f, "Enter values manually");
    } else if (menu_state == 1) { // Custom Input Menu
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%s%s", prompts[active_field], input_buffers[active_field]);
        drawText(-0.3f, 0.2f, buffer);

        drawText(-0.9f, -0.8f, "Press Enter to confirm input.");

        if (show_alert) {
            char error_message[100];
            snprintf(error_message, sizeof(error_message), "Invalid input for %s. Range: %d-%d",
                     prompts[active_field], min_values[active_field], max_values[active_field]);
            drawText(-0.9f, -0.6f, error_message);
        }
    }
    glutSwapBuffers();
}