// EXPLANATION:
// All the game logic, including how/when to draw to screen

#include "game.h"
#include "raymath.h"
#include "config.h"

// Game globals
EntityPinata pinata;
EntityHand hand;
Vector2 grabPos;
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
    SetTextureFilter(pinata.sprite, TEXTURE_FILTER_BILINEAR);
    pinata.rect.height = 800;
    pinata.rect.width = pinata.rect.height*((float)pinata.sprite.width/pinata.sprite.height);
    pinata.rect.x = 0 + pinata.padding;
    pinata.rect.y = (VIRTUAL_HEIGHT - pinata.rect.height)/2;
    pinata.startPos = (Vector2){ pinata.rect.x, pinata.rect.y };

    hand.padding = 500.0f;
    hand.sprite = LoadTexture("assets/hand.png");
    SetTextureFilter(hand.sprite, TEXTURE_FILTER_BILINEAR);
    hand.radius = 100.0f;
    hand.position.x = VIRTUAL_WIDTH - hand.padding - hand.radius/2;
    hand.position.y = (VIRTUAL_HEIGHT - hand.radius)/2;
    hand.startPos = hand.position;
    hand.startAngle = 90.0f;
    hand.angle = hand.startAngle;
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

    grabPos = GetScreenToWorld2D(GetMousePosition(), camera);

    // Grab hand
    // ----------------------------------------------------------------------------
    if (!hand.grabbed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        CheckCollisionPointCircle(grabPos, hand.position, hand.radius))
    {
        hand.grabbed = true;
        // grabOffset = Vector2Subtract(grabPos, hand.position);
        // grabAngle = hand.angle;
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        hand.grabbed = false;
    }

    if (hand.grabbed)
    {
        Vector2 prevPos = hand.position;
        // Vector2 seekPos = Vector2Subtract(grabPos, grabOffset);
        Vector2 newPos = Vector2Lerp(hand.position, grabPos, 20.0f*frameTime);
        Vector2 handVelocity = Vector2Subtract(newPos, prevPos);
        speed = Vector2Length(handVelocity)/frameTime*0.01f;
        float newAngle = atan2f(handVelocity.y, handVelocity.x)*RAD2DEG + 270.0f;
        float angleDelta = fmodf(newAngle - hand.angle + 540.0f, 360.0f) - 180.0f;

        if (speed > 0.05f)
            hand.angle = fmodf(Lerp(hand.angle, hand.angle + angleDelta, 10.0f*frameTime), 360.0f);
        if (!pinata.smashed && (speed > maxSpeed))
            maxSpeed = speed;

        hand.position = newPos;
    }
    else // !hand.grabbed
    {
        hand.position = Vector2Lerp(hand.position, hand.startPos, 5.0f*frameTime);

        // shortest angle difference
        float angleDelta = fmodf(hand.startAngle - hand.angle + 540.0f, 360.0f) - 180.0f;
        hand.angle = fmodf(hand.angle + angleDelta*0.05f, 360.0f);
    }

    // Hit pinata at minimum velocity
    // ----------------------------------------------------------------------------
    if (!pinata.smashed && hand.grabbed && (speed > 200.0f) &&
        CheckCollisionCircleRec(hand.position, hand.radius, pinata.rect))
    {
        pinata.smashed = true;
        pinata.rect.x -= speed*0.75f;
        score = speed;
        timer = 0;
    }

    // Reset after pinata smashed
    // ----------------------------------------------------------------------------
    if (pinata.smashed && (timer >= 1.0f)) ResetPinata();

    // Debug
    // ----------------------------------------------------------------------------
    if (IsKeyPressed(KEY_R))
    {
        ResetPinata();
    }
}

void ResetPinata(void)
{
    pinata.smashed = false;
    pinata.rect.x = 0 + pinata.padding;
    maxSpeed = 0;
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
    if (pinata.smashed)
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
    // DrawText(TextFormat("current speed: %.0f", speed), textX, textY, textSize, RAYWHITE);
    // textY += textSize;
    // DrawText(TextFormat("max speed: %.0f", maxSpeed), textX, textY, textSize, RAYWHITE);
    // textY += textSize;
    // DrawText(TextFormat("hand angle: %.0f", hand.angle), textX, textY, textSize, RAYWHITE);
}
