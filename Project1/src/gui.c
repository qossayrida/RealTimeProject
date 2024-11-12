#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#include "gui.h"


float scaleFactor= 0.6f; // Scale down elements to increase spacing
float baseBridgeHeight = -0.15f;

void drawHumanPlayer(float x, float y, float r, float g, float b, float alpha, int playerNumber) {
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(r, g, b, alpha); // Set color with transparency
    glPushMatrix();
    glTranslatef(x, y * scaleFactor, 0.0f);
    glScalef(scaleFactor, scaleFactor, scaleFactor); // Scale down each player

    // Head
    glPushMatrix();
    glTranslatef(0.0f, 0.15f, 0.0f);
    glutSolidSphere(0.05, 20, 20); // Head sphere

    // Smiley face details
    glColor3f(0, 0, 0); // Black color for eyes and mouth

    // Left eye
    glPushMatrix();
    glTranslatef(-0.015f, 0.02f, 0.0f); // Position left eye on the head
    glutSolidSphere(0.005, 10, 10); // Small circle for the eye
    glPopMatrix();

    // Right eye
    glPushMatrix();
    glTranslatef(0.015f, 0.02f, 0.0f); // Position right eye on the head
    glutSolidSphere(0.005, 10, 10); // Small circle for the eye
    glPopMatrix();

    // Mouth (smile)
    glBegin(GL_LINE_STRIP);
    for (float angle = 0; angle <= 180; angle += 10) {
        float rad = -1 * angle * (3.14159f / 180.0f);
        float x = 0.015f * cos(rad);
        float y = -0.01f + 0.01f * sin(rad);
        glVertex2f(x, y);
    }
    glEnd();
    glPopMatrix();

    // Draw the player's number above their head
    glColor4f(0, 0, 0, 1.0f); // Black color for the number with no transparency
    glRasterPos2f(-0.02f, 0.22f); // Position text slightly above the head
    char numStr[12]; // Increased buffer size to handle larger numbers safely
    sprintf(numStr, "%d", playerNumber + 1);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, numStr[0]);

    // Body
    glColor4f(0, 0, 0, 1.0f); // Reset to the player color with transparency
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

    // Disable blending after drawing the player to avoid affecting other objects
    glDisable(GL_BLEND);
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
    glTranslatef(0.0f, 0.15f, 0.0f);
    glutSolidSphere(0.06, 20, 20); // Head sphere

    // Smiley face details
    glColor3f(0, 0, 0); // Black color for eyes and mouth

    // Left eye
    glPushMatrix();
    glTranslatef(-0.015f, 0.02f, 0.0f); // Position left eye on the head
    glutSolidSphere(0.005, 10, 10); // Small circle for the eye
    glPopMatrix();

    // Right eye
    glPushMatrix();
    glTranslatef(0.015f, 0.02f, 0.0f); // Position right eye on the head
    glutSolidSphere(0.005, 10, 10); // Small circle for the eye
    glPopMatrix();

    // Mouth (smile)
    glBegin(GL_LINE_STRIP);
    for (float angle = 0; angle <= 180; angle += 10) {
        float rad = -1 * angle * (3.14159f / 180.0f);
        float x = 0.015f * cos(rad);
        float y = -0.01f + 0.01f * sin(rad);
        glVertex2f(x, y);
    }
    glEnd();
    glPopMatrix();

    // Body
    glBegin(GL_LINES);
    glVertex2f(0.0f, 0.09f);
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
    glColor3f(0.0f, 0.0f, 0.0f);
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
    glutSolidSphere(0.05, 20, 20); // Head sphere

    // Smiley face details
    glColor3f(0, 0, 0); // Black color for eyes and mouth

    // Left eye
    glPushMatrix();
    glTranslatef(-0.015f, 0.02f, 0.0f); // Position left eye on the head
    glutSolidSphere(0.005, 10, 10); // Small circle for the eye
    glPopMatrix();

    // Right eye
    glPushMatrix();
    glTranslatef(0.015f, 0.02f, 0.0f); // Position right eye on the head
    glutSolidSphere(0.005, 10, 10); // Small circle for the eye
    glPopMatrix();

    // Mouth (smile)
    glBegin(GL_LINE_STRIP);
    for (float angle = 0; angle <= 180; angle += 10) {
        float rad = -1 * angle * (3.14159f / 180.0f);
        float x = 0.015f * cos(rad);
        float y = -0.01f + 0.01f * sin(rad);
        glVertex2f(x, y);
    }
    glEnd();
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

    // Draw a small timer in the right hand
    glColor3f(1.0f, 1.0f, 1.0f); // White for timer face
    glPushMatrix();
    glTranslatef(0.05f, 0.06f, 0.0f); // Position in right hand
    glutSolidSphere(0.02, 20, 20);    // Timer circle
    glColor3f(0.0f, 0.0f, 0.0f);      // Black for timer hand
    glBegin(GL_LINES);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(0.01f, 0.01f);         // Line for timer hand
    glEnd();
    glPopMatrix();

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

