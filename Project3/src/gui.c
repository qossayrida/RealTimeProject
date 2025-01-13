#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glut.h>
#include <stdbool.h>

#define MAX_AGENCY_THREADS 200
#define MAX_CITIZEN_THREADS 200
#define MAX_ENEMY_PROCESSES 200
#define MAX_GROUPS 200
#define MAX_MEMBERS_PER_GROUP 200

// We'll allow multiple simultaneous visits
#define MAX_VISITS 200

typedef enum {
    STATE_ALIVE,
    STATE_INJURED,
    STATE_CAUGHT,
    STATE_DEAD
} MemberState;

typedef struct {
    char name[32];
    int  number;
    bool isSpy;
    bool killed;
    MemberState state;
    float suspicion;
    float x, y;    
    float whiteTimer; 
} Person;

typedef struct {
    char  groupName[32];
    Person members[MAX_MEMBERS_PER_GROUP];
    int   numMembers;
    bool  hasEnemy;
    bool  attackActive;
    float attackTimer;
    char groupType[32];
} ResistanceGroup;

/******************************************************************************
 * VisitState for each visiting citizen
 ******************************************************************************/
typedef enum {
    VISIT_NONE,
    VISIT_TRAVEL_TO,
    VISIT_STAYING,
    VISIT_TRAVEL_BACK
} VisitState;

typedef struct {
    bool  active;
    VisitState state;
    float travelTime;     // how long to travel each way
    float stayTime;       // how long to stay at group
    float timer;          // increments each frame
    float startX, startY; // citizen's base coords
    float endX,   endY;   // position near group
    Person person;        // copy of the traveling citizen
    int    citizenIndex;  // which slot in g_citizens
    char   groupName[32]; // group being visited
} Visit;

/******************************************************************************
 * Globals
 ******************************************************************************/
static Person g_agency[MAX_AGENCY_THREADS];
static int    g_numAgency = 0;

static Person g_citizens[MAX_CITIZEN_THREADS];
static int    g_numCitizens = 0;

static Person g_enemies[MAX_ENEMY_PROCESSES];
static int    g_numEnemies = 0;

static Person a_enemy;            
static bool   a_enemyActive = false; // indicates we created a single agency enemy

static ResistanceGroup g_groups[MAX_GROUPS];
static int  g_numGroups = 0;

// We'll store up to 10 concurrent visits
static Visit g_visits[MAX_VISITS];

// Window
static int g_winW = 1920, g_winH = 1080;

// how far horizontally we’ve scrolled
static float g_scrollX = 0.0f; 

// Info overlay
static bool  g_showInfo       = false;
static char  g_infoCategory[64];
static char  g_infoName[64];
static char  g_infoStatus[64];
static char   g_infoType[64];
static float g_infoSuspicion  = 0.0f;
static int   g_infoMembers    = 0;   // if we want to show group’s member count
static int   g_infoPID        = -1;  // some “process ID” if you store it

/******************************************************************************
 * Function Prototypes
 ******************************************************************************/
static void initOpenGL();
static void displayCallback();
static void reshapeCallback(int w, int h);
static void keyboardCallback(unsigned char key, int x, int y);
static void mouseCallback(int button, int state, int mx, int my);
static void timerCallback(int val);

static void drawScene();
static void drawAgencySection();
static void drawCitizenSection();
static void drawGroupsSection();
static void drawLegend();
static void drawInfoOverlay();

static void drawSquare(float cx, float cy, float sz);
static void drawCircle(float cx, float cy, float r);
static void drawTriangle(float cx, float cy, float sz, const char *text);
static void drawText(float x, float y, const char *str, void *font);

static void pickColor(Person *p);

// Create/modify
void createAgencyThread(const char *name);
void killAgencyThread(const char *name);
void createCitizen(const char *name, bool isSpy);
void killCitizen(const char *name);
void createGroup(const char *groupName, const char *groupType);
void createEnemy(const char *enemyName, bool forResistant, const char *groupID);
void addGroupMember(const char *groupName, const char *memberName, bool isSpy, MemberState st);

// Attack & Citizen visits
void enemyAttack(const char *groupName);
void citizenVisitGroup(const char *citizenName, const char *groupName, float stayDuration);

// Visit internals
static void updateVisit(Visit *v, float dt);
static void drawVisit(Visit *v);
static bool  agencyAttackActive = false;
static float agencyAttackTimer  = 0.0f;

/******************************************************************************
 * initOpenGL()
 ******************************************************************************/
static void initOpenGL()
{
    glClearColor(0.9f, 0.9f, 0.95f, 1.0f);
}

/******************************************************************************
 * displayCallback()
 ******************************************************************************/
static void displayCallback()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // SHIFT everything left by g_scrollX
    glPushMatrix();
    glTranslatef(-g_scrollX, 0, 0);

    drawScene();

    glPopMatrix(); // revert
    drawInfoOverlay();

    glutSwapBuffers();
}

/******************************************************************************
 * reshapeCallback()
 ******************************************************************************/
static void reshapeCallback(int w, int h)
{
    g_winW = w;
    g_winH = h;
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);

    glMatrixMode(GL_MODELVIEW);
}

/******************************************************************************
 * keyboardCallback()
 ******************************************************************************/
