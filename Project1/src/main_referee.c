#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <GL/glut.h>
#include <math.h>
#include "config.h"


typedef struct{
    float x, y;       // Position of the player
    int isJumping;    // Flag to check if player is jumping
    float jumpHeight; // Current vertical offset (for bungee effect)
    float velocity;   // Velocity for bungee effect
    int isPulling;    // Flag for players who are pulling the rope
    float energy;     // Player's energy level
    int score;        // Team score
    int isMoving;     // Flag for moving to a target
    float targetX;    // Target x position for movement
} Player;

Team teams[3];
pid_t assistant_referees[3];
Config game_config;
int num_messages=0;
int fifo_fd,num_messages;
Message message_buffer[10];

void handle_alarm(int signum);
void end_game();
void init();
void display();
void drawBridge();
void initiateSwap(int teamNumber, int jumpingPlayerIndex);
void startJumping(int teamNumber, int playerNumber);
void stopJumping(int teamNumber, int playerNumber);
void startPulling(int teamNumber, int playerNumber);
void stopPulling(int teamNumber, int playerNumber, const char *content, int score);
void drawTree(float x, float y);
void drawEnergyPlayer(float energy, float x, float y, int playerNumber);
void drawScore();
void drawHumanPlayer(float x, float y, float r, float g, float b, int playerNumber);
void drawReferees(float x, float y, float r, float g, float b);
void drawRope(Player *jumper, Player *left, Player *right);
void drawRefereeWithFlag(float x, float y, float waveAngle);
void timer(int value);
//void timer2(int value);



Player teamsInterface[3][3];    // 3 teams, each with 3 players
int currentTeam ;
int currentPlayer ;
const float gravity = -0.002f;
const float springFactor = 0.8f;  // Damping factor for bounce
const float pullUpSpeed = 0.005f; // Speed of pulling up the rope (smaller to fit scaling)
const float sideXOffset = 0.1f;   // Reduced distance for left and right players when they come to the middle

// Scaling factors
const float scaleFactor = 0.6f; // Scale down elements to increase spacing
const float baseBridgeHeight = -0.15f;

int main(int argc, char **argv) {

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1920, 1080);
    glutCreateWindow("Team Bungee Jump Game with Referee and Score");



    if (argc < 2) {
        fprintf(stderr, "Usage: %s <config_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Load configuration
    if (load_config(argv[1], &game_config) != 0) {
        fprintf(stderr, "Failed to load config file.\n");
        return EXIT_FAILURE;
    }

    // Set up alarm for maximum time
    signal(SIGALRM, handle_alarm);
    printf("Referee: Maximum time set to %d seconds\n", game_config.max_time);
    alarm(game_config.max_time);

    char fifo_name_str[50];
    sprintf(fifo_name_str, "/tmp/bungee_public");
    if ( mknod(fifo_name_str, S_IFIFO | 0666, 0) < 0 ) {
        perror(fifo_name_str);
        exit(1);
    }

    // Create assistant referee processes
    for (int i = 0; i < 3; i++) {
        if ((assistant_referees[i] = fork()) == 0) {
            char name [50];
            char id_str[10];
            snprintf(name, sizeof(name), "bungee_assistant_referee_%d", i+1);
            snprintf(id_str, sizeof(id_str), "%d", i);
            execlp("./bin/assistant_referee", name, id_str,fifo_name_str, argv[1], NULL);
            perror("execlp failed for assistant_referee");
            exit(EXIT_FAILURE);
        }
    }

    fifo_fd = open(fifo_name_str, O_RDONLY);

    // Receive team data from each assistant referee
    for (int i = 0; i < 3; i++){
        Team team_data;
        if (read(fifo_fd, &team_data, sizeof(Team)) == -1) {
            perror("Failed to read from assistant referee");
            return EXIT_FAILURE;
        }
        teams[team_data.team_id] = team_data;
    }

    close(fifo_fd);
    fifo_fd = open(fifo_name_str, O_RDONLY|O_NONBLOCK);


    // Send signal to assistant referees to start the game
    for (int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            printf("Team %d --> Player %d has initial energy %d\n", i + 1, j + 1 , teams[i].initial_energy[j]);
        }
        kill(assistant_referees[i], SIGUSR1);
        printf("Referee: Sent start signal to Team %d\n", i + 1);
    }

    init();
    glutDisplayFunc(display);
    glutIdleFunc(timer);
//    glutTimerFunc(1, timer2, 0);            // Repeat every 16ms (~60 FPS)
    glutMainLoop();


    return EXIT_SUCCESS;
}

