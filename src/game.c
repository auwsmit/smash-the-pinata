// EXPLANATION:
// All the game logic, including how/when to draw to screen

#include "game.h"
#include "config.h"

// Game globals
EntityPinata pinata;
EntityHand hand;

// Initialization
// ----------------------------------------------------------------------------

void InitGameState(ScreenState screen)
{
    game.camera.target = (Vector2){ VIRTUAL_WIDTH/2, VIRTUAL_HEIGHT/2 };
    game.currentScreen = screen;
    game.debugMode = false;

    float padding = 200.0f;
    pinata.sprite = LoadTexture("assets/pinata.png");
    pinata.scale = 1.0f;
    pinata.rect.width = pinata.sprite.width*pinata.scale;
    pinata.rect.height = pinata.sprite.height*pinata.scale;
    // pinata.rect.x = (VIRTUAL_WIDTH + pinata.rect.width)/2;
    // pinata.rect.y = (VIRTUAL_HEIGHT + pinata.rect.height)/2;
    pinata.rect.x = 0 + padding;
    pinata.rect.y = (VIRTUAL_HEIGHT - pinata.rect.height)/2;

    padding = 500.0f;
    hand.sprite = LoadTexture("assets/hand.png");
    hand.radius = 100.0f;
    hand.position.x = VIRTUAL_WIDTH - padding - hand.radius/2;
    hand.position.y = (VIRTUAL_HEIGHT - hand.radius)/2;

}

void FreeGameState(void)
{
    UnloadTexture(pinata.sprite);
    UnloadTexture(hand.sprite);
}

// Update & Draw
// ----------------------------------------------------------------------------

void UpdateGameFrame(void)
{
    // TODO:
    // - grab hand
    // - slap pinata at minimum velocity
    // - reset pinata
    // - display score
    // - extras (sounds, particles, other pinatas)
    // - polish
}

void DrawGameFrame(void)
{
    ClearBackground(ORANGE);

    // Draw Pinata
    Texture *sprite = &pinata.sprite;
    float spriteScaleX = pinata.rect.width*pinata.scale/sprite->width;
    float spriteScaleY = pinata.rect.height*pinata.scale/sprite->height;
    Rectangle spriteSrc = { 0.0f, 0.0f, (float)sprite->width, (float)sprite->height };
    Rectangle spriteDest = {
        pinata.rect.x, pinata.rect.y,
        sprite->width*spriteScaleX, sprite->height*spriteScaleY
    };
    Vector2 spriteOrigin = { 0 };

    DrawTexturePro(*sprite, spriteSrc, spriteDest, spriteOrigin, pinata.angle, WHITE);

    // Draw Hand
    sprite = &hand.sprite;
    float spriteScale = hand.radius*2.0f/sprite->width;
    spriteSrc = (Rectangle){ 0.0f, 0.0f, (float)sprite->width, (float)sprite->height };
    spriteDest = (Rectangle){
        hand.position.x, hand.position.y,
        sprite->width*spriteScale, sprite->height*spriteScale
    };
    spriteOrigin = (Vector2){
        sprite->width/2*spriteScale,
        sprite->height/2*spriteScale };

    // DrawCircleV(hand.position, hand.radius, WHITE);
    DrawTexturePro(*sprite, spriteSrc, spriteDest, spriteOrigin, hand.angle, WHITE);
}

// Collision
// ----------------------------------------------------------------------------