static void keyboardCallback(unsigned char key, int x, int y)
{
    if (key == 'q' || key == 27) {
        exit(0);
    }
    else if(key == 'a') {
        if(g_scrollX > 0) g_scrollX -= 50; // scroll left
        glutPostRedisplay();
    }
    else if(key == 'd') {
        g_scrollX += 50; // scroll right
        glutPostRedisplay();
    }
}

/******************************************************************************
 * mouseCallback()
 *   - Adjust bounding boxes so you don't have to click "too high."
 *   - Also handle group label clicks => show group info
 ******************************************************************************/
static void mouseCallback(int button, int state, int mx, int my)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        float clickX = (float)mx;
        float clickY = (float)(g_winH - my);

        // reset info
        g_showInfo      = false;
        strcpy(g_infoCategory,"");
        strcpy(g_infoName,"");
        strcpy(g_infoStatus,"");
        g_infoSuspicion = 0.0f;
        g_infoPID       = -1;
        g_infoMembers   = 0;

        // 1) Check Agency bounding boxes
        float sqSize = 30;
        float startY = g_winH - 100;
        for (int i = 0; i < g_numAgency; i++)
        {
            float x = 50, y = startY - i * (sqSize + 15);

            // bounding box
            if (clickX >= x && clickX <= (x + sqSize) &&
                clickY >= y && clickY <= (y + sqSize))
            {
                g_showInfo = true;
                strcpy(g_infoCategory, "Agency");
                // Show name as "Thread: <name>"
                sprintf(g_infoName, "%s", g_agency[i].name);

                if (g_agency[i].killed)
                    strcpy(g_infoStatus, "Dead");
                else
                    strcpy(g_infoStatus, "Alive");

                g_infoSuspicion = g_agency[i].suspicion;
                // We can store a "PID" if you like => g_agency[i].number ?

                glutPostRedisplay();
                return;
            }
        }

        // 2) Check single agency enemy (triangle near "Counter Espionage Agency")
        // Suppose we place it around (300, g_winH-65)
        if(a_enemyActive){
            float triX = 300, triY = g_winH - 65;
            float triSz= 25;
            // bounding box for triangle => from (triX - triSz, triY - triSz)
            // to (triX + triSz, triY + triSz). A bit approximate
            if ( clickX >= (triX - triSz) && clickX <= (triX + triSz) &&
                 clickY >= (triY - triSz) && clickY <= (triY + triSz) )
            {
                g_showInfo = true;
                strcpy(g_infoCategory, "Agency Enemy");
                sprintf(g_infoName, "ID: %s", a_enemy.name); // or number?
                strcpy(g_infoStatus, a_enemy.killed ? "Killed" : "Alive");
                g_infoSuspicion = a_enemy.suspicion;
                glutPostRedisplay();
                return;
            }
        }

        // 3) Check Citizens => circle bounding
        float radius = 20.0f;
        float cStartX = 50, cStartY = 200;
        for (int i = 0; i < g_numCitizens; i++)
        {
            if (!strlen(g_citizens[i].name)) continue; // hidden if traveling
            float cx = cStartX + i * (radius * 2 + 15);
            float cy = cStartY;
            float dx = clickX - cx, dy = clickY - cy;
            if ((dx*dx + dy*dy) <= (radius*radius))
            {
                g_showInfo = true;
                if (g_citizens[i].isSpy)
                    strcpy(g_infoCategory, "Citizen (Spy)");
                else
                    strcpy(g_infoCategory, "Citizen");

                // Name => e.g. "C3"
                strcpy(g_infoName, g_citizens[i].name);
                if (g_citizens[i].killed)
                    strcpy(g_infoStatus, "Dead");
                else
                    strcpy(g_infoStatus, "Alive");

                g_infoSuspicion = g_citizens[i].suspicion;
                // g_infoPID = ?

                glutPostRedisplay();
                return;
            }
        }

        // Now check groups
        const int groupsPerColumn = 5;
        float rowSpacing    = 150.0f;
        float columnSpacing = 300.0f;
        float baseX         = 350.0f;
        float baseY         = g_winH - 100.0f;

        // members layout
        const int membersPerRow = 8;
        float memberSpacingX    = 25.0f;
        float memberSpacingY    = 25.0f;
        float squareSize        = 20.0f;

        for (int i = 0; i < g_numGroups; i++)
        {
            int groupColumn = i / groupsPerColumn;
            int groupRow    = i % groupsPerColumn;

            float gx = baseX + groupColumn * columnSpacing;
            float gy = baseY - groupRow * rowSpacing;
            float groupBaseY = gy - 40;

            // bounding box for the group label => "G#i"
            // if label is drawn at (gx, gy), let's define 60×20 for it
            float labelW = 60, labelH = 20;
            if (clickX >= gx && clickX <= gx + labelW &&
                clickY >= gy && clickY <= gy + labelH)
            {
                // Clicked group label => show group info
                g_showInfo = true;
                strcpy(g_infoCategory,"Group");
                // e.g. "G#i"
                char lbl[32];
                sprintf(lbl, "G#%d", i);
                strcpy(g_infoName, lbl);

                // let's say the group has X members => show it
                g_infoMembers = g_groups[i].numMembers;
                // if you have a group PID => store in g_infoPID

                sprintf(g_infoStatus, "Members: %d", g_groups[i].numMembers);
                glutPostRedisplay();
                return;
            }

            // check enemy triangle
            if (g_groups[i].hasEnemy) {
                float tx = gx + 220;
                float ty = groupBaseY + 20;
                float tSz= 25;
                // bounding box for triangle (approx)
                if (clickX >= (tx - tSz) && clickX <= (tx + tSz) &&
                    clickY >= (ty - tSz) && clickY <= (ty + tSz))
                {
                    g_showInfo = true;
                    strcpy(g_infoCategory,"Enemy (Group)");
                    strcpy(g_infoName, g_groups[i].groupName);
                    strcpy(g_infoStatus, "Group's Enemy");
                    g_infoSuspicion=0.0f;
                    glutPostRedisplay();
                    return;
                }
            }

            // Now check each member => replicate the row/col logic
            for (int m = 0; m < g_groups[i].numMembers; m++)
            {
                int memberRow = m / membersPerRow;
                int memberCol = m % membersPerRow;

                float sx = gx + (memberCol * memberSpacingX);
                float sy = groupBaseY - (memberRow * memberSpacingY);

                // bounding box => (sx,sy) to (sx+squareSize, sy+squareSize)
                if ( clickX >= sx && clickX <= (sx + squareSize) &&
                     clickY >= sy && clickY <= (sy + squareSize) )
                {
                    // clicked a group member
                    g_showInfo = true;

                    Person *mem = &g_groups[i].members[m];
                    strcpy(g_infoCategory,"Group Member");
                    strcpy(g_infoName, mem->name);

                    char stStr[64];
                    switch(mem->state){
                        case STATE_ALIVE:   strcpy(stStr,"Alive");   break;
                        case STATE_INJURED: strcpy(stStr,"Injured"); break;
                        case STATE_CAUGHT:  strcpy(stStr,"Caught");  break;
                        case STATE_DEAD:    strcpy(stStr,"Dead");    break;
                    }
                    if(mem->isSpy) {
                        char g_infoStatus[128];
                        snprintf(g_infoStatus,sizeof(g_infoStatus),
                                 "%s (Spy)", stStr);
                    } else {
                        snprintf(g_infoStatus,sizeof(g_infoStatus),
                                 "%s", stStr);
                    }
                    g_infoSuspicion = mem->suspicion;
                    glutPostRedisplay();
                    return;
                }
            }
        }

        // if we get here => no click match
        glutPostRedisplay();
    }
}

