#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>
#include <cstring>

// Window and game constants
const int WIDTH = 1000;
const int HEIGHT = 800;
const int CELL_SIZE = 20;
const int TOP_UI_HEIGHT_PIXELS = 80;
const int BOTTOM_UI_HEIGHT_PIXELS = 80;
const float TOP_UI_HEIGHT_NDC = (float)TOP_UI_HEIGHT_PIXELS / HEIGHT * 2.0f;
const float BOTTOM_UI_HEIGHT_NDC = (float)BOTTOM_UI_HEIGHT_PIXELS / HEIGHT * 2.0f;
const float GAME_AREA_TOP_NDC = 1.0f - TOP_UI_HEIGHT_NDC;
const float GAME_AREA_BOTTOM_NDC = -1.0f + BOTTOM_UI_HEIGHT_NDC;
const float GAME_AREA_LEFT_NDC = -1.0f;
const float GAME_AREA_RIGHT_NDC = 1.0f;
const int GAME_AREA_PIXEL_WIDTH = WIDTH;
const int GAME_AREA_PIXEL_HEIGHT = HEIGHT - TOP_UI_HEIGHT_PIXELS - BOTTOM_UI_HEIGHT_PIXELS;
const int gridWidth = GAME_AREA_PIXEL_WIDTH / CELL_SIZE;
const int gridHeight = GAME_AREA_PIXEL_HEIGHT / CELL_SIZE;
const int MAX_SNAKE_LENGTH = gridWidth * gridHeight;

// M_PI for some compilers
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

enum Direction { UP, DOWN, LEFT, RIGHT };
enum GameState { MENU, DIFFICULTY_SELECT, PLAYING, GAME_OVER, ABOUT, PAUSED };
enum Difficulty { EASY, MEDIUM, HARD };

struct Point { int x, y; };
struct Color { float r, g, b, a; };

const Color BG_COLOR        = {0.08f, 0.12f, 0.16f, 1.0f};
const Color SNAKE_HEAD_COLOR= {0.5f, 0.4f, 0.3f, 1.0f};
const Color SNAKE_BODY_COLOR= {0.15f, 0.5f, 0.25f, 1.0f};
const Color FOOD_COLOR      = {1.0f, 1.0f, 1.0f, 1.0f};
const Color UI_COLOR        = {0.1f, 0.2f, 0.3f, 0.9f};
const Color TEXT_COLOR      = {0.85f, 0.9f, 1.0f, 1.0f};
const Color ACCENT_COLOR    = {0.4f, 0.75f, 1.0f, 1.0f};
const Color GAME_BORDER_COLOR = {0.15f, 0.3f, 0.5f, 1.0f};

Point snake[MAX_SNAKE_LENGTH];
int snakeLen = 0;
Point food;
Direction dir = RIGHT;
GameState gameState = MENU;
Difficulty difficulty = MEDIUM;
int score = 0;
int selectedMenuItem = 0;
int selectedDifficulty = 1;
float animationTime = 0.0f;
float gameOverAnimation = 0.0f;

const char* STUDENT_NAME = "TIJUL KABIR TOHA";
const char* STUDENT_ID = "240113";

// --- Utility ---
double getUpdateInterval() {
    switch (difficulty) {
        case EASY: return 0.25;
        case MEDIUM: return 0.15;
        case HARD: return 0.08;
        default: return 0.15;
    }
}

bool isSnakeAt(int x, int y) {
    for (int i = 0; i < snakeLen; ++i)
        if (snake[i].x == x && snake[i].y == y)
            return true;
    return false;
}

