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

    pinata.sprite = LoadTexture("assets/pinata.png");
    SetTextureFilter(pinata.sprite, TEXTURE_FILTER_BILINEAR);
    pinata.rect.height = 800;
    pinata.rect.width = pinata.rect.height*((float)pinata.sprite.width/pinata.sprite.height);
    // pinata.rect.x = 200.0f;
    // pinata.rect.y = (VIRTUAL_HEIGHT - pinata.rect.height)/2;
    pinata.rect.x = pinata.rect.width;
    pinata.rect.y = pinata.rect.height*0.6666f;
    pinata.startPos = (Vector2){ pinata.rect.x, pinata.rect.y };

    hand.sprite = LoadTexture("assets/hand.png");
    SetTextureFilter(hand.sprite, TEXTURE_FILTER_BILINEAR);
    hand.radius = 100.0f;
    hand.position.x = VIRTUAL_WIDTH - 500.0f - hand.radius/2;
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
    timer -= frameTime;

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
        maxSpeed = 0;
    }

    if (hand.grabbed)
    {
        Vector2 prevPos = hand.position;
        // Vector2 seekPos = Vector2Subtract(grabPos, grabOffset);
        Vector2 newPos = Vector2Lerp(hand.position, grabPos, 25.0f*frameTime);
        hand.velocity = Vector2Subtract(newPos, prevPos);
        speed = Vector2Length(hand.velocity)/frameTime*camera.zoom*0.0075f;
        float newAngle = atan2f(hand.velocity.y, hand.velocity.x)*RAD2DEG + 270.0f;
        newAngle = fmodf(newAngle, 360.0f);
        if (newAngle < 0.0f) newAngle += 360.0f;
        float angleDelta = newAngle - hand.angle;
        if (angleDelta > 180.0f) angleDelta -= 360.0f;
        if (angleDelta < -180.0f) angleDelta += 360.0f;

        if (speed > 0.05f)
        {
            hand.angle = fmodf(hand.angle + Lerp(0.0f, angleDelta, 10.0f*frameTime), 360.0f);
            if (hand.angle < 0.0f) hand.angle += 360.0f;
        }
        if (!pinata.smashed && (speed > maxSpeed))
            maxSpeed = speed;

        hand.position = newPos;
    }
    else // hand is released
    {
        hand.position = Vector2Lerp(hand.position, hand.startPos, 5.0f*frameTime);

        // shortest angle difference
        float angleDelta = hand.startAngle - hand.angle;
        if (angleDelta > 180.0f) angleDelta -= 360.0f;
        if (angleDelta < -180.0f) angleDelta += 360.0f;
        hand.angle = fmodf(hand.angle + angleDelta*0.05f, 360.0f);
        if (hand.angle < 0.0f) hand.angle += 360.0f;
    }

    // Hit pinata at minimum velocity
    // ----------------------------------------------------------------------------
    Vector2 origin = { pinata.rect.width/2, pinata.rect.height/2 };
    if (!pinata.smashed && hand.grabbed && (speed > 50.0f) && (hand.velocity.x < 0) &&
        CheckCollisionCircleRecRotated(hand.position, hand.radius, pinata.rect, origin, pinata.angle))
    {
        pinata.smashed = true;
        pinata.spinRate = -speed*0.01f;
        score = speed;
        if (score > 200.0f)
            timer = 3.0f;
        else
            timer = 1.0f;
    }
    if (pinata.smashed)
    {
        pinata.rect.x -= score*frameTime*8.0f;
        pinata.angle += pinata.spinRate;
    }

    // Reset after pinata smashed
    // ----------------------------------------------------------------------------
    if (pinata.smashed && (timer < 0)) ResetPinata();

    // Debug
    // ----------------------------------------------------------------------------
    if (IsKeyPressed(KEY_R))
    {
        ResetPinata();
    }
}