void timer(int value){

    int bytes_read = read(fifo_fd, message_buffer, sizeof(message_buffer));

    if(bytes_read != -1){
        num_messages = bytes_read / sizeof(Message);
        printf("Referee: Received %d messages\n", num_messages);
    } else {
        num_messages = 0;
    }
    
    for (int i = 0; i<num_messages; i++){
        if (message_buffer[i].type == JUMP_START_ID) {
            startJumping(message_buffer[i].team_id, message_buffer[i].player_id);
            printf("Team %d --> Player %d is starting to jump\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1);
        } else if (message_buffer[i].type == PULL_START_ID) {
            startPulling(message_buffer[i].team_id, message_buffer[i].player_id);
            printf("Team %d --> Player %d is starting to pull\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1);
        } else if (message_buffer[i].type == JUMP_END_ID) {
            stopJumping(message_buffer[i].team_id, message_buffer[i].player_id);
            printf("Team %d --> Player %d has finished jumping\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1);
        } else if (message_buffer[i].type == PULL_END_ID) {
            stopPulling(message_buffer[i].team_id, message_buffer[i].player_id, message_buffer[i].content, teams[message_buffer[i].team_id].score);
            teams[message_buffer[i].team_id].score += (int) (0.7 * (game_config.max_score * (game_config.stabilization_time_min/1000.0) / atof(message_buffer[i].content)));
            printf("Team %d --> Player %d has finished pulling with time %s with score %d\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1,message_buffer[i].content,teams[message_buffer[i].team_id].score);
            if (teams[message_buffer[i].team_id].score >= game_config.max_score) {
                end_game();
            }
        } else if (message_buffer[i].type == ENERGY_ID) {
            printf("Team %d --> Player %d has %s energy\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1, message_buffer[i].content);
        } else if (message_buffer[i].type == TEAM_STOP) {
            printf("Team %d has stopped\n", message_buffer[i].team_id + 1);
        }
    }

    glutPostRedisplay();                                      // Trigger a display update
}

void handle_alarm(int signum) {
    printf("Referee: Maximum time reached. Ending game.\n");
    end_game();
}

void end_game(){

    int max_score = teams[0].score;
    int winning_team_id = 0;
    int is_draw = 1;

    for (int j = 1; j < 3; j++) {
        if (teams[j].score > max_score) {
            max_score = teams[j].score;
            winning_team_id = j;
            is_draw = 0;  // There’s a unique highest score, so it’s not a draw
        } else if (teams[j].score < max_score) {
            is_draw = 0;  // Scores are not all equal
        }
    }

    if (is_draw) {
        printf("Referee: The game is a draw\n");
    } else {
        printf("Referee: Team %d has won the game with a score of %d\n", winning_team_id + 1, max_score);
    }

    system("rm /tmp/bungee_*");
    printf("Referee: Removed all FIFOs\n");

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            kill(teams[i].players[j], SIGTERM);
        }
        kill(assistant_referees[i], SIGTERM);
    }

    exit(EXIT_SUCCESS);
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT);
    drawBridge();

    float waveAngle = 10 * sin(glutGet(GLUT_ELAPSED_TIME) / 100.0); // Waving flag angle
    drawRefereeWithFlag(0.9f, 0.9f, waveAngle);

    drawScore();

    // Draw players and energy bars
    for (int t = 0; t < 3; t++){

        float r = (t == 0) ? 1.0f : 0.0f;
        float g = (t == 1) ? 1.0f : 0.0f;
        float b = (t == 2) ? 1.0f : 0.0f;

        drawReferees(teamsInterface[t][1].x - 0.25, teamsInterface[t][1].y, r, g, b);

        for (int p = 0; p < 3; p++){

            float playerX = teamsInterface[t][p].x;
            float playerY = teamsInterface[t][p].y + teamsInterface[t][p].jumpHeight;
            drawHumanPlayer(playerX, playerY, r, g, b, p);
            playerX += 0.5f;
            playerY += 0.5f;
            drawEnergyPlayer(teamsInterface[t][p].energy, teamsInterface[t][1].x - 0.02f, 1.0f - (0.2f + p * 0.06f), p);
        }
    }


    // Draw trees at the bottom of the scene
    for (float x = -1.0f; x <= 1.0f; x += 1)
    {
        drawTree(x, -1.0f); // Position trees below the bridge
    }

    glutSwapBuffers();
}

void init(){
    glClearColor(0.5, 0.8, 1.0, 1.0);                        // Sky blue background
    const float teamStartPositions[3] = {-0.6f, 0.0f, 0.6f}; // Adjust these for even spacing
    const float playerSpacing = 0.1f;                        // Spacing between players within a team

    for (int t = 0; t < 3; t++)
    {
        for (int p = 0; p < 3; p++)
        {
            // Set each team's starting position, offset players within the team by `playerSpacing`
            teamsInterface[t][p].x = teamStartPositions[t] + (p * playerSpacing) - (playerSpacing);
            teamsInterface[t][p].y = 0.0f;
            teamsInterface[t][p].isJumping = 0;
            teamsInterface[t][p].isPulling = 0;
            teamsInterface[t][p].isMoving = 0;
            teamsInterface[t][p].jumpHeight = 0.0f;
            teamsInterface[t][p].velocity = 0.02f; // Adjusted velocity for smaller scale
            teamsInterface[t][p].energy = 1.0f;    // Full energy initially
            teamsInterface[t][p].score = 0;        // Start with zero score
        }
    }
}



void drawBridge()
{
    glColor3f(0.3, 0.2, 0.1); // Brown color for bridge
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, baseBridgeHeight * scaleFactor - 0.1f); // Bridge height adjusted for smaller scale
    glVertex2f(1.0f, baseBridgeHeight * scaleFactor - 0.1f);
    glVertex2f(1.0f, baseBridgeHeight * scaleFactor);
    glVertex2f(-1.0f, baseBridgeHeight * scaleFactor);
    glEnd();
}

void drawHumanPlayer(float x, float y, float r, float g, float b, int playerNumber)
{
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
    char numStr[2];
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

void drawRope(Player *jumper, Player *left, Player *right)
{
    glColor3f(0.6f, 0.4f, 0.2f); // Brown color for rope
    glBegin(GL_LINES);
    glVertex2f(left->x, left->y * scaleFactor + 0.15f);                          // Left player's hand position
    glVertex2f(jumper->x, jumper->y * scaleFactor + jumper->jumpHeight + 0.15f); // Jumper’s head position
    glVertex2f(right->x, right->y * scaleFactor + 0.15f);                        // Right player's hand position
    glVertex2f(jumper->x, jumper->y * scaleFactor + jumper->jumpHeight + 0.15f);
    glEnd();
}

void drawReferees(float x, float y, float r, float g, float b)
{
    glColor3f(r, g, b); // Yellow color for referee
    glPushMatrix();
    glTranslatef(x, y * scaleFactor, 0.0f);
    glScalef(scaleFactor, scaleFactor, scaleFactor); // Scale down the referee

    // Head
    glPushMatrix();
    glTranslatef(0.0f, 0.15f, 0.0f);
    glutSolidSphere(0.05, 20, 20);
    glPopMatrix();

    glColor3f(0, 0, 0); // Black color for the number
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

    // Draw the player's number above their head
    glColor3f(0, 0, 0);           // Black color for the number
    glRasterPos2f(-0.02f, 0.22f); // Position text slightly above the head
    char numStr[8];
    sprintf(numStr, "Referee");
    for (char *c = numStr; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    glPopMatrix();
}

void drawRefereeWithFlag(float x, float y, float waveAngle)
{
    // Change scale factor to increase overall size
    float refereeScale = 1.5f;   // Increase the size
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow color for referee
    glPushMatrix();
    glTranslatef(x - 0.05, 0.09, 0.0f);
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

    glPopMatrix();
}

void drawTree(float x, float y)
{
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

void drawEnergyPlayer(float energy, float x, float y, int playerNumber)
{
    glColor3f(0, 0, 0);        // Black color for energy text
    glRasterPos2f(x - 0.1, y); // Position energy text
    char energyStr[20];
    sprintf(energyStr, "Player %d: %0.2f", playerNumber + 1, energy);
    for (char *c = energyStr; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

void drawScore()
{
    for (int t = 0; t < 3; t++)
    {
        glColor3f(0, 0, 0);                       // Black color for score text
        glRasterPos2f(teamsInterface[t][1].x - 0.1, 0.9f); // Position scores
        char scoreStr[30];
        sprintf(scoreStr, "Team %d: %d", t + 1, teamsInterface[t][0].score);
        for (char *c = scoreStr; *c != '\0'; c++)
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    }
}



void initiateSwap(int teamNumber, int jumpingPlayerIndex)
{
    Player *middlePlayer = &teamsInterface[teamNumber][1]; // Middle player at index 1
    Player *jumpingPlayer = &teamsInterface[teamNumber][jumpingPlayerIndex];

    middlePlayer->isMoving = 1;
    middlePlayer->targetX = jumpingPlayer->x;

    jumpingPlayer->isMoving = 1;
    jumpingPlayer->targetX = middlePlayer->x; // Middle position

    jumpingPlayer->isJumping = 0;
}

void startJumping(int teamNumber, int playerNumber)
{
    if (playerNumber != 1)
    {
        initiateSwap(teamNumber, playerNumber);
    }
    else
    {
        Player *current = &teamsInterface[teamNumber][playerNumber];
        current->isJumping = 1;
        current->velocity = 0.03f;
    }
}

void stopJumping(int teamNumber, int playerNumber)
{
    Player *jumper = &teamsInterface[teamNumber][playerNumber];

    jumper->isJumping = 0;
    jumper->jumpHeight = -0.6f;
    jumper->velocity = 0.0f;
}

void startPulling(int teamNumber, int playerNumber)
{
    Player *left = &teamsInterface[teamNumber][(playerNumber + 1) % 3];
    Player *right = &teamsInterface[teamNumber][(playerNumber + 2) % 3];

    left->isPulling = 1;
    right->isPulling = 1;
}

void stopPulling(int teamNumber, int playerNumber, const char *content, int score)
{
    Player *jumper = &teamsInterface[teamNumber][playerNumber];
    Player *left = &teamsInterface[teamNumber][(playerNumber + 1) % 3];
    Player *right = &teamsInterface[teamNumber][(playerNumber + 2) % 3];

    left->isPulling = 0;
    right->isPulling = 0;
    jumper->isPulling = 0;

    left->jumpHeight = 0.0f;
    right->jumpHeight = 0.0f;
    jumper->jumpHeight = 0.0f;

    left->energy = fmin(1.0f, left->energy + 0.002f);
    right->energy = fmin(1.0f, right->energy + 0.002f);

    left->x = jumper->x + 0.15f;
    right->x = jumper->x - 0.15f;
}

void updateJump(int teamNumber, int playerNumber)
{
    Player *jumper = &teamsInterface[teamNumber][playerNumber];
    Player *left = &teamsInterface[teamNumber][(playerNumber + 1) % 3];
    Player *right = &teamsInterface[teamNumber][(playerNumber + 2) % 3];

    if (jumper->isJumping)
    {
        jumper->velocity += gravity;
        jumper->jumpHeight += jumper->velocity;
        jumper->energy -= 0.001f; // Slight decrease in energy while jumping

        if (jumper->jumpHeight < -0.6f)
        {
            jumper->velocity = -jumper->velocity * springFactor;
            jumper->jumpHeight = -0.6f;

            if (fabs(jumper->velocity) < 0.005f)
            {
                stopJumping(teamNumber, playerNumber);
                startPulling(teamNumber, playerNumber);
                teamsInterface[teamNumber][0].score++;
            }
        }

        left->x = jumper->x + sideXOffset;
        right->x = jumper->x - sideXOffset;
    }

    if (!jumper->isJumping && (left->isPulling || right->isPulling))
    {
        if (jumper->jumpHeight < 0.0f)
        {
            jumper->jumpHeight = fmin(0.0f, jumper->jumpHeight + pullUpSpeed);
        }
        else
        {
            stopPulling(teamNumber, playerNumber, "0", 0);
        }
    }
}