/******************************************************************************
 * timerCallback()
 ******************************************************************************/
static void timerCallback(int val)
{
    float dt = 0.033f; // ~30 fps

    // update group attacks
    for (int i = 0; i < g_numGroups; i++)
    {
        if (g_groups[i].attackActive)
        {
            g_groups[i].attackTimer += dt;
            if (g_groups[i].attackTimer > 2.0f)
            {
                g_groups[i].attackActive = false;
                g_groups[i].attackTimer = 0.0f;
            }
        }
    }
    
    if(agencyAttackActive){
        agencyAttackTimer += dt;

        if(agencyAttackTimer>2.0f){
            agencyAttackActive=false;
            agencyAttackTimer=0.0f;
        }
    }

    // update visits
    for (int i = 0; i < MAX_VISITS; i++)
    {
        if (g_visits[i].active)
        {
            updateVisit(&g_visits[i], dt);
        }
    }

    for (int i = 0; i < g_numGroups; i++) {
        for(int m = 0; m < g_groups[i].numMembers; m++){
            Person* pm = &g_groups[i].members[m];
            if(pm->whiteTimer > 0.0f) {
                pm->whiteTimer -= dt;
                if(pm->whiteTimer <= 0.0f) {
                    pm->whiteTimer = 0.0f;
                    
                    // Force them back to ALIVE or normal
                    pm->killed = false;
                    pm->state  = STATE_ALIVE;  // or whatever suits your logic
                }
            }
        }
    }

    for(int i=0; i<g_numAgency; i++){
        if(g_agency[i].whiteTimer>0.0f){
            g_agency[i].whiteTimer -= dt;
            if(g_agency[i].whiteTimer<0) g_agency[i].whiteTimer=0.0f;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(33, timerCallback, 0);
}

/******************************************************************************
 * drawScene()
 ******************************************************************************/
static void drawScene()
{
    drawAgencySection();
    drawCitizenSection();
    drawGroupsSection();
    drawLegend();

    // Draw all active visits
    for (int i = 0; i < MAX_VISITS; i++)
    {
        if (g_visits[i].active)
        {
            drawVisit(&g_visits[i]);
        }
    }
}

/******************************************************************************
 * drawAgencySection()
 *   - Also draw the single "agency enemy" triangle next to the title if active
 ******************************************************************************/
static void drawAgencySection()
{
    glColor3f(0, 0, 0);
    drawText(50, g_winH - 50, "Counter Espionage Agency", GLUT_BITMAP_HELVETICA_18);

    float cx = 300;
    float cy = g_winH - 65;
    float sz = 25.0f;

    // We'll do the blinking if "agencyAttackActive == true"
    if(a_enemyActive) {
        if (agencyAttackActive) {
            float flash = fabs(sinf(agencyAttackTimer * 10.0f));
            // This yields a color from 0.5 .. 1.0
            glColor3f(flash, 0, 0);
        }
        else {
            glColor3f(0, 0, 0);
        }
        drawTriangle(cx, cy, sz, "Agency Enemy");
    }

    float startY = g_winH - 100;
    float size = 30;
    for (int i = 0; i < g_numAgency; i++)
    {
        float x = 50, y = startY - i * (size + 15);

        if(g_agency[i].whiteTimer>0.0f){
            glColor3f(0.9f, 0.9f, 0.95f);
            drawSquare(x, y, 20);
            char agencyNumber[32];
            sprintf(agencyNumber, "M%d", g_agency[i].number);
            drawText(x + 8, y + 10, agencyNumber, GLUT_BITMAP_HELVETICA_12);
        }
        else{
            glColor3f(0.1f, 0.1f, 0.8f);
            drawSquare(x, y, size);
            glColor3f(1, 1, 1);

            char agencyNumber[32];
            sprintf(agencyNumber, "M%d", g_agency[i].number);
            drawText(x + 8, y + 10, agencyNumber, GLUT_BITMAP_HELVETICA_12);
        }

    }
}

/******************************************************************************
 * drawCitizenSection()
 ******************************************************************************/
static void drawCitizenSection()
{
    glColor3f(0, 0, 0);
    drawText(50, 250, "Citizens", GLUT_BITMAP_HELVETICA_18);

    float radius = 20;
    for (int i = 0; i < g_numCitizens; i++)
    {
        if (!strlen(g_citizens[i].name))
            continue; // hidden if visiting
        if (!g_citizens[i].killed)
        {
            pickColor(&g_citizens[i]);
            float cx = 50 + i * (radius * 2 + 15);
            float cy = 200;
            drawCircle(cx, cy, radius);
            glColor3f(0, 0, 0);
            char snum[10];
            sprintf(snum, "%d", g_citizens[i].number);
            drawText(cx - 5, cy - 5, snum, GLUT_BITMAP_HELVETICA_12);
        }
        else
        {
            // killed => grey circle + X
            float cx = 50 + i * (radius * 2 + 15);
            float cy = 200;
            glColor3f(0.5f, 0.5f, 0.5f);
            drawCircle(cx, cy, radius);
            glColor3f(1, 1, 1);
            drawText(cx - 5, cy - 5, "X", GLUT_BITMAP_HELVETICA_18);
        }
    }
}

/******************************************************************************
 * drawGroupsSection()
 *   - 5 groups per column
 *   - group label is clickable => show group info
 ******************************************************************************/
static void drawGroupsSection()
{
    glColor3f(0, 0, 0);
    drawText(350, g_winH - 50, "Resistance Groups", GLUT_BITMAP_HELVETICA_18);

    const int groupsPerColumn = 5;    // how many groups per column
    float rowSpacing    = 150.0f;
    float columnSpacing = 300.0f;
    float baseX         = 350.0f;
    float baseY         = g_winH - 100.0f;

    // We'll allow 8 members per row within each group
    const int membersPerRow = 8;
    float memberSpacingX    = 25.0f;  // space horizontally
    float memberSpacingY    = 25.0f;  // vertical distance between rows of members

    for (int i = 0; i < g_numGroups; i++)
    {
        int column = i / groupsPerColumn;
        int row    = i % groupsPerColumn;

        float gx = baseX + column * columnSpacing;
        float gy = baseY - row    * rowSpacing;

        // Label => "G#i"
        glColor3f(0, 0, 0);
        char groupLabel[32];
        sprintf(groupLabel, "G#%d", i);
        drawText(gx, gy, groupLabel, GLUT_BITMAP_HELVETICA_18);

        float groupBaseY = gy - 40;

        // Enemy => triangle
        float tx  = gx + 220;
        float ty  = groupBaseY + 20;
        float tSz = 25.0f;

        if (g_groups[i].attackActive)
        {
            float flash = fabs(sinf(g_groups[i].attackTimer * 10.0f));
            glColor3f(flash, 0, 0);
        }
        else
        {
            glColor3f(0, 0, 0);
        }
        drawTriangle(tx, ty, tSz, "Enemy");

        // Members with 8 per row
        for (int m = 0; m < g_groups[i].numMembers; m++)
        {
            Person *mem = &g_groups[i].members[m];

            // Compute row/col within the group’s members
            int memberRow = m / membersPerRow;  // 0.. for multiple rows
            int memberCol = m % membersPerRow;  // 0..7

            float sx = gx + (memberCol * memberSpacingX);
            float sy = groupBaseY - (memberRow * memberSpacingY);

            if(mem->whiteTimer>0.0f){
                glColor3f(0.9f, 0.9f, 0.95f);
                drawSquare(sx, sy, 20);
                glColor3f(0.9f, 0.9f, 0.95f);
                drawText(sx+4, sy+6, mem->name, GLUT_BITMAP_HELVETICA_10);
            }
            else{
                pickColor(mem);
                drawSquare(sx, sy, 20);

                glColor3f(0, 0, 0);

                // show e.g. mem->name or mem->number
                drawText(sx + 4, sy + 6, mem->name, GLUT_BITMAP_HELVETICA_10);
            }

        }
    }
}

/******************************************************************************
 * drawLegend()
 ******************************************************************************/
static void drawLegend()
{
    float w = 280, h = 300;
    float x = g_winW - w - 10, y = 10;

    glColor4f(0.8f, 0.8f, 0.8f, 0.7f);
    glBegin(GL_QUADS);
      glVertex2f(x, y);
      glVertex2f(x + w, y);
      glVertex2f(x + w, y + h);
      glVertex2f(x, y + h);
    glEnd();

    glColor3f(0, 0, 0);
    drawText(x + 10, y + h - 20, "Legend", GLUT_BITMAP_HELVETICA_18);

    float lx = x + 20, ly = y + h - 40;

    // Agency alive
    glColor3f(0.1f, 0.1f, 0.8f);
    drawSquare(lx, ly, 15);
    glColor3f(0, 0, 0);
    drawText(lx + 25, ly + 5, "= Agency Thread (Alive)", GLUT_BITMAP_HELVETICA_12);
    ly -= 20;

    // Agency killed
    glColor3f(0.8f, 0, 0);
    drawSquare(lx, ly, 15);
    glColor3f(0, 0, 0);
    drawText(lx + 25, ly + 5, "= Agency Thread (Killed)", GLUT_BITMAP_HELVETICA_12);
    ly -= 20;

    // Agency enemy
    glColor3f(0,0,0);
    drawTriangle(lx + 12, ly + 10, 10, NULL);
    drawText(lx + 25, ly + 6, "= Agency Enemy", GLUT_BITMAP_HELVETICA_12);
    ly -= 20;

    // Citizen normal alive
    glColor3f(0.2f, 0.8f, 0.2f);
    drawCircle(lx + 7, ly + 7, 7);
    glColor3f(0, 0, 0);
    drawText(lx + 25, ly + 3, "= Citizen Normal (Alive)", GLUT_BITMAP_HELVETICA_12);
    ly -= 20;

    // Citizen spy alive
    glColor3f(1.0f, 0.2f, 0.2f);
    drawCircle(lx + 7, ly + 7, 7);
    glColor3f(0, 0, 0);
    drawText(lx + 25, ly + 3, "= Citizen Spy (Alive)", GLUT_BITMAP_HELVETICA_12);
    ly -= 20;

    // Group normal alive => green
    glColor3f(0.3f, 1.0f, 0.3f);
    drawSquare(lx, ly, 12);
    glColor3f(0, 0, 0);
    drawText(lx + 20, ly + 3, "= Group Member (Alive)", GLUT_BITMAP_HELVETICA_12);
    ly -= 20;

    // Normal injured => darker green
    glColor3f(0.15f, 0.5f, 0.15f);
    drawSquare(lx, ly, 12);
    glColor3f(0, 0, 0);
    drawText(lx + 20, ly + 3, "= Group Member (Injured)", GLUT_BITMAP_HELVETICA_12);
    ly -= 20;

    // Spy alive => magenta
    glColor3f(1.0f, 0.0f, 1.0f);
    drawSquare(lx, ly, 12);
    glColor3f(0, 0, 0);
    drawText(lx + 20, ly + 3, "= Group Spy (Alive)", GLUT_BITMAP_HELVETICA_12);
    ly -= 20;

    // Spy injured => darker magenta
    glColor3f(0.5f, 0.0f, 0.5f);
    drawSquare(lx, ly, 12);
    glColor3f(0, 0, 0);
    drawText(lx + 20, ly + 3, "= Group Spy (Injured)", GLUT_BITMAP_HELVETICA_12);
    ly -= 20;

    // Caught => orange
    glColor3f(1.0f, 0.6f, 0.0f);
    drawSquare(lx, ly, 12);
    glColor3f(0, 0, 0);
    drawText(lx + 20, ly + 3, "= Group Member (Caught)", GLUT_BITMAP_HELVETICA_12);
    ly -= 20;

    // Dead => grey
    glColor3f(0.5f, 0.5f, 0.5f);
    drawSquare(lx, ly, 12);
    glColor3f(0, 0, 0);
    drawText(lx + 20, ly + 3, "= Group Member (Dead)", GLUT_BITMAP_HELVETICA_12);
    ly -= 20;
}

/******************************************************************************
 * drawInfoOverlay()
 *   - Show customized overlays for Agency, Citizen, Enemy, or Group
 ******************************************************************************/
static void drawInfoOverlay()
{
    if (!g_showInfo) return;

    float w = 300, h = 140; // slightly taller
    float x = 50,  y = 50;

    // background
    glColor4f(0, 0, 0, 0.4f);
    glBegin(GL_QUADS);
      glVertex2f(x,   y);
      glVertex2f(x+w, y);
      glVertex2f(x+w, y+h);
      glVertex2f(x,   y+h);
    glEnd();

    // Title
    glColor3f(1, 1, 1);
    drawText(x + 10, y + h - 20, "Info", GLUT_BITMAP_HELVETICA_18);

    char line[128];

    // Category line
    snprintf(line, sizeof(line), "Category: %s", g_infoCategory);
    drawText(x + 10, y + h - 40, line, GLUT_BITMAP_HELVETICA_12);

    // The rest depends on category
    if(!strcmp(g_infoCategory,"Agency")){

    }
    else if(!strcmp(g_infoCategory,"Agency Enemy")){
        // single enemy for the agency
        snprintf(line, sizeof(line), "%s", g_infoName);
        drawText(x+10, y+h-60, line, GLUT_BITMAP_HELVETICA_12);
    }
    else if(!strcmp(g_infoCategory,"Citizen") || !strcmp(g_infoCategory,"Citizen (Spy)")){
        // show citizen info
        snprintf(line, sizeof(line), "Number: %s", g_infoName);
        drawText(x+10, y+h-60, line, GLUT_BITMAP_HELVETICA_12);
    }
    else if(!strcmp(g_infoCategory,"Group")){
        // show group info => e.g. "G#2"

        snprintf(line, sizeof(line), "Number: %s", g_infoName);
        drawText(x+10, y+h-60, line, GLUT_BITMAP_HELVETICA_12);

        snprintf(line, sizeof(line), "Label: %s", g_infoName);
        drawText(x+10, y+h-80, line, GLUT_BITMAP_HELVETICA_12);

        // We store number of members in g_infoMembers
        snprintf(line, sizeof(line), "Members: %d", g_infoMembers);
        drawText(x+10, y+h-100, line, GLUT_BITMAP_HELVETICA_12);

        // We store number of members in g_infoMembers
        if (strcmp(g_infoType, "0")){
            snprintf(line, sizeof(line), "Type: %s", "Military  Type");
        }
        else{
            snprintf(line, sizeof(line), "Type: %s", "Social Type");
        }
        drawText(x+10, y+h-120, line, GLUT_BITMAP_HELVETICA_12);

        // If you have a PID, you can do:
        if(g_infoPID>=0){
            snprintf(line,sizeof(line),"PID: %d", g_infoPID);
            drawText(x+10, y+h-140, line, GLUT_BITMAP_HELVETICA_12);
        }
    }
    else if(!strcmp(g_infoCategory,"Group Member")){
        // e.g. a normal or spy member
        snprintf(line, sizeof(line), "Name: %s", g_infoName);
        drawText(x+10, y+h-60, line, GLUT_BITMAP_HELVETICA_12);

        snprintf(line, sizeof(line), "Status: %s", g_infoStatus);
        drawText(x+10, y+h-80, line, GLUT_BITMAP_HELVETICA_12);

        snprintf(line, sizeof(line), "Suspicion: %.1f", g_infoSuspicion);
        drawText(x+10, y+h-100, line, GLUT_BITMAP_HELVETICA_12);
    }
    else if(!strcmp(g_infoCategory," Enemy") || !strcmp(g_infoCategory,"Enemy (Group)")){
        // group enemy
        snprintf(line, sizeof(line), "%s", g_infoName);
        drawText(x+10, y+h-60, line, GLUT_BITMAP_HELVETICA_12);
    }
    else {
        // fallback
        snprintf(line, sizeof(line), "Name: %s", g_infoName);
        drawText(x+10, y+h-60, line, GLUT_BITMAP_HELVETICA_12);
        drawText(x+10, y+h-80, g_infoStatus, GLUT_BITMAP_HELVETICA_12);
    }
}

/******************************************************************************
 * pickColor(p)
 ******************************************************************************/
static void pickColor(Person *p)
{
    // If we still have whiteTimer>0 => forcibly white color
    if (p->whiteTimer > 0.0f) {
        glColor3f(1.0f, 1.0f, 1.0f); // bright white
        return;
    }

    float r = 0, g = 0, b = 0;
    if (p->isSpy) {
        // For spy citizens or group members
        if (p->state == STATE_ALIVE) {
            r=1.0f; g=0.2f; b=0.2f;
        }
        else if (p->state == STATE_INJURED) {
            r=0.5f; g=0.1f; b=0.1f;
        }
        else if (p->state == STATE_CAUGHT) {
            r=1.0f; g=0.6f; b=0.0f;
        }
        else if (p->state == STATE_DEAD) {
            r=0.5f; g=0.5f; b=0.5f;
        }
    } else {
        // Normal
        if (p->state == STATE_ALIVE) {
            r=0.2f; g=0.8f; b=0.2f;
        }
        else if (p->state == STATE_INJURED) {
            r=0.1f; g=0.4f; b=0.1f;
        }
        else if (p->state == STATE_CAUGHT) {
            r=1.0f; g=0.6f; b=0.0f;
        }
        else if (p->state == STATE_DEAD) {
            r=0.5f; g=0.5f; b=0.5f;
        }
    }
    glColor3f(r, g, b);
}

/******************************************************************************
 * drawSquare()
 ******************************************************************************/
static void drawSquare(float cx, float cy, float sz)
{
    glBegin(GL_QUADS);
      glVertex2f(cx,      cy);
      glVertex2f(cx + sz, cy);
      glVertex2f(cx + sz, cy + sz);
      glVertex2f(cx,      cy + sz);
    glEnd();
}

/******************************************************************************
 * drawCircle()
 ******************************************************************************/
static void drawCircle(float cx, float cy, float r)
{
    glBegin(GL_TRIANGLE_FAN);
      glVertex2f(cx, cy);
      int segs = 48;
      for (int i = 0; i <= segs; i++)
      {
        float angle = 2.0f * M_PI * (float)i / (float)segs;
        float x = cx + r * cosf(angle);
        float y = cy + r * sinf(angle);
        glVertex2f(x, y);
      }
    glEnd();
}

/******************************************************************************
 * drawTriangle()
 *   - text is placed *inside* the triangle better
 ******************************************************************************/
static void drawTriangle(float cx, float cy, float sz, const char *text)
{
    // Draw an upright triangle
    glBegin(GL_TRIANGLES);
      glVertex2f(cx,       cy + sz);
      glVertex2f(cx - sz,  cy - sz*0.6f);
      glVertex2f(cx + sz,  cy - sz*0.6f);
    glEnd();

    if (text) {
        // place text near middle
        glColor3f(1,1,1);
        float tx = cx - 4.0f*(float)strlen(text);
        float ty = cy - sz*0.2f;
        drawText(tx, ty, text, GLUT_BITMAP_HELVETICA_10);
    }
}

/******************************************************************************
 * drawText()
 ******************************************************************************/
static void drawText(float x, float y, const char *str, void *font)
{
    glRasterPos2f(x, y);
    while (*str) {
        glutBitmapCharacter(font, *str);
        str++;
    }
}

/******************************************************************************
 * Implementation: Create/Modify
 ******************************************************************************/
void createAgencyThread(const char *name)
{
    if (g_numAgency >= MAX_AGENCY_THREADS) return;
    Person *p = &g_agency[g_numAgency++];
    strcpy(p->name, name);
    p->number    = g_numAgency;
    p->isSpy     = false;
    p->killed    = false;
    p->state     = STATE_ALIVE;
    p->suspicion = 0;
}

void killAgencyThread(const char *name)
{
    for (int i = 0; i < g_numAgency; i++)
    {
        if (!strcmp(g_agency[i].name, name))
        {
            g_agency[i].killed = true;
            return;
        }
    }
}

void createCitizen(const char *name, bool isSpy)
{
    if (g_numCitizens >= MAX_CITIZEN_THREADS) return;
    Person *p = &g_citizens[g_numCitizens++];
    strcpy(p->name, name);
    p->number    = g_numCitizens;
    p->isSpy     = isSpy;
    p->killed    = false;
    p->state     = STATE_ALIVE;
    p->suspicion = 0;
}

void killCitizen(const char *name)
{
    for (int i = 0; i < g_numCitizens; i++)
    {
        if (!strcmp(g_citizens[i].name, name))
        {
            g_citizens[i].killed = true;
            return;
        }
    }
}

void createGroup(const char *groupName, const char *groupType)
{
    if(g_numGroups >= MAX_GROUPS) {
        printf("createGroup: max group limit reached!\n");
        return;
    }

    ResistanceGroup *rg = &g_groups[g_numGroups++];
    strcpy(rg->groupName, groupName);
    rg->numMembers   = 0;
    rg->hasEnemy     = false;
    rg->attackActive = false;
    rg->attackTimer  = 0.0f;
    strcpy(rg->groupType, groupType);
    strcpy(g_infoType, groupType);

    printf("[GUI] Created group: %s (index %d)\n", groupName, g_numGroups-1);
    glutPostRedisplay();
}

/******************************************************************************
 * createEnemy()
 ******************************************************************************/
void createEnemy(const char *enemyName, bool forResistant, const char *groupID)
{
    if(!forResistant) {
        // We create the single agency enemy
        strcpy(a_enemy.name, enemyName);
        a_enemy.killed    = false;
        a_enemy.number    = 0;        // or enemyID
        a_enemy.state     = STATE_ALIVE;
        a_enemy.suspicion = -1.0f;
        a_enemy.x         = 0.0f;
        a_enemy.y         = 0.0f;
        a_enemy.isSpy     = false;    // or parse from somewhere
        a_enemyActive     = true;

        printf("[GUI] Created agency enemy: %s\n", enemyName);
    }
    else {
        // We add an enemy for a group in g_enemies[]
        if(g_numEnemies >= MAX_ENEMY_PROCESSES) {
            printf("createEnemy: max enemies limit reached!\n");
            return;
        }
        Person *eg = &g_enemies[g_numEnemies++];
        strcpy(eg->name, enemyName);
        eg->killed    = false;
        eg->number    = g_numEnemies;
        eg->state     = STATE_ALIVE;
        eg->suspicion = -1.0f;
        eg->x         = 0.0f;
        eg->y         = 0.0f;
        eg->isSpy     = false; // or parse from content if needed

        // also set hasEnemy=true in the specified group
        // groupID might be "2" => means "G2"
        char gName[32];
        sprintf(gName, "G%s", groupID);

        g_groups[g_numEnemies - 1].hasEnemy = true;
        printf("[GUI] Created enemy '%s' for group %s (index %d)\n",
                enemyName, gName, g_numEnemies - 1);
        }
    glutPostRedisplay();
}

void addGroupMember(const char *groupName, const char *memberName, bool isSpy, MemberState st)
{
    for (int i = 0; i < g_numGroups; i++)
    {
        if (!strcmp(g_groups[i].groupName, groupName))
        {
            if (g_groups[i].numMembers >= MAX_MEMBERS_PER_GROUP) return;
            Person *pm = &g_groups[i].members[g_groups[i].numMembers++];
            strcpy(pm->name, memberName);
            pm->isSpy     = isSpy;
            pm->killed    = false;
            pm->state     = st;
            pm->suspicion = 0;
            return;
        }
    }
    printf("addGroupMember: group %s not found\n", groupName);
}

/******************************************************************************
 * enemyAttack()
 ******************************************************************************/
void startEnemyAttack(bool attackAgency, int ID)
{
    if (attackAgency) {
        agencyAttackActive = true;
        agencyAttackTimer  = 0.0f;
    }
    else {
        char targetName[32];
        sprintf(targetName, "E%d", ID);
        for (int i = 0; i < g_numGroups; i++)
        {
            if (!strcmp(g_enemies[i].name, targetName))
            {
                g_groups[i].attackActive = true;
                g_groups[i].attackTimer  = 0.0f;
                printf("[Attack] Group %s attacked.\n", g_groups[i].groupName);
                return;
            }
        }
        printf("[Attack] group %s not found.\n", targetName);
    }
    glutPostRedisplay();
}

/******************************************************************************
 * citizenVisitGroup(citizenName, groupName, stayDuration)
 ******************************************************************************/
void citizenVisitGroup(const char *citizenName, const char *groupName, float stayDuration)
{

    // find the citizen index
    int cidx = -1;
    for (int i = 0; i < g_numCitizens; i++)
    {
        if (!strcmp(g_citizens[i].name, citizenName))
        {
            cidx = i;
            break;
        }
    }
    if (cidx < 0)
    {
        printf("[Visit] Citizen %s not found.\n", citizenName);
        return;
    }

    // find a free visit slot
    int slot = -1;
    for (int i = 0; i < MAX_VISITS; i++)
    {
        if (!g_visits[i].active)
        {
            slot = i;
            break;
        }
    }
    if (slot < 0)
    {
        printf("[Visit] No free visit slot. Ignored.\n");
        return;
    }

    // fill the slot
    g_visits[slot].active      = true;
    g_visits[slot].state       = VISIT_TRAVEL_TO;
    g_visits[slot].travelTime  = 1.0f; // 1s to travel
    g_visits[slot].stayTime    = stayDuration;
    g_visits[slot].timer       = 0.0f;
    strcpy(g_visits[slot].groupName, groupName);

    // copy the citizen data
    g_visits[slot].person      = g_citizens[cidx];
    g_visits[slot].citizenIndex= cidx;

    // remove them from the displayed array => name=""
    g_citizens[cidx].name[0] = '\0';

    // compute start coords
    float radius=20.0f;
    float cStartX = 50 + cidx * (radius*2 + 15);
    float cStartY = 200;

    g_visits[slot].startX = cStartX;
    g_visits[slot].startY = cStartY;

    // find group coords
    const int groupsPerColumn = 5;
    float rowSpacing    = 150.0f;
    float columnSpacing = 300.0f;
    float baseX         = 350.0f;
    float baseY         = g_winH - 100.0f;

    int grpIndex = -1;
    for(int i=0; i<g_numGroups; i++){
        if(!strcmp(g_groups[i].groupName, groupName)){
            grpIndex = i; 
            break;
        }
    }
    if(grpIndex<0){
        printf("citizenVisitGroup: group %s not found\n", groupName);
        return;
    }

    int col = grpIndex / groupsPerColumn;
    int row=  grpIndex % groupsPerColumn;

    float gx = baseX + col*columnSpacing;
    float gy = baseY - row*rowSpacing;
    float groupTitleX = gx;
    float groupTitleY = gy;

    // position the target ~ 40 px left from the group label
    float endX= groupTitleX - 40;
    float endY= groupTitleY;

    g_visits[slot].endX = endX;
    g_visits[slot].endY = endY;

    printf("[Visit] Citizen %s traveling to %s, stay=%.1f\n", citizenName, groupName, stayDuration);
}

/******************************************************************************
 * updateVisit()
 ******************************************************************************/
static void updateVisit(Visit *v, float dt)
{
    if(!v->active) return;

    v->timer += dt;
    switch(v->state)
    {
    case VISIT_TRAVEL_TO:
        if(v->timer >= v->travelTime) {
            v->timer = 0.0f;
            v->state = VISIT_STAYING;
        }
        break;

    case VISIT_STAYING:
        if(v->timer >= v->stayTime) {
            v->timer = 0.0f;
            v->state = VISIT_TRAVEL_BACK;
        }
        break;

    case VISIT_TRAVEL_BACK:
        if(v->timer >= v->travelTime) {
            v->timer = 0.0f;
            v->state = VISIT_NONE;
            v->active= false;

            // restore the citizen
            int cidx = v->citizenIndex;
            strcpy(g_citizens[cidx].name, v->person.name);
            g_citizens[cidx].isSpy    = v->person.isSpy;
            g_citizens[cidx].killed   = v->person.killed;
            g_citizens[cidx].state    = v->person.state;
            g_citizens[cidx].suspicion= v->person.suspicion;
        }
        break;

    default: break;
    }
}

/******************************************************************************
 * drawVisit()
 ******************************************************************************/
static void drawVisit(Visit *v)
{
    if(!v->active) return;
    if(v->state == VISIT_NONE) return;

    float t   = 0.0f;
    float xPos= 0.0f;
    float yPos= 0.0f;

    switch (v->state) {
    case VISIT_TRAVEL_TO:
        t = v->timer / v->travelTime; 
        xPos= v->startX + (v->endX - v->startX)*t;
        yPos= v->startY + (v->endY - v->startY)*t;
        break;

    case VISIT_STAYING:
        xPos= v->endX;
        yPos= v->endY;
        break;

    case VISIT_TRAVEL_BACK:
        t= v->timer / v->travelTime;
        xPos= v->endX + (v->startX - v->endX)*t;
        yPos= v->endY + (v->startY - v->endY)*t;
        break;

    default:
        return;
    }

    // Pick color based on the traveling citizen's spy status, state, etc.
    pickColor(&v->person);
    drawCircle(xPos, yPos, 20);

    // Draw the circle label => the citizen's number
    glColor3f(0,0,0);
    char numStr[16];
    snprintf(numStr, sizeof(numStr), "%d", v->person.number);
    drawText(xPos - 5, yPos - 5, numStr, GLUT_BITMAP_HELVETICA_12);
}

