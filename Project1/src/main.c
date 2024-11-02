#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include "config.h"
#include "referee.h"
#include "team.h"
#include "player.h"
#include <math.h>       
#include <unistd.h>     

// Game configuration
Config game_config;

// Team positions on the bridge
float team_positions[3] = { -0.5f, 0.0f, 0.5f };
int team_scores[3] = { 0, 0, 0 };

// Function to initialize OpenGL settings
void initOpenGL() {
    glClearColor(0.0f, 0.5f, 1.0f, 1.0f); // Set background color to light blue (sky color)
    glEnable(GL_DEPTH_TEST);              // Enable depth testing for 3D rendering
}

// Draw the bridge
void drawBridge() {
    glColor3f(0.5f, 0.3f, 0.2f); // Brown color for the bridge
    glBegin(GL_QUADS);
        glVertex3f(-0.8f, 0.0f, 0.0f);
        glVertex3f(0.8f, 0.0f, 0.0f);
        glVertex3f(0.8f, -0.05f, 0.0f);
        glVertex3f(-0.8f, -0.05f, 0.0f);
    glEnd();
}

// Draw a player as a small circle
void drawPlayer(float x, float y, float color[3]) {
    glColor3fv(color);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y); // Center of the circle
        for (int i = 0; i <= 20; ++i) {
            float angle = i * 2.0f * 3.14159f / 20; 
            glVertex2f(x + 0.03f * cos(angle), y + 0.03f * sin(angle));
        }
    glEnd();
}

// Display callback for rendering the scene
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawBridge();

    // Draw players on the bridge
    float player_color[3] = { 1.0f, 1.0f, 1.0f }; // White color for players

    for (int i = 0; i < 3; ++i) {
        drawPlayer(team_positions[i], 0.05f, player_color);
    }

    glutSwapBuffers();
}

// Update function for animations
void update(int value) {
    // Update team positions or player jumps here as needed
    // Placeholder for any animations or movements
    // For example, you could change `team_positions` here if simulating jumps

    glutPostRedisplay();               // Redraw the screen
    glutTimerFunc(100, update, 0);     // Call update every 100 ms
}

// Main function
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <config_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Load configuration
    if (load_config(argv[1], &game_config) != 0) {
        fprintf(stderr, "Failed to load config file.\n");
        return EXIT_FAILURE;
    }

    // Fork process to start referee and game logic
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: referee's game control
        referee_main(&game_config);
        exit(EXIT_SUCCESS);
    }

    // Parent process: OpenGL display and visualization
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Bungee Jumping Simulation");

    initOpenGL();
    glutDisplayFunc(display);
    glutTimerFunc(100, update, 0);

    // Start the OpenGL main loop
    glutMainLoop();

    return EXIT_SUCCESS;
}