void DrawGameFrame(void)
{
    ClearBackground(ORANGE);

    // Draw Pinata
    DrawSpriteRectangle(&pinata.sprite, pinata.rect, pinata.angle);

    // Draw Hand
    DrawSpriteCircle(&hand.sprite, hand.position, hand.radius, hand.angle);

    // Draw score
    if (pinata.smashed)
    {
        int fontSize = 180;
        Color fontColor = ColorBrightness(YELLOW,0.5);
        if (score > 200.0f)
        {
            const char *text = "Holy Crap!";
            fontColor = YELLOW;
            int textLength = MeasureText(text, fontSize);
            DrawText(text,
                     (VIRTUAL_WIDTH - textLength)/2,
                     (VIRTUAL_HEIGHT - fontSize)/2 - fontSize,
                     fontSize, fontColor);
        }
        const char *scoreText = TextFormat("Score: %.0f", score);
        int textLength = MeasureText(scoreText, fontSize);
        DrawText(scoreText,
                 (VIRTUAL_WIDTH - textLength)/2,
                 (VIRTUAL_HEIGHT - fontSize)/2,
                 fontSize, fontColor);
    }

    // Debug
    // const int textSize = 50;
    // int textX = 50;
    // int textY = 50;
    // DrawText(TextFormat("zoom: %.3f", camera.zoom), textX, textY, textSize, RAYWHITE);
    // textY += textSize;
    // DrawText(TextFormat("current speed: %.0f", speed), textX, textY, textSize, RAYWHITE);
    // textY += textSize;
    // DrawText(TextFormat("max speed: %.0f", maxSpeed), textX, textY, textSize, RAYWHITE);
    // textY += textSize;
    // DrawText(TextFormat("hand angle: %.0f", hand.angle), textX, textY, textSize, RAYWHITE);
}

void DrawSpriteRectangle(Texture *sprite, Rectangle rect, float angle)
{
    Rectangle src = { 0, 0, (float)sprite->width, (float)sprite->height };
    Vector2 origin = { rect.width/2.0f, rect.height/2.0f };

    // DrawRectanglePro(rect, origin, angle, WHITE);
    DrawTexturePro(*sprite, src, rect, origin, angle, WHITE);
}

void DrawSpriteCircle(Texture *sprite, Vector2 center, float radius, float angle)
{
    float spriteScale = radius*2.0f/sprite->width;
    Rectangle spriteSrc = { 0.0f, 0.0f, (float)sprite->width, (float)sprite->height };
    Rectangle spriteDest = {
        center.x, center.y,
        sprite->width*spriteScale, sprite->height*spriteScale
    };
    Vector2 spriteOrigin = {
        sprite->width/2*spriteScale,
        sprite->height/2*spriteScale };

    // DrawCircleV(hand.position, hand.radius, WHITE);
    DrawTexturePro(*sprite, spriteSrc, spriteDest, spriteOrigin, angle, WHITE);
}

bool CheckCollisionPointRecRotated(Vector2 point, Rectangle rect, Vector2 origin, float angle)
{
    Rectangle localRect = { 0, 0, rect.width, rect.height };
    localRect.x -= origin.x*2;
    localRect.y -= origin.y*2;
    Vector2 rotVector = Vector2Subtract(point, (Vector2){ rect.x, rect.y });
    rotVector = Vector2Rotate(rotVector, -angle*DEG2RAD);
    Vector2 localPoint = Vector2Subtract(rotVector, (Vector2){ origin.x, origin.y });
    return CheckCollisionPointRec(localPoint, localRect);
}

bool CheckCollisionCircleRecRotated(Vector2 center, float radius, Rectangle rect, Vector2 origin, float angle)
{
    Rectangle localRect = { 0, 0, rect.width, rect.height };
    localRect.x -= origin.x*2;
    localRect.y -= origin.y*2;
    Vector2 rotVector = Vector2Subtract(center, (Vector2){ rect.x, rect.y });
    rotVector = Vector2Rotate(rotVector, -angle*DEG2RAD);
    Vector2 localCenter = Vector2Subtract(rotVector, (Vector2){ origin.x, origin.y });
    return CheckCollisionCircleRec(localCenter, radius, localRect);
}

void ResetPinata(void)
{
    pinata.smashed = false;
    pinata.rect.x = pinata.startPos.x;
    pinata.angle = 0;
    maxSpeed = 0;
    score = 0;
}
