#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#include "gui.h"


float scaleFactor= 0.6f; // Scale down elements to increase spacing
float baseBridgeHeight = -0.15f;

void drawHumanPlayer(float x, float y, float r, float g, float b, int playerNumber){

    glColor3f(r, g, b);
    glPushMatrix();
    glTranslatef(x, y * scaleFactor, 0.0f);
    glScalef(scaleFactor, scaleFactor, scaleFactor); // Scale down each player

    // Head
    glPushMatrix();
    glTranslatef(0.0f, 0.15f, 0.0f);
    glutSolidSphere(0.05, 20, 20);
    glPopMatrix();

    // Draw the player's number above their head
    glColor3f(0, 0, 0);           // Black color for the number
    glRasterPos2f(-0.02f, 0.22f); // Position text slightly above the head
    char numStr[12]; // Increased buffer size to handle larger numbers safely
    sprintf(numStr, "%d", playerNumber + 1);

    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, numStr[0]);

    // Body
    glBegin(GL_LINES);
    glVertex2f(0.0f, 0.1f);
    glVertex2f(0.0f, -0.05f);
    glEnd();

    // Arms
    glBegin(GL_LINES);
    glVertex2f(-0.05f, 0.05f);
    glVertex2f(0.05f, 0.05f);
    glEnd();

    // Legs
    glBegin(GL_LINES);
    glVertex2f(0.0f, -0.05f);
    glVertex2f(-0.05f, -0.15f);
    glVertex2f(0.0f, -0.05f);
    glVertex2f(0.05f, -0.15f);
    glEnd();

    glPopMatrix();
}

void drawRefereeWithFlag(float x, float y, float waveAngle){
    // Change scale factor to increase overall size
    float refereeScale = 1.3f;   // Increase the size
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow color for referee
    glPushMatrix();
    glTranslatef(x, y * scaleFactor, 0.0f);
    glScalef(scaleFactor * refereeScale, scaleFactor * refereeScale, scaleFactor); // Scale up the referee

    // Head
    glPushMatrix();
    glTranslatef(0.0f, 0.2f, 0.0f); // Move head up slightly
    glutSolidSphere(0.08, 20, 20);  // Increase head size
    glPopMatrix();

    // Body
    glBegin(GL_LINES);
    glVertex2f(0.0f, 0.12f);
    glVertex2f(0.0f, -0.08f); // Increase body length
    glEnd();

    // Arms
    glBegin(GL_LINES);
    glVertex2f(-0.08f, 0.05f); // Widen arm span
    glVertex2f(0.0f, 0.05f);
    glVertex2f(0.0f, 0.05f);
    glVertex2f(0.08f, 0.07f); // Widen arm span on right side
    glEnd();

    // Flag
    glPushMatrix();
    glTranslatef(0.1f, 0.07f, 0.0f);        // Position flag at the end of right arm
    glRotatef(waveAngle, 0.0f, 0.0f, 1.0f); // Rotate to create waving effect
    glColor3f(1.0f, 0.0f, 0.0f);            // Red color for the flag
    glBegin(GL_POLYGON);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(0.06f, 0.0f);  // Increase flag width
    glVertex2f(0.06f, 0.12f); // Increase flag height
    glVertex2f(0.0f, 0.12f);
    glEnd();
    glPopMatrix();

    // Legs
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex2f(0.0f, -0.08f);
    glVertex2f(-0.06f, -0.2f); // Increase leg length and width
    glVertex2f(0.0f, -0.08f);
    glVertex2f(0.06f, -0.2f); // Increase leg length and width
    glEnd();

    // Base under the referee
    glColor3f(0.3f, 0.3f, 0.3f); // Gray color for the base
    glBegin(GL_QUADS);
    glVertex2f(-0.1f, -0.22f); // Width of base
    glVertex2f(0.1f, -0.22f);
    glVertex2f(0.1f, -0.25f);  // Height of base
    glVertex2f(-0.1f, -0.25f);
    glEnd();

    // Draw the referee's title slightly above the head
    glColor3f(0, 0, 0);           // Black color for the text
    glRasterPos2f(-0.06f, 0.3f); // Position text above the head
    char numStr[20];
    sprintf(numStr, "Main Referee");
    for (char *c = numStr; *c != '\0'; c++){
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *c);
    }

    glPopMatrix();
}



