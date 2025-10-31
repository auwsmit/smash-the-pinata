// EXPLANATION:
// All the game logic, including how/when to draw to screen

#ifndef SMASHTHEPINATA_GAME_HEADER_GUARD
#define SMASHTHEPINATA_GAME_HEADER_GUARD

#include "raylib.h"

// Macros
// ----------------------------------------------------------------------------
#define CANDY_AMOUNT 50

// Types and Structures
// ----------------------------------------------------------------------------

typedef enum { SCREEN_LOGO, SCREEN_GAMEPLAY } ScreenState;

typedef enum { MODE_BAT, MODE_HAND } GameMode;

typedef struct {
    Texture sprite;
    Sound soundHit;
    Rectangle rect;
    Vector2 startPos;
    Vector2 origin;
    float scale;
    float angle;
    float spinRate;
    float xVelocity;
    bool smashed;
} EntityPinata;

typedef struct {
    Texture sprite;
    Sound soundHit;
    Rectangle rect;
    Vector2 origin;
    float angle;
    float startAngle;
} EntityBat;

typedef struct {
    Texture spriteOpen;
    Texture spriteClosed;
    Vector2 position;
    Vector2 velocity;
    Vector2 startPos;
    float radius;
    float angle;
    float startAngle;
    bool grabbed;
} EntityHand;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    int textureId;
    Color color;
    float angle;
    float rotationRate;
} Candy;

// Game state, used across project
extern Camera2D camera;
extern ScreenState currentScreen;
extern float frameTime;
extern bool gameShouldExit;

// Prototypes
// ----------------------------------------------------------------------------

// Initialization
void InitGameState(void); // Initialize game data and allocate memory for sounds
void FreeGameState(void); // Free any allocated memory within game state
Texture LoadFilteredTexture(char* path);

// Update
void UpdateGameFrame(void); // Updates all the game's data and objects for the current frame
void SpawnCandy(void);

// Collision (for rotated rectangles)
bool CheckCollisionPointRecRotated(Vector2 point, Rectangle rect, Vector2 origin, float angle);
bool CheckCollisionCircleRecRotated(Vector2 center, float radius, Rectangle rect, Vector2 origin, float angle);

// Draw
void DrawGameFrame(void); // Draws all the game's objects for the current frame
void DrawSpriteRectangle(Texture *sprite, Rectangle rect, Vector2 origin, float angle);
void DrawSpriteCircle(Texture *sprite, Vector2 center, float radius, float angle);
void DrawCenterText(const char* text, Color fontColor, bool nextLine);

// Misc

#endif // SMASHTHEPINATA_GAME_HEADER_GUARD