void placeFood() {
    int x, y;
    do {
        x = rand() % gridWidth;
        y = rand() % gridHeight;
    } while (isSnakeAt(x, y));
    food.x = x; food.y = y;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// ---- Drawing Primitives ----
void drawGradientBackground() {
    glBegin(GL_QUADS);
    glColor4f(BG_COLOR.r * 1.2f, BG_COLOR.g * 1.2f, BG_COLOR.b * 1.2f, BG_COLOR.a);
    glVertex2f(-1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
    glColor4f(BG_COLOR.r, BG_COLOR.g, BG_COLOR.b, BG_COLOR.a);
    glVertex2f(1.0f, -1.0f); glVertex2f(-1.0f, -1.0f);
    glEnd();
}

void drawRoundedRect(float x, float y, float width, float height, float radius, Color color) {
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_QUADS);
    glVertex2f(x + radius, y);
    glVertex2f(x + width - radius, y);
    glVertex2f(x + width - radius, y + height);
    glVertex2f(x + radius, y + height);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(x, y + radius);
    glVertex2f(x + radius, y + radius);
    glVertex2f(x + radius, y + height - radius);
    glVertex2f(x, y + height - radius);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(x + width - radius, y + radius);
    glVertex2f(x + width, y + radius);
    glVertex2f(x + width, y + height - radius);
    glVertex2f(x + width - radius, y + height - radius);
    glEnd();
    int segments = 16;
    for (int corner = 0; corner < 4; corner++) {
        float cx, cy, start_angle_offset;
        switch (corner) {
            case 0: cx = x + radius; cy = y + radius; start_angle_offset = M_PI; break;
            case 1: cx = x + width - radius; cy = y + radius; start_angle_offset = 1.5 * M_PI; break;
            case 2: cx = x + width - radius; cy = y + height - radius; start_angle_offset = 0; break;
            case 3: cx = x + radius; cy = y + height - radius; start_angle_offset = 0.5 * M_PI; break;
        }
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= segments / 4; i++) {
            float angle = start_angle_offset + (i * M_PI / 2) / (segments / 4);
            glVertex2f(cx + radius * cos(angle), cy + radius * sin(angle));
        }
        glEnd();
    }
}

void drawCircle(float x, float y, float radius, Color color) {
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    int segments = 32;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        glVertex2f(x + radius * cos(angle), y + radius * sin(angle));
    }
    glEnd();
}

