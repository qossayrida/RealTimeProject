#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "config.h"

#define NUM_INPUT_FIELDS 11

Config config;
int menu_state = 0;                                              // 0: Main menu, 1: Custom input menu, 2: Display final configuration
char input_buffers[NUM_INPUT_FIELDS][50] = {"", "", "", "", "", "", "", "", "", "", ""}; // Buffers for user inputs
int show_alert = 0;    // Alert flag for validation errors
int active_field = 0; // Tracks which input field is currently active for keyboard input
extern const char *directory_names[];
extern DirectoryData directory_data;


const char* prompts[] = {
        "Number of File Generators (1-100): ",
        "Number of File Calculators (1-100): ",
        "Number of File Movers (1-100): ",
        "Number of Type 1 Inspectors (1-100): ",
        "Number of Type 2 Inspectors (1-100): ",
        "Number of Type 3 Inspectors (1-100): ",
        "Number of Rows (upper limit) (5000-10000): ",
        "Number of Rows (lower limit) (1-5000): ",
        "Number of Columns (upper limit) (50-100): ",
        "Number of Columns (lower limit) (1-50): ",
        "Miss Percentage (0-100): "
};

int* config_values[] = {
        &config.num_file_generators,
        &config.num_file_calculators,
        &config.num_file_movers,
        &config.num_type1_inspectors,
        &config.num_type2_inspectors,
        &config.num_type3_inspectors,
        &config.default_upper_rows,
        &config.default_lower_rows,
        &config.default_upper_columns,
        &config.default_lower_columns,
        &config.miss_percentage
};

int min_values[] = {1,1,1,1 ,1, 1, 5000,1, 50, 1, 0};
int max_values[] = {100, 100, 100,100,100,100,10000,5000,100, 50, 100};

void init() {
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f); // R: 0.9, G: 0.9, B: 0.9, Alpha: 1.0
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
}

