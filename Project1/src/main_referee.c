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
#include "gui.h"


Team teams[3];
pid_t assistant_referees[3];
Config game_config;
int num_messages=0,is_game_over=0;
int fifo_fd,num_messages;
int jumping_player_is[3] = {-1, -1, -1};
Message message_buffer[10];


Player teamsInterface[3][3];    // 3 teams, each with 3 players
int currentTeam ;
int currentPlayer ;
const float teamStartPositions[3] = {-0.6, 0.0, 0.6};
const float gravity = -0.002f;
const float springFactor = 0.8f;  // Damping factor for bounce
const float pullUpSpeed = 0.005f; // Speed of pulling up the rope (smaller to fit scaling)
const float sideXOffset = 0.1f;   // Reduced distance for left and right players when they come to the middle

// Scaling factors


// Signal timers (in milliseconds)
int jumpSignalTimer[3];
int pullSignalTimer[3];
int stopPullSignalTimer[3];

void handle_alarm();
void end_game();
void init();
void display();
void initiateSwap(int teamNumber, int jumpingPlayerIndex);
void startJumping(int teamIndex, int playerIndex);
void stopJumping(int teamIndex, int playerIndex);
void startPulling(int teamIndex);
void stopPulling(int teamIndex);
void timer();

void drawBridge();
void drawScore();
void drawEnergy();

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
    glutMainLoop();


    return EXIT_SUCCESS;
}