void drawTree(float x, float y) {
    float scale = 0.5f; // Scale down the tree size
    // Draw tapered tree trunk
    glColor3f(0.5f, 0.3f, 0.0f); // Brown color for the trunk
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f); // Scale the trunk
    glBegin(GL_QUADS);
    glVertex2f(-0.04f, 0.0f);       // Left bottom
    glVertex2f(0.04f, 0.0f);        // Right bottom
    glVertex2f(0.03f, 0.2f);        // Right top (tapered)
    glVertex2f(-0.03f, 0.2f);       // Left top (tapered)
    glEnd();
    glPopMatrix();

    // Draw layered, conical foliage
    glColor3f(0.0f, 0.6f, 0.1f); // Darker green for lower foliage
    glPushMatrix();
    glTranslatef(x, y + 0.25f * scale, 0.0f);
    glutSolidCone(0.12f * scale, 0.2f * scale, 20, 20); // Bottom foliage layer
    glPopMatrix();

    glColor3f(0.0f, 0.8f, 0.2f); // Lighter green for upper foliage
    glPushMatrix();
    glTranslatef(x, y + 0.4f * scale, 0.0f);
    glutSolidCone(0.09f * scale, 0.15f * scale, 20, 20); // Middle foliage layer
    glPopMatrix();

    glColor3f(0.0f, 0.9f, 0.3f); // Bright green for top foliage
    glPushMatrix();
    glTranslatef(x, y + 0.5f * scale, 0.0f);
    glutSolidCone(0.06f * scale, 0.1f * scale, 20, 20); // Top foliage layer
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

void drawRoundedRectangle(float x, float y, float width, float height, float radius) {
    // Draw a rounded rectangle by combining four corner arcs and four straight edges.
    glBegin(GL_POLYGON);
    for (int i = 0; i <= 360; i++) {
        float theta = (i * 3.14159f) / 180.0f;
        float dx = radius * cosf(theta);
        float dy = radius * sinf(theta);
        if (i < 90) {
            glVertex2f(x + width / 2 - radius + dx, y + height / 2 - radius + dy);
        } else if (i < 180) {
            glVertex2f(x - width / 2 + radius + dx, y + height / 2 - radius + dy);
        } else if (i < 270) {
            glVertex2f(x - width / 2 + radius + dx, y - height / 2 + radius + dy);
        } else {
            glVertex2f(x + width / 2 - radius + dx, y - height / 2 + radius + dy);
        }
    }
    glEnd();
}

void drawGameResult(const char* resultMessage) {
    float x = 0.0f;
    float y = 0.0f;

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw a semi-transparent rounded background box with soft blue color
    glColor4f(0.2f, 0.5f, 0.8f, 0.9f); // Slightly brighter and more transparent
    float boxWidth = 0.7f;
    float boxHeight = 0.25f;
    float cornerRadius = 0.05f;
    drawRoundedRectangle(x, y, boxWidth, boxHeight, cornerRadius);

    // Set color for the main text
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // White text
    glRasterPos2f(x - 0.2f, y); // Adjust position slightly to center text

    // Render each character of the result message
    for (const char* c = resultMessage; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    // Draw smaller, animated twinkling stars around the result message
    int numStars = 50; // Increase the number of stars
    float starRadius = 0.007f; // Smaller star size
    float maxStarDistance = 0.5f; // Max distance from the center for stars
    for (int i = 0; i < numStars; i++) {
        float angle = (float)i / numStars * 2.0f * 3.14159f;
        float distance = ((float)rand() / RAND_MAX) * maxStarDistance;
        float starX = x + cos(angle) * distance;
        float starY = y + sin(angle) * distance;
        float starAlpha = 0.3f + 0.2f * sin((float)glutGet(GLUT_ELAPSED_TIME) / 100.0f + i); // Pulsing twinkle effect

        glColor4f(1.0f, 1.0f, 1.0f, starAlpha); // Dimmer stars
        glPushMatrix();
        glTranslatef(starX, starY, 0.0f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0.0f, 0.0f); // Center of the star
        for (int j = 0; j <= 10; j++) {
            float starAngle = j * 2.0f * 3.14159f / 10;
            glVertex2f(cos(starAngle) * starRadius, sin(starAngle) * starRadius);
        }
        glEnd();
        glPopMatrix();
    }

    // Disable blending after drawing the result to avoid affecting other objects
    glDisable(GL_BLEND);
}
