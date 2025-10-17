// EXPLANATION:
// All the game logic, including how/when to draw to screen

#ifndef SLAPMASTER_GAME_HEADER_GUARD
#define SLAPMASTER_GAME_HEADER_GUARD

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
    float scale;
    float angle;
} EntityPinata;

typedef struct EntityHand {
    Texture sprite;
    Sound whoosh;
    Sound slap;
    Vector2 position;
    Vector2 velocity;
    float radius;
    float angle;
    bool grabbed;
} EntityHand;

typedef struct GameState {
    Camera2D camera;
    ScreenState currentScreen;
    float frameTime;
    bool gameShouldExit;
    bool debugMode;
} GameState;

extern GameState game; // global declaration

// Prototypes
// ----------------------------------------------------------------------------

// Initialization
void InitGameState(ScreenState screen); // Initialize game data and allocate memory for sounds
void FreeGameState(void); // Free any allocated memory within game state

// Update & Draw
void UpdateGameFrame(void); // Updates all the game's data and objects for the current frame
void DrawGameFrame(void); // Draws all the game's objects for the current frame

#endif // SLAPMASTER_GAME_HEADER_GUARD
