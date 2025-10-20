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
    float padding;
    bool smashed;
} EntityPinata;

typedef struct EntityHand {
    Texture sprite;
    Sound whoosh;
    Sound slap;
    Vector2 position;
    Vector2 startPos;
    Vector2 offset;
    float radius;
    float angle;
    float startAngle;
    float grabAngle;
    float padding;
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

// Update & Draw
void UpdateGameFrame(void); // Updates all the game's data and objects for the current frame
void DrawGameFrame(void); // Draws all the game's objects for the current frame

void ResetPinata(void);

#endif // SMASHTHEPINATA_GAME_HEADER_GUARD
