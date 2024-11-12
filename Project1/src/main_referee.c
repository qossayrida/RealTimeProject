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
double jump_interval[3] = {0, 0, 0};
Message message_buffer[50];
char result_game_buffer[100];


Player teamsInterface[3][3];    // 3 teams, each with 3 players
int currentTeam ;
int currentPlayer ;
const float teamStartPositions[3] = {-0.6, 0.0, 0.6};
const float assistantRefereePositions[3] = {-0.82f, -0.22f, 0.38f};
const float gravity = -0.002f;
const float springFactor = 0.8f;  // Damping factor for bounce
const float pullUpSpeed = 0.005f; // Speed of pulling up the rope (smaller to fit scaling)
const float sideXOffset = 0.1f;   // Reduced distance for left and right players when they come to the middle
float player_color[3][3] = {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}};
int jumpSignalTimer[3];
int pullSignalTimer[3];
int stopPullSignalTimer[3];

void handle_alarm();
void handle_alarm_to_end();
void end_game(int winner);
void init();
void display();
void display_result();
void startJumping(int teamIndex, int playerIndex);
void stopJumping(int teamIndex, int playerIndex);
void startPulling(int teamIndex);
void stopPulling(int teamIndex);
void timer();
void drawBridge();
void drawScore();
void drawEnergy();
void drawSetting();

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
            teamsInterface[i][j].energy = teams[i].initial_energy[j];
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

    if (is_game_over==3) {
        exit(EXIT_SUCCESS);
    } else if (is_game_over==2){
        glutPostRedisplay();
        return;
    } else if (is_game_over==1){
        end_game(-1);
        is_game_over=2;
        glutPostRedisplay();
        return;
    }

    int bytes_read = read(fifo_fd, message_buffer, sizeof(message_buffer));

    if(bytes_read != -1){
        num_messages = bytes_read / sizeof(Message);
        printf("Referee: Received %d messages\n", num_messages);
    } else {
        num_messages = 0;
    }

    for (int i = 0; i<num_messages ; i++){
        if (message_buffer[i].type == JUMP_START_ID) {
            startJumping(message_buffer[i].team_id, message_buffer[i].player_id);
            jumping_player_is[message_buffer[i].team_id] = message_buffer[i].player_id;
            printf("Team %d --> Player %d is starting to jump\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1);
        } else if (message_buffer[i].type == PULL_START_ID) {
            startPulling(message_buffer[i].team_id);
            printf("Team %d --> Player %d is starting to pull\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1);
        } else if (message_buffer[i].type == JUMP_END_ID) {
            stopJumping(message_buffer[i].team_id, message_buffer[i].player_id);
            jump_interval[message_buffer[i].team_id] = atof(message_buffer[i].content);
            printf("Team %d --> Player %d has finished jumping\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1);
        } else if (message_buffer[i].type == PULL_END_ID) {
            stopPulling(message_buffer[i].team_id);
            teams[message_buffer[i].team_id].score += (int) (0.1 * (game_config.max_score * (game_config.stabilization_time_min/1000.0) / atof(message_buffer[i].content)));
            printf("Team %d --> Player %d has finished pulling with time %s with score %d\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1,message_buffer[i].content,teams[message_buffer[i].team_id].score);
            if (teams[message_buffer[i].team_id].score >= game_config.max_score) {
                end_game(message_buffer[i].team_id);
                is_game_over=2;
                break;
            }
        } else if (message_buffer[i].type == ENERGY_ID) {
            if (atoi(message_buffer[i].content) == 0) {
                player_color[message_buffer[i].team_id][message_buffer[i].player_id] = 0.2f;
            }
            teamsInterface[message_buffer[i].team_id][message_buffer[i].player_id].energy = atoi(message_buffer[i].content);
            printf("Team %d --> Player %d has %s energy\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1, message_buffer[i].content);
        }
    }

    glutPostRedisplay();
}

void handle_alarm() {
    printf("Referee: Maximum time reached. Ending game.\n");
    is_game_over=1;
}

void handle_alarm_to_end(){
    is_game_over=3;
}

void end_game(int winner) {
    glutDisplayFunc(display_result);
    int max_score = teams[0].score;
    int winning_team_id = 0;
    int draw_count = 0;
    int draw_teams[3] = {0, -1, -1};

    for (int j = 1; j < 3; j++) {
        if (teams[j].score > max_score) {
            max_score = teams[j].score;
            winning_team_id = j;
            draw_count = 0;
            draw_teams[0] = j;
            draw_teams[1] = -1;
            draw_teams[2] = -1;
        } else if (teams[j].score == max_score) {
            draw_teams[++draw_count] = j;
        }
    }

    if (winner == -1) {
        if (draw_count == 0) {
            snprintf(result_game_buffer, sizeof(result_game_buffer), "Team %d has won the game with a score of %d", winning_team_id + 1, max_score);
        } else if (draw_count == 1) {
            snprintf(result_game_buffer, sizeof(result_game_buffer), "Teams %d and %d have drawn the game with a score of %d", draw_teams[0] + 1, draw_teams[1] + 1, max_score);
        } else {
            snprintf(result_game_buffer, sizeof(result_game_buffer), "The game is a draw with a score of %d", max_score);
        }
    } else {
        snprintf(result_game_buffer, sizeof(result_game_buffer), "Team %d has won the game with a score of %d", winner + 1, teams[winner].score);
    }

    system("rm /tmp/bungee_*");
    printf("Referee: Removed all FIFOs\n");

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            kill(teams[i].players[j], SIGKILL);
        }
        kill(assistant_referees[i], SIGKILL);
    }

    signal(SIGALRM, handle_alarm_to_end);
    alarm(8);
}

void display_result(){
    glClear(GL_COLOR_BUFFER_BIT);
    drawGameResult(result_game_buffer);
    glutSwapBuffers();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawBridge();
    drawReferees(assistantRefereePositions[0], baseBridgeHeight, 1.0f, 1.0f, 0.0f,0,"");
    drawReferees(assistantRefereePositions[1], baseBridgeHeight, 1.0f, 1.0f, 0.0f,0,"");
    drawReferees(assistantRefereePositions[2], baseBridgeHeight, 1.0f, 1.0f, 0.0f,0,"");

    float waveAngle = 10 * sin(glutGet(GLUT_ELAPSED_TIME) / 100.0); // Waving flag angle
    drawRefereeWithFlag(-0.9f, 1.1f, waveAngle);

    drawScore();
    drawEnergy();
    drawSetting();

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
                jumper->velocity += gravity;
                jumper->jumpHeight += jumper->velocity;

                if (jumper->jumpHeight < -0.6f) {
                    jumper->velocity = -jumper->velocity * springFactor;
                    jumper->jumpHeight = -0.6f;
                }

                float playerX = jumper->x;
                float playerY = jumper->y + jumper->jumpHeight;
                drawHumanPlayer(playerX, playerY, r, g, b,player_color[t][p], p);
            }else if (jumper->isStuck && (left->isPulling || right->isPulling)) {
                char message[50];
                sprintf(message, "Jumping time = %.2f", jump_interval[t]);
                drawReferees(assistantRefereePositions[t], baseBridgeHeight, 1.0f, 1.0f, 0.0f,1,message);

                if(jumper->jumpHeight >= baseBridgeHeight){
                    jumper->jumpHeight = baseBridgeHeight;
                }
                else{
                    jumper->jumpHeight += 0.001f;
                }
                jumper->x = teamStartPositions[t] + (p - 1) * sideXOffset;
                jumper->y = jumper->jumpHeight;
                drawHumanPlayer(jumper->x, jumper->y, r, g, b,player_color[t][p], p);
            }else{
                jumper->x = teamStartPositions[t] + (p - 1) * sideXOffset;
                jumper->y = baseBridgeHeight;
                drawHumanPlayer(jumper->x, jumper->y, r, g, b,player_color[t][p], p);
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
    for (float x = -1.0f; x <= 1.0f; x+=0.25f) {
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
        }
    }
}

void drawEnergy(){
    for (int t = 0; t < 3; t++){
        for (int p = 0; p < 3; p++){
            glColor3f(0, 0, 0);
            glRasterPos2f(teamsInterface[t][1].x - 0.05f, 0.8f - p * 0.1f);
            char energyStr[30];
            sprintf(energyStr, "Player: %d Energy=%d",p+1,teamsInterface[t][p].energy);
            for (char *c = energyStr; *c != '\0'; c++){
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
            }
        }
    }
}


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

void drawSetting(){
    glColor3f(0, 0, 0);
    glRasterPos2f(0.85f, 0.9f);
    char scoreStr[30];
    sprintf(scoreStr, "Max Score %d", game_config.max_score);
    for (char *c = scoreStr; *c != '\0'; c++){
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    glColor3f(0, 0, 0);
    glRasterPos2f(0.86f, 0.8f);
    char timeStr[30];
    sprintf(timeStr, "Max Time %d",game_config.max_time);
    for (char *c = timeStr; *c != '\0'; c++){
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}


void startPulling(int teamIndex){
    Player *left = &teamsInterface[teamIndex][(jumping_player_is[teamIndex] + 1) % 3];
    Player *right = &teamsInterface[teamIndex][(jumping_player_is[teamIndex] + 2) % 3];
    left->isPulling = 1;
    right->isPulling = 1;
}

void stopPulling(int teamIndex){
    Player *jumper = &teamsInterface[teamIndex][jumping_player_is[teamIndex]];
    Player *left = &teamsInterface[teamIndex][(jumping_player_is[teamIndex] + 1) % 3];
    Player *right = &teamsInterface[teamIndex][(jumping_player_is[teamIndex] + 2) % 3];
    jumping_player_is[teamIndex] = -1;
    jumper->isStuck = 0;
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