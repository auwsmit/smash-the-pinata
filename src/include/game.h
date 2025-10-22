// EXPLANATION:
// All the game logic, including how/when to draw to screen

#ifndef SMASHTHEPINATA_GAME_HEADER_GUARD
#define SMASHTHEPINATA_GAME_HEADER_GUARD

#include "raylib.h"

// Macros
// ----------------------------------------------------------------------------

// Types and Structures
// ----------------------------------------------------------------------------

typedef enum ScreenState {
    SCREEN_LOGO, SCREEN_GAMEPLAY
} ScreenState;

typedef struct EntityPinata {
    Texture sprite;
    Rectangle rect;
    Vector2 startPos;
    float scale;
    float angle;
    float spinRate;
    bool smashed;
} EntityPinata;

typedef struct EntityHand {
    Texture sprite;
    Sound whoosh;
    Sound slap;
    Vector2 position;
    Vector2 velocity;
    Vector2 startPos;
    Vector2 offset;
    float radius;
    float angle;
    float startAngle;
    float grabAngle;
    bool grabbed;
} EntityHand;

// Game state, used across project
extern Camera2D camera;
extern ScreenState currentScreen;
extern float frameTime;
extern bool gameShouldExit;

// Prototypes
// ----------------------------------------------------------------------------

// Initialization
void InitGameState(ScreenState screen); // Initialize game data and allocate memory for sounds
void FreeGameState(void); // Free any allocated memory within game state

// Update
void UpdateGameFrame(void); // Updates all the game's data and objects for the current frame
void ResetPinata(void);

// Collision
bool CheckCollisionPointRecRotated(Vector2 point, Rectangle rect, Vector2 origin, float angle);
bool CheckCollisionCircleRecRotated(Vector2 center, float radius, Rectangle rect, Vector2 origin, float angle);

// Draw
void DrawGameFrame(void); // Draws all the game's objects for the current frame
void DrawSpriteRectangle(Texture *sprite, Rectangle rect, float angle);
void DrawSpriteCircle(Texture *sprite, Vector2 center, float radius, float angle);

// Misc

#endif // SMASHTHEPINATA_GAME_HEADER_GUARD