void drawReferees(float x, float y, float r, float g, float b, int print, char message[]) {
    glColor3f(r, g, b); // Color for referee
    glPushMatrix();
    glTranslatef(x, y * scaleFactor, 0.0f);
    glScalef(scaleFactor, scaleFactor, scaleFactor); // Scale the referee

    // Head
    glPushMatrix();
    glTranslatef(0.0f, 0.15f, 0.0f);
    glutSolidSphere(0.05, 20, 20);
    glPopMatrix();

    glColor3f(0, 0, 0); // Black color for the body and limbs
    // Body
    glBegin(GL_LINES);
    glVertex2f(0.0f, 0.1f);
    glVertex2f(0.0f, -0.05f);
    glEnd();

    // Arms
    glBegin(GL_LINES);
    glVertex2f(-0.05f, 0.05f);
    glVertex2f(0.0f, 0.05f);
    glVertex2f(0.0f, 0.05f);
    glVertex2f(0.05f, 0.06f);
    glEnd();

    // Legs
    glBegin(GL_LINES);
    glVertex2f(0.0f, -0.05f);
    glVertex2f(-0.05f, -0.15f);
    glVertex2f(0.0f, -0.05f);
    glVertex2f(0.05f, -0.15f);
    glEnd();

    // Base under the referee
    glColor3f(0.3f, 0.3f, 0.3f); // Gray color for the base
    glBegin(GL_QUADS);
    glVertex2f(-0.08f, -0.17f); // Adjust base width
    glVertex2f(0.08f, -0.17f);
    glVertex2f(0.08f, -0.20f);  // Adjust base height
    glVertex2f(-0.08f, -0.20f);
    glEnd();

    // Draw the referee's title slightly above the head
    glColor3f(0, 0, 0);           // Black color for the text
    glRasterPos2f(-0.12f, 0.25f); // Position text slightly above the head
    char numStr[20];
    sprintf(numStr, "Assistant Referee");
    for (char *c = numStr; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }

    // If print is 1, display a message in a speech bubble
    if (print == 1) {
        // Speech bubble background
        glColor3f(1.0f, 1.0f, 1.0f); // White color for bubble background
        glPushMatrix();
        glTranslatef(-0.05f, 0.45f, 0.0f); // Position the bubble above and to the side of the referee
        glBegin(GL_POLYGON);
        glVertex2f(-0.1f, -0.05f);
        glVertex2f(0.15f, -0.05f);
        glVertex2f(0.15f, 0.1f);
        glVertex2f(-0.1f, 0.1f);
        glEnd();

        // Triangle tail of the speech bubble
        glBegin(GL_TRIANGLES);
        glVertex2f(-0.05f, -0.05f);
        glVertex2f(-0.02f, -0.12f);
        glVertex2f(0.0f, -0.05f);
        glEnd();

        // Display message inside the speech bubble
        glColor3f(0, 0, 0); // Black color for the message text
        glRasterPos2f(-0.09f, 0.0f); // Position text inside the bubble
        for (char *c = message; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
        }
        glPopMatrix();
    }

    glPopMatrix();
}


void drawRope(Player *jumper, Player *left, Player *right){
    glColor3f(0.6f, 0.4f, 0.2f); // Brown color for rope
    glBegin(GL_LINES);
    glVertex2f(left->x, left->y * scaleFactor + 0.15f);                          // Left player's hand position
    glVertex2f(jumper->x, baseBridgeHeight * scaleFactor + jumper->jumpHeight + 0.22f);
    glVertex2f(right->x, right->y * scaleFactor + 0.15f);                        // Right player's hand position
    glVertex2f(jumper->x, baseBridgeHeight * scaleFactor + jumper->jumpHeight + 0.22f);
    glEnd();
}

void drawTree(float x, float y){
    // Draw tree trunk
    glColor3f(0.5f, 0.3f, 0.0f); // Brown color for the trunk
    glBegin(GL_QUADS);
    glVertex2f(x - 0.05f, y);        // Left bottom
    glVertex2f(x + 0.05f, y);        // Right bottom
    glVertex2f(x + 0.05f, y + 0.2f); // Right top
    glVertex2f(x - 0.05f, y + 0.2f); // Left top
    glEnd();

    // Draw tree foliage
    glColor3f(0.0f, 0.7f, 0.0f); // Green color for the foliage
    glPushMatrix();
    glTranslatef(x, y + 0.3f, 0.0f); // Position foliage above the trunk
    glutSolidSphere(0.1, 20, 20);    // Foliage as a sphere
    glPopMatrix();
}

void drawBridge(){
    glColor3f(0.3, 0.2, 0.1); // Brown color for bridge
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, baseBridgeHeight * scaleFactor - 0.1f); // Bridge height adjusted for smaller scale
    glVertex2f(1.0f, baseBridgeHeight * scaleFactor - 0.1f);
    glVertex2f(1.0f, baseBridgeHeight * scaleFactor);
    glVertex2f(-1.0f, baseBridgeHeight * scaleFactor);
    glEnd();
}
