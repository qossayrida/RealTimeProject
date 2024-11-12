#ifndef GUI_H
#define GUI_H

extern float scaleFactor; // Scale down elements to increase spacing
extern float baseBridgeHeight;

typedef struct{
    float x, y;       // Position of the player
    int isJumping;    // Flag to check if player is jumping
    int isStuck;      // Flag to check if player is stuck
    float jumpHeight; // Current vertical offset (for bungee effect)
    float velocity;   // Velocity for bungee effect
    int isPulling;    // Flag for players who are pulling the rope
    int energy;     // Player's energy level
    int isMoving;     // Flag for moving to a target
    float targetX;    // Target x position for movement
} Player;


void drawRefereeWithFlag(float x, float y, float waveAngle);
void drawReferees(float x, float y, float r, float g, float b, int print, char message[]);
void drawHumanPlayer(float x, float y, float r, float g, float b, float alpha, int playerNumber);
void drawTree(float x, float y);
void drawRope(Player *jumper, Player *left, Player *right);
void drawBridge();
void drawGameResult(const char* resultMessage);


#endif // GUI_H