void timer(){

    int bytes_read = read(fifo_fd, message_buffer, sizeof(message_buffer));

    if(bytes_read != -1){
        num_messages = bytes_read / sizeof(Message);
        printf("Referee: Received %d messages\n", num_messages);
    } else {
        num_messages = 0;
    }

    if (is_game_over) {
        end_game();
    }

    for (int i = 0; i<num_messages; i++){
        if (message_buffer[i].type == JUMP_START_ID) {
            startJumping(message_buffer[i].team_id, message_buffer[i].player_id);
            jumping_player_is[message_buffer[i].team_id] = message_buffer[i].player_id;
            printf("Team %d --> Player %d is starting to jump\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1);
        } else if (message_buffer[i].type == PULL_START_ID) {
            startPulling(message_buffer[i].team_id);
            printf("Team %d --> Player %d is starting to pull\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1);
        } else if (message_buffer[i].type == JUMP_END_ID) {
            stopJumping(message_buffer[i].team_id, message_buffer[i].player_id);
            printf("Team %d --> Player %d has finished jumping\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1);
        } else if (message_buffer[i].type == PULL_END_ID) {
            stopPulling(message_buffer[i].team_id);
            teams[message_buffer[i].team_id].score += (int) (0.5 * (game_config.max_score * (game_config.stabilization_time_min/1000.0) / atof(message_buffer[i].content)));
            printf("Team %d --> Player %d has finished pulling with time %s with score %d\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1,message_buffer[i].content,teams[message_buffer[i].team_id].score);
            if (teams[message_buffer[i].team_id].score >= game_config.max_score) {
                end_game();
            }
        } else if (message_buffer[i].type == ENERGY_ID) {
            teamsInterface[message_buffer[i].team_id][message_buffer[i].player_id].energy = atoi(message_buffer[i].content);
            printf("Team %d --> Player %d has %s energy\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1, message_buffer[i].content);
        } else if (message_buffer[i].type == TEAM_STOP) {
            printf("Team %d has stopped\n", message_buffer[i].team_id + 1);
        }
    }

    glutPostRedisplay();                                      // Trigger a display update
}

void handle_alarm() {
    printf("Referee: Maximum time reached. Ending game.\n");
    is_game_over=1;
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
            kill(teams[i].players[j], SIGKILL);
        }
        kill(assistant_referees[i], SIGKILL);
    }
    exit(EXIT_SUCCESS);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawBridge();
    drawReferees(-0.82f, baseBridgeHeight, 1.0f, 1.0f, 0.0f,0,"");
    drawReferees(-0.22f, baseBridgeHeight, 1.0f, 1.0f, 0.0f,0,"");
    drawReferees(0.38f, baseBridgeHeight, 1.0f, 1.0f, 0.0f,0,"");

    float waveAngle = 10 * sin(glutGet(GLUT_ELAPSED_TIME) / 100.0); // Waving flag angle
    drawRefereeWithFlag(-0.9f, 1.1f, waveAngle);

    drawScore();

    for (int t = 0; t < 3; t++) {
        float r = (t == 0) ? 1.0f : 0.0f;
        float g = (t == 1) ? 1.0f : 0.0f;
        float b = (t == 2) ? 1.0f : 0.0f;

        for (int p = 0; p < 3; p++) {
            Player* jumper = &teamsInterface[t][p];
            Player* left = &teamsInterface[t][(p + 1) % 3];
            Player* right = &teamsInterface[t][(p + 2) % 3];

            // Check if player is jumping and apply jump physics
            if (jumper->isJumping) {
                drawReferees(0.38f, baseBridgeHeight, 1.0f, 1.0f, 0.0f,1,"Jump time: 5s");
                jumper->velocity += gravity;
                jumper->jumpHeight += jumper->velocity;
                jumper->energy -= 0.001f;

                if (jumper->jumpHeight < -0.6f) {
                    jumper->velocity = -jumper->velocity * springFactor;
                    jumper->jumpHeight = -0.6f;
                }

                float playerX = jumper->x;
                float playerY = jumper->y + jumper->jumpHeight;
                drawHumanPlayer(playerX, playerY, r, g, b, p);
            }else if (jumper->isStuck && (left->isPulling || right->isPulling)) {
                jumper->x = teamStartPositions[t] + (p - 1) * sideXOffset;
                jumper->y = baseBridgeHeight;
                drawHumanPlayer(jumper->x, jumper->y, r, g, b, p);
            }else{
                jumper->x = teamStartPositions[t] + (p - 1) * sideXOffset;
                jumper->y = baseBridgeHeight;
                drawHumanPlayer(jumper->x, jumper->y, r, g, b, p);
            }

        }
    }

    // Draw ropes for jumping or pulling players
    for (int t = 0; t < 3; t++) {
        if (jumping_player_is[t] != -1) {
            Player *jumper = &teamsInterface[t][jumping_player_is[t]];
            Player *left = &teamsInterface[t][(jumping_player_is[t] + 1) % 3];
            Player *right = &teamsInterface[t][(jumping_player_is[t] + 2) % 3];

            if (jumper->isJumping || left->isPulling || right->isPulling) {
                drawRope(jumper, left, right);
            }
        }
    }

    // Draw trees in the background
    for (float x = -1.0f; x <= 1.0f; x++) {
        drawTree(x, -1.0f);
    }

    glutSwapBuffers();
}


void init() {
    glClearColor(0.5, 0.8, 1.0, 1.0);
    for (int t = 0; t < 3; t++) {
        for (int p = 0; p < 3; p++) {
            teamsInterface[t][p].x = teamStartPositions[t] + (p - 1) * sideXOffset;
            teamsInterface[t][p].y = baseBridgeHeight;
            teamsInterface[t][p].isJumping = 0;
            teamsInterface[t][p].jumpHeight = 0.0f;
            teamsInterface[t][p].velocity = 0.0f;
            teamsInterface[t][p].isPulling = 0;
            teamsInterface[t][p].energy = 1.0f;
        }
    }
}

//void drawEnergy(){
//    glColor3f(0, 0, 0);  // Black color for energy text
//    glRasterPos2f(x, y); // Position energy text
//    char energyStr[30];
//    sprintf(energyStr, "Player %d: %0.2f", playerNumber + 1, energy);
//    for (char *c = energyStr; *c != '\0'; c++){
//        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
//    }
//}

void drawScore(){
    for (int t = 0; t < 3; t++){
        glColor3f(0, 0, 0);
        glRasterPos2f(teamsInterface[t][1].x - 0.05f, 0.9f);
        char scoreStr[30];
        sprintf(scoreStr, "Team %d: %d", t + 1, teams[t].score);
        for (char *c = scoreStr; *c != '\0'; c++){
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    }
}



void startPulling(int teamIndex){
    int jumpingPlayerIndex = jumping_player_is[teamIndex];
    jumping_player_is[teamIndex] = -1;
    Player *left = &teamsInterface[teamIndex][(jumpingPlayerIndex + 1) % 3];
    Player *right = &teamsInterface[teamIndex][(jumpingPlayerIndex + 2) % 3];
    left->isPulling = 1;
    right->isPulling = 1;
}

void stopPulling(int teamIndex){
    int jumpingPlayerIndex = jumping_player_is[teamIndex];
    Player *jumper = &teamsInterface[teamIndex][jumpingPlayerIndex];
    Player *left = &teamsInterface[teamIndex][(jumpingPlayerIndex + 1) % 3];
    Player *right = &teamsInterface[teamIndex][(jumpingPlayerIndex + 2) % 3];
    left->isPulling = 0;
    right->isPulling = 0;
    jumper->isPulling = 0;
    left->jumpHeight = 0.0f;
    right->jumpHeight = 0.0f;
    jumper->jumpHeight = 0.0f;
    left->x = jumper->x + 0.15f;
    right->x = jumper->x - 0.15f;
}

void startJumping(int teamIndex, int playerIndex) {
    teamsInterface[teamIndex][playerIndex].isJumping = 1;
    teamsInterface[teamIndex][playerIndex].velocity = 0.02f;
    teamsInterface[teamIndex][playerIndex].jumpHeight = 0.0f;
}

void stopJumping(int teamIndex, int playerIndex){
    Player *jumper = &teamsInterface[teamIndex][jumping_player_is[teamIndex]];
    jumper->isJumping = 0;
    jumper->isStuck = 1;
    jumper->jumpHeight = -0.6f;
    jumper->velocity = 0.0f;
}