// --- Simple bitmap font for "drawText" ---
// Only capital Latin A-Z, 0-9, colon, dot, dash, space
void getCharPattern(char c, int pattern[7][5]) {
    // All zero
    for (int r=0;r<7;r++) for(int col=0;col<5;col++) pattern[r][col]=0;
    // Characters
    if (c>='a' && c<='z') c = c-'a'+'A';
    switch (c) {
        case '0': {int p[7][5]={{1,1,1,1,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case '1': {int p[7][5]={{0,0,1,0,0},{0,1,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{1,1,1,1,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case '2': {int p[7][5]={{1,1,1,1,1},{0,0,0,0,1},{0,0,0,0,1},{1,1,1,1,1},{1,0,0,0,0},{1,0,0,0,0},{1,1,1,1,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case '3': {int p[7][5]={{1,1,1,1,1},{0,0,0,0,1},{0,0,0,0,1},{1,1,1,1,1},{0,0,0,0,1},{0,0,0,0,1},{1,1,1,1,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case '4': {int p[7][5]={{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,1},{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case '5': {int p[7][5]={{1,1,1,1,1},{1,0,0,0,0},{1,0,0,0,0},{1,1,1,1,1},{0,0,0,0,1},{0,0,0,0,1},{1,1,1,1,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case '6': {int p[7][5]={{1,1,1,1,1},{1,0,0,0,0},{1,0,0,0,0},{1,1,1,1,1},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case '7': {int p[7][5]={{1,1,1,1,1},{0,0,0,0,1},{0,0,0,0,1},{0,0,0,1,0},{0,0,1,0,0},{0,1,0,0,0},{1,0,0,0,0}}; memcpy(pattern,p,sizeof(p)); break;}
        case '8': {int p[7][5]={{1,1,1,1,1},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,1},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case '9': {int p[7][5]={{1,1,1,1,1},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,1},{0,0,0,0,1},{0,0,0,0,1},{1,1,1,1,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'A': {int p[7][5]={{0,1,1,1,0},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'B': {int p[7][5]={{1,1,1,1,0},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,0},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,0}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'C': {int p[7][5]={{0,1,1,1,1},{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0},{0,1,1,1,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'D': {int p[7][5]={{1,1,1,1,0},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,0}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'E': {int p[7][5]={{1,1,1,1,1},{1,0,0,0,0},{1,0,0,0,0},{1,1,1,1,0},{1,0,0,0,0},{1,0,0,0,0},{1,1,1,1,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'F': {int p[7][5]={{1,1,1,1,1},{1,0,0,0,0},{1,0,0,0,0},{1,1,1,1,0},{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'G': {int p[7][5]={{0,1,1,1,1},{1,0,0,0,0},{1,0,0,0,0},{1,0,1,1,1},{1,0,0,0,1},{1,0,0,0,1},{0,1,1,1,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'H': {int p[7][5]={{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'I': {int p[7][5]={{1,1,1,1,1},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{1,1,1,1,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'J': {int p[7][5]={{1,1,1,1,1},{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1},{1,0,0,0,1},{0,1,1,1,0}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'K': {int p[7][5]={{1,0,0,0,1},{1,0,0,1,0},{1,0,1,0,0},{1,1,0,0,0},{1,0,1,0,0},{1,0,0,1,0},{1,0,0,0,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'L': {int p[7][5]={{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0},{1,1,1,1,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'M': {int p[7][5]={{1,0,0,0,1},{1,1,0,1,1},{1,0,1,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'N': {int p[7][5]={{1,0,0,0,1},{1,1,0,0,1},{1,0,1,0,1},{1,0,0,1,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'O': {int p[7][5]={{0,1,1,1,0},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{0,1,1,1,0}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'P': {int p[7][5]={{1,1,1,1,0},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,0},{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'Q': {int p[7][5]={{0,1,1,1,0},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,1,0,1},{1,0,0,1,0},{0,1,1,0,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'R': {int p[7][5]={{1,1,1,1,0},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,0},{1,0,1,0,0},{1,0,0,1,0},{1,0,0,0,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'S': {int p[7][5]={{0,1,1,1,1},{1,0,0,0,0},{1,0,0,0,0},{0,1,1,1,0},{0,0,0,0,1},{0,0,0,0,1},{1,1,1,1,0}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'T': {int p[7][5]={{1,1,1,1,1},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'U': {int p[7][5]={{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{0,1,1,1,0}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'V': {int p[7][5]={{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{0,1,0,1,0},{0,1,0,1,0},{0,0,1,0,0}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'W': {int p[7][5]={{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,1,0,1},{1,0,1,0,1},{1,1,0,1,1},{1,0,0,0,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'X': {int p[7][5]={{1,0,0,0,1},{0,1,0,1,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,1,0,1,0},{1,0,0,0,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'Y': {int p[7][5]={{1,0,0,0,1},{0,1,0,1,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0}}; memcpy(pattern,p,sizeof(p)); break;}
        case 'Z': {int p[7][5]={{1,1,1,1,1},{0,0,0,0,1},{0,0,0,1,0},{0,0,1,0,0},{0,1,0,0,0},{1,0,0,0,0},{1,1,1,1,1}}; memcpy(pattern,p,sizeof(p)); break;}
        case ':': {int p[7][5]={{0,0,0,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,1,0,0},{0,0,0,0,0}}; memcpy(pattern,p,sizeof(p)); break;}
        case '.': {int p[7][5]={{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,1,0,0},{0,0,0,0,0}}; memcpy(pattern,p,sizeof(p)); break;}
        case '-': {int p[7][5]={{0,0,0,0,0},{0,0,0,0,0},{1,1,1,1,1},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}}; memcpy(pattern,p,sizeof(p)); break;}
        default: break;
    }
}

void drawText(float x, float y, const char* text, float size, Color color) {
    glColor4f(color.r, color.g, color.b, color.a);
    float curX = x;
    float charWidth = size * 0.7f, charHeight = size;
    float pixelWidth = charWidth / 5.0f * 1.05f, pixelHeight = charHeight / 7.0f * 1.05f;
    for (const char* p = text; *p; ++p) {
        char c = *p;
        if (c == ' ') { curX += charWidth; continue; }
        int pattern[7][5]; getCharPattern(c, pattern);
        for (int row=0; row<7; row++) for (int col=0; col<5; col++)
            if (pattern[row][col]) {
                float px = curX + col * (charWidth / 5.0f);
                float py = y - row * (charHeight / 7.0f);
                glBegin(GL_QUADS);
                glVertex2f(px, py);
                glVertex2f(px + pixelWidth, py);
                glVertex2f(px + pixelWidth, py + pixelHeight);
                glVertex2f(px, py + pixelHeight);
                glEnd();
            }
        curX += charWidth + size * 0.1f;
    }
}

// --- Menu, Game, About, Pause, Game Over screens (simplified) ---
void drawMenu() {
    float titleY = 0.6f;
    float glowSize = 0.08f + 0.01f * sin(animationTime * 3.0f);
    drawText(-0.35f, titleY, "SNAKE GAME", glowSize, Color{ACCENT_COLOR.r, ACCENT_COLOR.g, ACCENT_COLOR.b, 0.3f});
    drawText(-0.35f, titleY, "SNAKE GAME", 0.08f, ACCENT_COLOR);
    const char* menuItems[3] = {"START", "ABOUT", "EXIT"};
    float menuY = 0.2f, menuSpacing = 0.15f;
    for (int i=0;i<3;i++) {
        Color itemColor = (i == selectedMenuItem) ? ACCENT_COLOR : TEXT_COLOR;
        float itemSize = (i == selectedMenuItem) ? 0.05f : 0.04f;
        float textWidth = strlen(menuItems[i]) * itemSize * 0.7f;
        if (i == selectedMenuItem)
            drawRoundedRect(-textWidth/2-0.05f, menuY-itemSize/2-0.02f, textWidth+0.1f, itemSize+0.04f, 0.02f, Color{ACCENT_COLOR.r, ACCENT_COLOR.g, ACCENT_COLOR.b, 0.2f});
        drawText(-textWidth/2, menuY, menuItems[i], itemSize, itemColor);
        menuY -= menuSpacing;
    }
}

void drawDifficultySelect() {
    drawText(-0.25f, 0.5f, "SELECT DIFFICULTY", 0.06f, ACCENT_COLOR);
    const char* diffs[3] = {"EASY", "MEDIUM", "HARD"};
    const char* descs[3] = {"SLOW", "NORMAL", "FAST"};
    float menuY = 0.2f, menuSpacing = 0.12f;
    for (int i=0;i<3;i++) {
        Color itemColor = (i == selectedDifficulty) ? ACCENT_COLOR : TEXT_COLOR;
        float itemSize = (i == selectedDifficulty) ? 0.05f : 0.04f;
        float textWidth = strlen(diffs[i]) * itemSize * 0.7f;
        if (i == selectedDifficulty)
            drawRoundedRect(-textWidth/2-0.05f, menuY-itemSize/2-0.02f, textWidth+0.1f, itemSize+0.04f, 0.02f, Color{ACCENT_COLOR.r, ACCENT_COLOR.g, ACCENT_COLOR.b, 0.2f});
        drawText(-textWidth/2, menuY, diffs[i], itemSize, itemColor);
        float descWidth = strlen(descs[i]) * 0.025f * 0.7f;
        drawText(-descWidth/2, menuY-0.06f, descs[i], 0.025f, Color{TEXT_COLOR.r, TEXT_COLOR.g, TEXT_COLOR.b, 0.7f});
        menuY -= menuSpacing;
    }
    drawText(-0.3f, -0.5f, "PRESS ENTER TO START", 0.03f, Color{TEXT_COLOR.r, TEXT_COLOR.g, TEXT_COLOR.b, 0.8f});
    drawText(-0.25f, -0.6f, "PRESS ESC TO GO BACK", 0.03f, Color{TEXT_COLOR.r, TEXT_COLOR.g, TEXT_COLOR.b, 0.8f});
}

void drawAbout() {
    drawText(-0.15f, 0.6f, "ABOUT", 0.08f, ACCENT_COLOR);
    drawRoundedRect(-0.6f, -0.1f, 1.2f, 0.5f, 0.05f, Color{UI_COLOR.r, UI_COLOR.g, UI_COLOR.b, 0.8f});
    float infoY = 0.3f;
    char buf[128];
    snprintf(buf, sizeof(buf), "DEVELOPER: %s", STUDENT_NAME);
    drawText(-0.5f, infoY, buf, 0.04f, TEXT_COLOR);
    snprintf(buf, sizeof(buf), "STUDENT ID: %s", STUDENT_ID);
    drawText(-0.5f, infoY-0.08f, buf, 0.04f, TEXT_COLOR);
    drawText(-0.5f, infoY-0.16f, "COURSE: OBJECT ORIENTED PROGRAMMING", 0.04f, TEXT_COLOR);
    drawText(-0.5f, infoY-0.24f, "TECHNOLOGY: OPENGL", 0.04f, TEXT_COLOR);
    drawText(-0.5f, infoY-0.4f, "CONTROLS:", 0.035f, ACCENT_COLOR);
    drawText(-0.5f, infoY-0.48f, "ARROW KEYS - MOVE SNAKE", 0.03f, TEXT_COLOR);
    drawText(-0.5f, infoY-0.55f, "ESC - PAUSE/MENU", 0.03f, TEXT_COLOR);
    drawText(-0.25f, -0.6f, "PRESS ESC TO GO BACK", 0.03f, Color{TEXT_COLOR.r, TEXT_COLOR.g, TEXT_COLOR.b, 0.8f});
}

void drawGameOverScreen() {
    float overlayAlpha = gameOverAnimation;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, overlayAlpha * 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f); glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f); glVertex2f(-1.0f, 1.0f);
    glEnd();
    if (overlayAlpha > 0.5f) {
        float pulseScale = 1.0f + 0.1f * sin(animationTime * 4.0f);
        float gameOverSize = 0.08f * pulseScale;
        drawText(-0.35f, 0.3f, "GAME OVER", gameOverSize, Color{1.0f, 0.3f, 0.3f, overlayAlpha});
        char scoreText[64]; snprintf(scoreText, sizeof(scoreText), "FINAL SCORE: %d", score);
        float scoreWidth = strlen(scoreText) * 0.05f * 0.7f;
        drawText(-scoreWidth/2, 0.1f, scoreText, 0.05f, Color{TEXT_COLOR.r, TEXT_COLOR.g, TEXT_COLOR.b, overlayAlpha});
        drawText(-0.25f, -0.1f, "PRESS R TO RESTART", 0.04f, Color{ACCENT_COLOR.r, ACCENT_COLOR.g, ACCENT_COLOR.b, overlayAlpha});
        drawText(-0.2f, -0.2f, "PRESS ESC FOR MENU", 0.04f, Color{TEXT_COLOR.r, TEXT_COLOR.g, TEXT_COLOR.b, overlayAlpha*0.8f});
    }
    glDisable(GL_BLEND);
}

void drawPauseScreen() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f); glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f); glVertex2f(-1.0f, 1.0f);
    glEnd();
    drawText(-0.15f, 0.1f, "PAUSED", 0.08f, ACCENT_COLOR);
    drawText(-0.23f, -0.1f, "PRESS ESC TO RESUME", 0.035f, TEXT_COLOR);
    glDisable(GL_BLEND);
}

void drawGame() {
    // Top UI panel
    drawRoundedRect(-1.0f, 1.0f-TOP_UI_HEIGHT_NDC, 2.0f, TOP_UI_HEIGHT_NDC, 0.02f, Color{UI_COLOR.r, UI_COLOR.g, UI_COLOR.b, 0.9f});
    float topPanelCenterY = 1.0f-(TOP_UI_HEIGHT_NDC/2.0f), textLineOffset = 0.02f;
    drawText(-0.95f, topPanelCenterY+textLineOffset, "SCORE", 0.03f, ACCENT_COLOR);
    char buf[32]; snprintf(buf, sizeof(buf), "%d", score);
    drawText(-0.95f, topPanelCenterY-textLineOffset, buf, 0.04f, TEXT_COLOR);
    const char* diffHeading = "DIFFICULTY";
    float diffHeadingWidth = strlen(diffHeading)*0.03f*0.7f;
    drawText(-diffHeadingWidth/2, topPanelCenterY+textLineOffset, diffHeading, 0.03f, ACCENT_COLOR);
    const char* diffValueText = (difficulty==EASY)?"EASY":((difficulty==MEDIUM)?"MEDIUM":"HARD");
    float diffValueTextWidth = strlen(diffValueText)*0.04f*0.7f;
    drawText(-diffValueTextWidth/2, topPanelCenterY-textLineOffset, diffValueText, 0.04f, TEXT_COLOR);
    float studentTextSize=0.025f, studentLineHeight=studentTextSize*1.2f, studentInfoBlockTopY=topPanelCenterY+(studentLineHeight/2.0f);
    float nameTextWidth = strlen(STUDENT_NAME)*studentTextSize*0.7f;
    drawText(0.95f-nameTextWidth, studentInfoBlockTopY-(studentTextSize*0.5f), STUDENT_NAME, studentTextSize, TEXT_COLOR);
    char idText[32]; snprintf(idText, sizeof(idText), "ID: %s", STUDENT_ID);
    float idTextWidth = strlen(idText)*studentTextSize*0.7f;
    drawText(0.95f-idTextWidth, studentInfoBlockTopY-studentLineHeight-(studentTextSize*0.5f), idText, studentTextSize, TEXT_COLOR);

    // Bottom UI panel
    drawRoundedRect(-1.0f, -1.0f, 2.0f, BOTTOM_UI_HEIGHT_NDC, 0.02f, Color{UI_COLOR.r, UI_COLOR.g, UI_COLOR.b, 0.9f});
    float bottomPanelCenterY = -1.0f+(BOTTOM_UI_HEIGHT_NDC/2.0f);
    drawText(-0.95f, bottomPanelCenterY+textLineOffset, "CONTROLS: ARROWS - MOVE", 0.03f, ACCENT_COLOR);
    drawText(-0.95f, bottomPanelCenterY-textLineOffset, "ESC - PAUSE/MENU", 0.03f, ACCENT_COLOR);
    const char* lengthHeading = "LENGTH";
    float lengthHeadingWidth = strlen(lengthHeading)*0.03f*0.7f;
    drawText(0.95f-lengthHeadingWidth, bottomPanelCenterY+textLineOffset, lengthHeading, 0.03f, ACCENT_COLOR);
    snprintf(buf, sizeof(buf), "%d", snakeLen);
    float lengthValueTextWidth = strlen(buf)*0.04f*0.7f;
    drawText(0.95f-lengthValueTextWidth, bottomPanelCenterY-textLineOffset, buf, 0.04f, TEXT_COLOR);

    // Game area border
    float borderThicknessNDC_X = (float)2/WIDTH*2.0f, borderThicknessNDC_Y=(float)2/HEIGHT*2.0f;
    drawRoundedRect(GAME_AREA_LEFT_NDC-borderThicknessNDC_X, GAME_AREA_BOTTOM_NDC-borderThicknessNDC_Y,
        (GAME_AREA_RIGHT_NDC-GAME_AREA_LEFT_NDC)+2*borderThicknessNDC_X, (GAME_AREA_TOP_NDC-GAME_AREA_BOTTOM_NDC)+2*borderThicknessNDC_Y,
        0.03f, Color{GAME_BORDER_COLOR.r, GAME_BORDER_COLOR.g, GAME_BORDER_COLOR.b, 0.5f});

    // Food
    float gameAreaWidthNDC = GAME_AREA_RIGHT_NDC-GAME_AREA_LEFT_NDC, gameAreaHeightNDC = GAME_AREA_TOP_NDC-GAME_AREA_BOTTOM_NDC;
    float cellWidthNDC = gameAreaWidthNDC/gridWidth, cellHeightNDC = gameAreaHeightNDC/gridHeight;
    float foodX = GAME_AREA_LEFT_NDC+(food.x+0.5f)*cellWidthNDC, foodY = GAME_AREA_BOTTOM_NDC+(food.y+0.5f)*cellHeightNDC;
    float foodHalfSize = cellWidthNDC*0.45f;
    glColor4f(FOOD_COLOR.r, FOOD_COLOR.g, FOOD_COLOR.b, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(foodX-foodHalfSize*1.2f, foodY-foodHalfSize*1.2f);
    glVertex2f(foodX+foodHalfSize*1.2f, foodY-foodHalfSize*1.2f);
    glVertex2f(foodX+foodHalfSize*1.2f, foodY+foodHalfSize*1.2f);
    glVertex2f(foodX-foodHalfSize*1.2f, foodY+foodHalfSize*1.2f);
    glEnd();
    glColor4f(FOOD_COLOR.r, FOOD_COLOR.g, FOOD_COLOR.b, FOOD_COLOR.a);
    glBegin(GL_QUADS);
    glVertex2f(foodX-foodHalfSize, foodY-foodHalfSize);
    glVertex2f(foodX+foodHalfSize, foodY-foodHalfSize);
    glVertex2f(foodX+foodHalfSize, foodY+foodHalfSize);
    glVertex2f(foodX-foodHalfSize, foodY+foodHalfSize);
    glEnd();

    // Snake
    for (int i=0;i<snakeLen;i++) {
        float snakeX = GAME_AREA_LEFT_NDC+(snake[i].x+0.5f)*cellWidthNDC, snakeY = GAME_AREA_BOTTOM_NDC+(snake[i].y+0.5f)*cellHeightNDC;
        float snakeRadius = cellWidthNDC*0.48f;
        Color segmentColor = (i==0) ? SNAKE_HEAD_COLOR : SNAKE_BODY_COLOR;
        if (i==0) drawCircle(snakeX, snakeY, snakeRadius*1.2f, Color{segmentColor.r, segmentColor.g, segmentColor.b, 0.4f});
        drawCircle(snakeX, snakeY, snakeRadius, segmentColor);
    }
}

// --- SNAKE GAME LOGIC ---
void updateSnake() {
    for (int i = snakeLen-1; i > 0; --i) snake[i] = snake[i-1];
    switch (dir) {
        case UP:    snake[0].y += 1; break;
        case DOWN:  snake[0].y -= 1; break;
        case LEFT:  snake[0].x -= 1; break;
        case RIGHT: snake[0].x += 1; break;
    }
    if (snake[0].x < 0) snake[0].x = gridWidth - 1;
    else if (snake[0].x >= gridWidth) snake[0].x = 0;
    if (snake[0].y < 0) snake[0].y = gridHeight - 1;
    else if (snake[0].y >= gridHeight) snake[0].y = 0;
    for (int i = 1; i < snakeLen; ++i)
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
            gameState = GAME_OVER, gameOverAnimation = 0.0f;
    if (snake[0].x == food.x && snake[0].y == food.y) {
        if (snakeLen < MAX_SNAKE_LENGTH) {
            snake[snakeLen] = snake[snakeLen-1];
            ++snakeLen;
        }
        score += 10;
        placeFood();
    }
}

void resetGame() {
    snakeLen = 1;
    snake[0].x = gridWidth/2;
    snake[0].y = gridHeight/2;
    dir = RIGHT;
    score = 0;
    placeFood();
}

// --- Input ---
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    switch (gameState) {
        case MENU:
            if (key == GLFW_KEY_UP) selectedMenuItem = (selectedMenuItem + 2) % 3;
            else if (key == GLFW_KEY_DOWN) selectedMenuItem = (selectedMenuItem + 1) % 3;
            else if (key == GLFW_KEY_ENTER) {
                if (selectedMenuItem==0) gameState = DIFFICULTY_SELECT;
                else if (selectedMenuItem==1) gameState = ABOUT;
                else if (selectedMenuItem==2) glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            break;
        case DIFFICULTY_SELECT:
            if (key == GLFW_KEY_UP) selectedDifficulty = (selectedDifficulty + 2) % 3;
            else if (key == GLFW_KEY_DOWN) selectedDifficulty = (selectedDifficulty + 1) % 3;
            else if (key == GLFW_KEY_ENTER) {
                difficulty = (Difficulty)selectedDifficulty;
                resetGame();
                gameState = PLAYING;
            } else if (key == GLFW_KEY_ESCAPE) gameState = MENU;
            break;
        case ABOUT:
            if (key == GLFW_KEY_ESCAPE) gameState = MENU;
            break;
        case PLAYING:
            if (key == GLFW_KEY_UP && dir != DOWN) dir = UP;
            else if (key == GLFW_KEY_DOWN && dir != UP) dir = DOWN;
            else if (key == GLFW_KEY_LEFT && dir != RIGHT) dir = LEFT;
            else if (key == GLFW_KEY_RIGHT && dir != LEFT) dir = RIGHT;
            else if (key == GLFW_KEY_ESCAPE) gameState = PAUSED;
            break;
        case PAUSED:
            if (key == GLFW_KEY_ESCAPE) gameState = PLAYING;
            break;
        case GAME_OVER:
            if (key == GLFW_KEY_R) { resetGame(); gameState = PLAYING; }
            else if (key == GLFW_KEY_ESCAPE) gameState = MENU;
            break;
    }
}

void draw() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawGradientBackground();
    switch (gameState) {
        case MENU: drawMenu(); break;
        case DIFFICULTY_SELECT: drawDifficultySelect(); break;
        case ABOUT: drawAbout(); break;
        case PLAYING: drawGame(); break;
        case PAUSED: drawGame(); drawPauseScreen(); break;
        case GAME_OVER: drawGame(); drawGameOverScreen(); break;
    }
    glDisable(GL_BLEND);
}

// --- Main ---
int main() {
    srand((unsigned int)time(NULL));
    if (!glfwInit()) { std::cerr << "Failed to initialize GLFW\n"; return -1; }
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Snake Game Toha(240113)", NULL, NULL);
    if (!window) { std::cerr << "Failed to create GLFW window\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n"; return -1;
    }
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    resetGame();
    double lastUpdateTime = glfwGetTime();
    double lastAnimationTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastUpdateTime;
        double animationDeltaTime = currentTime - lastAnimationTime;
        glClear(GL_COLOR_BUFFER_BIT);

        animationTime = currentTime;
        if (gameState == PLAYING && deltaTime >= getUpdateInterval()) {
            updateSnake();
            lastUpdateTime = currentTime;
        }
        if (gameState == GAME_OVER && gameOverAnimation < 1.0f) {
            gameOverAnimation += 0.5f * animationDeltaTime;
            if (gameOverAnimation > 1.0f) gameOverAnimation = 1.0f;
        } else if (gameState != GAME_OVER) gameOverAnimation = 0.0f;
        lastAnimationTime = currentTime;

        draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}