void validateAndStoreInput() {
    show_alert = 0;

    for (int i = 0; i < NUM_INPUT_FIELDS; i++) {
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
        float mx = (float) x / 960.0f - 1.0f; // Scale for 1920px width
        float my = 1.0f - (float) y / 540.0f; // Scale for 1080px height

        if (menu_state == 0) { // Main Menu buttons
            if (mx >= -0.6f && mx <= 0.2f && my <= 0.6f && my >= 0.4f) {
                processDefaultConfig(); // Default Config
            } else if (mx >= -0.6f && mx <= 0.2f && my <= 0.3f && my >= 0.1f) {
                processDefaultConfig();
                menu_state = 1; // Custom Input Menu
                glutPostRedisplay();
            }
        } else if (menu_state == 1) {
            for (int i = 0; i < NUM_INPUT_FIELDS; i++) {
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

void drawText(float x, float y, const char *text) {
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f); // R: 0.0, G: 0.0, B: 0.5, Alpha: 1.0
    glRasterPos2f(x, y);
    for (const char *c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

void drawButton(float x, float y, float width, float height, const char *label) {
    glColor4f(0.5f, 0.85f, 0.5f, 1.0f); // R: 0.5, G: 0.85, B: 0.5, Alpha: 1.0
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y - height);
    glVertex2f(x, y - height);
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // R: 1.0, G: 1.0, B: 1.0, Alpha: 1.0
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
                    active_field++;                       // Move to the next input field
                    show_alert = 0;                       // Reset alert
                    if (active_field == NUM_INPUT_FIELDS) { // All inputs collected
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
            if (len > 0)
                input_buffers[active_field][len - 1] = '\0';
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

void draw_chart() {
    float chart_width = 0.3f;    // Each chart width (30% of screen width)
    float chart_spacing = 0.05f; // Spacing between charts
    float max_height = 0.8f;     // Maximum chart height
    int max_files = 0;

    // Determine the maximum number of files across all directories
    for (int i = 0; i < 6; i++) {
        if (directory_data.file_counts[i] > max_files) {
            max_files = directory_data.file_counts[i];
        }
    }

    if (max_files == 0) {
        return; // No files to display, skip drawing
    }

    for (int i = 0; i < 6; i++) // Iterate through all directories
    {
        float x_start = -0.95f +
                        i * (chart_width + chart_spacing);                   // Horizontal start position for each chart
        float height = (float) directory_data.file_counts[i] / max_files * max_height; // Scale bar height

        // Draw the chart bar
        glColor3f(0.2f, 0.6f, 0.2f); // Bar color (green)
        glBegin(GL_QUADS);
        glVertex2f(x_start, -0.8f);
        glVertex2f(x_start + chart_width, -0.8f);
        glVertex2f(x_start + chart_width, -0.8f + height);
        glVertex2f(x_start, -0.8f + height);
        glEnd();

        // Draw the file count label on top of the bar
        glColor3f(0.0f, 0.0f, 0.0f); // White text
        char label[10];
        sprintf(label, "%d", directory_data.file_counts[i]);
        glRasterPos2f(x_start + chart_width / 2.0f - 0.02f, -0.8f + height + 0.05f);
        for (const char *c = label; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }

        // Alternate the y-coordinate for directory names
        float name_y_position = (i % 2 == 0) ? -0.85f : -0.88f; // Alternate between two vertical positions

        // Draw the directory name below the chart
        glColor3f(0.0f, 0.0f, 0.0f); // White text
        glRasterPos2f(x_start + chart_width / 2.0f - 0.17f, name_y_position);
        for (const char *c = directory_names[i]; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
        }
    }
}

void draw_table() {
    float col_width = 0.4f;    // Width of each column
    float row_height = 0.07f;  // Height for each row
    float square_size = 0.06f; // Size of file squares
    int files_per_column = 26; // Maximum files displayed in one column

    // Clear the background
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();

    // Draw directory headers (skip "Generated")
    for (int i = 1; i < 6; i++) // Start from index 1 to skip "Generated"
    {
        float x_start = -1.0f + (i - 1) * col_width; // Shift columns left by one

        // Draw header background
        glColor3f(0.2f, 0.6f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(x_start, 1.0f);
        glVertex2f(x_start + col_width, 1.0f);
        glVertex2f(x_start + col_width, 1.0f - row_height);
        glVertex2f(x_start, 1.0f - row_height);
        glEnd();

        // Draw header text
        glColor3f(0.0f, 0.0f, 0.0f);
        glRasterPos2f(x_start + 0.02f, 1.0f - 0.03f);
        for (const char *c = directory_names[i]; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    }

    // Draw files in each directory (skip "Generated")
    for (int dir = 1; dir < 6; dir++) // Start from index 1 to skip "Generated"
    {
        float x_start = -1.0f + (dir - 1) * col_width; // Shift columns left by one

        for (int file = 0; file < directory_data.file_counts[dir]; file++) {
            // Calculate position dynamically
            int column = file / files_per_column; // Determine which column within the directory
            int row = file % files_per_column;    // Determine row within the column

            float x_offset = column * (square_size + 0.02f); // Offset for multiple columns
            float y_start = 1.0f - row_height * (row + 2);   // Adjust y position for row

            int fileNumber = directory_data.number[dir][file];

            // Retrieve the stored color
            float r = directory_data.colors[fileNumber][0];
            float g = directory_data.colors[fileNumber][1];
            float b = directory_data.colors[fileNumber][2];

            // Skip invalid colors
            if (r == 0 && g == 0 && b == 0) {
                continue;
            }

            // Draw file square
            glColor3f(r, g, b); // Use the stored color
            glBegin(GL_QUADS);
            glVertex2f(x_start + x_offset + 0.02f, y_start - square_size);
            glVertex2f(x_start + x_offset + 0.02f + square_size, y_start - square_size);
            glVertex2f(x_start + x_offset + 0.02f + square_size, y_start);
            glVertex2f(x_start + x_offset + 0.02f, y_start);
            glEnd();

            // Draw file name (e.g., F1, F2)
            glColor3f(0.0f, 0.0f, 0.0f); // Black text for visibility
            char file_name[10];
            sprintf(file_name, "F%d", fileNumber);
            glRasterPos2f(x_start + x_offset + 0.03f, y_start - square_size / 2);
            for (const char *c = file_name; *c != '\0'; c++) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
            }
        }
    }
}

void get_color(int generator_id, float *r, float *g, float *b) {
    unsigned int hashed_id = (unsigned int) generator_id;
    hashed_id = ((hashed_id >> 16) ^ hashed_id) * 0x45d9f3b;
    hashed_id = ((hashed_id >> 16) ^ hashed_id) * 0x45d9f3b;
    hashed_id = (hashed_id >> 16) ^ hashed_id;

    // Map hashed_id to Hue, Saturation, and Value
    float hue = (hashed_id % 360) / 360.0f; // Hue: 0 to 360 degrees
    float saturation = 0.7f;                // High saturation for vivid colors
    float value = 0.9f;                     // Bright colors

    // Convert HSV to RGB
    float c = value * saturation; // Chroma
    float x = c * (1 - fabs(fmod(hue * 6, 2) - 1));
    float m = value - c;

    float r_temp, g_temp, b_temp;

    if (hue < 1.0 / 6.0) {
        r_temp = c;
        g_temp = x;
        b_temp = 0;
    } else if (hue < 2.0 / 6.0) {
        r_temp = x;
        g_temp = c;
        b_temp = 0;
    } else if (hue < 3.0 / 6.0) {
        r_temp = 0;
        g_temp = c;
        b_temp = x;
    } else if (hue < 4.0 / 6.0) {
        r_temp = 0;
        g_temp = x;
        b_temp = c;
    } else if (hue < 5.0 / 6.0) {
        r_temp = x;
        g_temp = 0;
        b_temp = c;
    } else {
        r_temp = c;
        g_temp = 0;
        b_temp = x;
    }

    *r = r_temp + m;
    *g = g_temp + m;
    *b = b_temp + m;
}