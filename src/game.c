// EXPLANATION:
// All the game logic, including how/when to draw to screen

#include "game.h"
#include "raymath.h"
#include "config.h"

// Game globals scoped within this file
EntityPinata pinata;
EntityHand hand;
Vector2 prevMousePos;
Vector2 mousePos;
Vector2 mouseDelta;
float timer;
float score;
float speed;
float maxSpeed;

// Initialization
// ----------------------------------------------------------------------------

void InitGameState(ScreenState screen)
{
    currentScreen = screen;
    camera.target = (Vector2){ VIRTUAL_WIDTH/2, VIRTUAL_HEIGHT/2 };

    pinata.padding = 200.0f;
    pinata.sprite = LoadTexture("assets/pinata.png");
    pinata.rect.height = 800;
    pinata.rect.width = pinata.rect.height*((float)pinata.sprite.width/pinata.sprite.height);
    pinata.rect.x = 0 + pinata.padding;
    pinata.rect.y = (VIRTUAL_HEIGHT - pinata.rect.height)/2;

    hand.padding = 500.0f;
    hand.sprite = LoadTexture("assets/hand.png");
    hand.radius = 100.0f;
    hand.position.x = VIRTUAL_WIDTH - hand.padding - hand.radius/2;
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
    timer += frameTime;
    prevMousePos  = mousePos;
    mousePos   = GetScreenToWorld2D(GetMousePosition(), camera);
    mouseDelta = Vector2Subtract(mousePos, prevMousePos);

    // Grab hand
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        CheckCollisionPointCircle(mousePos, hand.position, hand.radius))
    {
        hand.grabbed = true;
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        hand.grabbed = false;

    if (hand.grabbed)
    {
        speed = Vector2Length(mouseDelta);
        if (speed > maxSpeed) maxSpeed = speed;
        hand.position = Vector2Add(hand.position, mouseDelta);
    }
    else maxSpeed = 0;

    // Slap pinata at minimum velocity
    if (!pinata.slapped && hand.grabbed && (speed > 100) &&
        CheckCollisionCircleRec(hand.position, hand.radius, pinata.rect))
    {
        pinata.slapped = true;
        pinata.rect.x -= speed*1.5f;
        score = speed;
        maxSpeed = 0;
        timer = 0;
    }

    // Reset after pinata slapped
    if (pinata.slapped && (timer >= 1.0f)) ResetGame();

    // TODO:
    // - hand and pinata physics/inertia/animation
    // - extras (sounds, particles, other pinatas)
    // - polish

    // Debug
    if (IsKeyPressed(KEY_R))
    {
        ResetGame();
    }
}

void ResetGame(void)
{
    hand.grabbed = false;
    hand.position.x = VIRTUAL_WIDTH - hand.padding - hand.radius/2;
    hand.position.y = (VIRTUAL_HEIGHT - hand.radius)/2;
    pinata.slapped = false;
    pinata.rect.x = 0 + pinata.padding;
    score = 0;
}

void DrawGameFrame(void)
{
    ClearBackground(ORANGE);

    // Draw Pinata
    Texture *sprite = &pinata.sprite;
    float spriteScaleX = pinata.rect.width/sprite->width;
    float spriteScaleY = pinata.rect.height/sprite->height;
    Rectangle spriteSrc = { 0.0f, 0.0f, (float)sprite->width, (float)sprite->height };
    Rectangle spriteDest = {
        pinata.rect.x, pinata.rect.y,
        sprite->width*spriteScaleX, sprite->height*spriteScaleY
    };
    Vector2 spriteOrigin = { 0 };

    // DrawRectangleRec(pinata.rect, WHITE);
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

    // Draw score
    if (pinata.slapped)
    {
        const char *scoreText = TextFormat("Score: %.0f", score);
        int fontSize = 180;
        int textLength = MeasureText(scoreText, fontSize);
        DrawText(scoreText,
                 (VIRTUAL_WIDTH - textLength)/2,
                 (VIRTUAL_HEIGHT - fontSize)/2,
                 fontSize, RAYWHITE);
    }

    // Debug
    // const int textSize = 50;
    // int textX = 50;
    // int textY = 50;
    // DrawText(TextFormat("current speed: %.0f", speed), 0, textY, textSize, RAYWHITE);
    // textY += textSize;
    // DrawText(TextFormat("max speed: %.0f", maxSpeed), textX, textY, textSize, RAYWHITE);
}
