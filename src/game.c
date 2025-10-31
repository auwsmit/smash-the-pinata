// EXPLANATION:
// All the game logic, including how/when to draw to screen

#include "game.h"
#include "raymath.h"
#include "config.h"

// Game globals
GameMode currentMode           = { 0 };
EntityPinata pinata            = { 0 };
EntityHand hand                = { 0 };
EntityBat bat                  = { 0 };
Candy candy[CANDY_AMOUNT] = { 0 };
Texture candyTexture[8];
Font textFont;
Music musicBackground;
Music musicWin;
Sound soundWhoosh;
Vector2 mousePos;
float timer;
float score;
float speed;
float maxSpeed;
bool showHint;

// Initialization
// ----------------------------------------------------------------------------

void InitGameState(void)
{
    currentScreen = SCREEN_LOGO;
    currentMode   = MODE_BAT;
    camera.target = (Vector2){ VIRTUAL_WIDTH/2, VIRTUAL_HEIGHT/2 };

    // Load Assets
    textFont = LoadFontEx("assets/TheVisitor.ttf", 100, 0, 0);
    SetTextureFilter(textFont.texture, TEXTURE_FILTER_BILINEAR);
    musicBackground   = LoadMusicStream("assets/music_background.wav");
    musicWin          = LoadMusicStream("assets/music_highscore.wav");
    soundWhoosh       = LoadSound("assets/whoosh.wav");
    pinata.sprite     = LoadFilteredTexture("assets/pinata.png");
    pinata.soundHit   = LoadSound("assets/hit.wav");
    bat.sprite        = LoadFilteredTexture("assets/bat.png");
    bat.soundHit      = LoadSound("assets/bonk.wav");
    hand.spriteOpen   = LoadFilteredTexture("assets/hand_open.png");
    hand.spriteClosed = LoadFilteredTexture("assets/hand_closed.png");
    for (unsigned int i = 0; i < 8; i++)
        candyTexture[i] = LoadFilteredTexture((char *)TextFormat("assets/candy%i.png", i + 1));

    // Pinata
    pinata.rect.height = 800;
    pinata.rect.width  = pinata.rect.height*((float)pinata.sprite.width/pinata.sprite.height);
    pinata.rect.x      = pinata.rect.width;
    pinata.rect.y      = pinata.rect.height*(2.0f/3.0f);
    pinata.startPos    = (Vector2){ pinata.rect.x, pinata.rect.y };
    pinata.origin      = (Vector2){ pinata.rect.width/2.0f, pinata.rect.height/2.0f };

    // Hand
    hand.startAngle = 90.0f;
    hand.angle      = hand.startAngle;
    hand.radius     = 90.0f;
    hand.position.x = VIRTUAL_WIDTH - 700.0f - hand.radius/2;
    hand.position.y = (VIRTUAL_HEIGHT - hand.radius)/2;
    if (currentMode == MODE_BAT)
        hand.position.y += 200.0f;
    hand.startPos   = hand.position;

    // Bat
    bat.rect.height = 800;
    bat.rect.width  = bat.rect.height*((float)bat.sprite.width/bat.sprite.height);
    bat.origin = (Vector2){ bat.rect.width/2.0f, bat.rect.height - bat.rect.height/6.0f };

    showHint = true;
    PlayMusicStream(musicBackground);
}

void FreeGameState(void)
{
    UnloadFont(textFont);
    UnloadMusicStream(musicBackground);
    UnloadMusicStream(musicWin);
    UnloadSound(soundWhoosh);
    UnloadTexture(pinata.sprite);
    UnloadSound(pinata.soundHit);
    UnloadTexture(bat.sprite);
    UnloadSound(bat.soundHit);
    UnloadTexture(hand.spriteOpen);
    UnloadTexture(hand.spriteClosed);
    for (unsigned int i = 0; i < 8; i++)
        UnloadTexture(candyTexture[i]);
}

Texture LoadFilteredTexture(char* path)
{
    Texture tex = LoadTexture(path);
    SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
    return tex;
}

// Update
// ----------------------------------------------------------------------------

void UpdateGameFrame(void)
{
    UpdateMusicStream(musicBackground);
    UpdateMusicStream(musicWin);
    if (!IsSoundPlaying(soundWhoosh))
        PlaySound(soundWhoosh);

    timer -= frameTime;
    mousePos = GetScreenToWorld2D(GetMousePosition(), camera);

    // Mode swap
    // ----------------------------------------------------------------------------
    if (IsKeyPressed(KEY_SPACE))
    {
        if (currentMode == MODE_HAND)
        {
            currentMode = MODE_BAT;
            hand.startPos.y += 200.0f;
            hand.angle = (hand.position.x - pinata.startPos.x)*0.1f + 30.0f;
        }
        else
        {
            currentMode = MODE_HAND;
            hand.startPos.y -= 200.0f;
        }
    }

    // Grab or Release Hand
    // ----------------------------------------------------------------------------
    if (!hand.grabbed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        hand.grabbed = true;
        showHint = false;
    }

    if (hand.grabbed && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        hand.grabbed = false;
        speed = 0;
        maxSpeed = 0;
    }

    if (hand.grabbed)
    {
        Vector2 prevPos = hand.position;
        Vector2 newPos = Vector2Lerp(hand.position, mousePos, 25.0f*frameTime);
        hand.velocity = Vector2Subtract(newPos, prevPos);
        speed = Vector2Length(hand.velocity)*camera.zoom/frameTime*0.01f;
#if defined(PLATFORM_WEB) // web canvas scales differently
        speed *= 1.5f;
#endif

        float newAngle;
        if (currentMode == MODE_HAND)
        {
            newAngle = atan2f(hand.velocity.y, hand.velocity.x)*RAD2DEG + 270.0f;
            newAngle = fmodf(newAngle, 360.0f);
        }
        else // if (currentMode == MODE_BAT)
        {
            newAngle = (hand.position.x - pinata.startPos.x)*0.1f + 30.0f;
        }
        float angleDelta = newAngle - hand.angle;
        if (angleDelta > 180.0f) angleDelta -= 360.0f;
        if (angleDelta < -180.0f) angleDelta += 360.0f;
        if (Vector2Length(hand.velocity)/frameTime > 0.05f) // don't rotate when nearly stopped
        {
            hand.angle = fmodf(hand.angle + Lerp(0.0f, angleDelta, 20.0f*frameTime), 360.0f);
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

    if (currentMode == MODE_BAT)
    {
        bat.rect.x = hand.position.x;
        bat.rect.y = hand.position.y;
        bat.angle = hand.angle - 90.0f;
    }

    static float whooshVolume = 0.0f;
    static float whooshPitch = 1.0f;
    float pitchMin = (currentMode == MODE_HAND)? 2.5f : 1.0f;
    float targetVolume = (speed < 15)? 0 : Remap(speed, 15, 100.0f, 0, 1.0f);
    float targetPitch = Remap(speed, 0, 200.0f, pitchMin, pitchMin*4);
    whooshVolume = Lerp(whooshVolume, targetVolume, 5.0f*frameTime);
    whooshPitch  = Lerp(whooshPitch, targetPitch, 5.0f*frameTime);
    SetSoundVolume(soundWhoosh, whooshVolume);
    SetSoundPitch(soundWhoosh, whooshPitch);

    // Hit pinata at minimum velocity
    // ----------------------------------------------------------------------------
    Vector2 origin = { pinata.rect.width/2, pinata.rect.height/2 };
    Vector2 hitPosition = hand.position;
    if (currentMode == MODE_BAT) // set hitPosition at center of bat
    {
        Vector2 hitOffset = { 0, bat.origin.y/2 };
        Vector2 batHandle = { bat.rect.x, bat.rect.y };
        hitOffset = Vector2Rotate(hitOffset, bat.angle*DEG2RAD);
        hitPosition = Vector2Subtract(batHandle, hitOffset);
    }
    if (!pinata.smashed && hand.grabbed && (speed > 50.0f) && (hand.velocity.x < 0) &&
        CheckCollisionCircleRecRotated(hitPosition, hand.radius, pinata.rect, origin, pinata.angle))
    {
        score = speed;
        pinata.smashed = true;
        pinata.spinRate = -speed*0.03f;
        pinata.xVelocity = score*0.1f;
        if (score > 200.0f)
        {
            timer = 3.0f;
            pinata.spinRate *= 1.5f;
            pinata.xVelocity *= 0.3f;
            SpawnCandy();
            PlayMusicStream(musicWin);
            if (currentMode == MODE_BAT) PlaySound(bat.soundHit);

        }
        else timer = 1.0f;

        PauseMusicStream(musicBackground);
        PlaySound(pinata.soundHit);
    }

    if (pinata.smashed)
    {
        pinata.rect.x -= pinata.xVelocity;
        pinata.angle += pinata.spinRate;
    }

    // Reset after pinata smashed
    // ----------------------------------------------------------------------------
    if (pinata.smashed && (timer < 0))
    {
        pinata.smashed = false;
        pinata.rect.x = pinata.startPos.x;
        pinata.angle = 0;
        maxSpeed = 0;
        score = 0;
        StopMusicStream(musicWin);
        PlayMusicStream(musicBackground);
    }

    // Update Candy
    // ----------------------------------------------------------------------------
    if (pinata.smashed)
    {
        for (unsigned int i = 0; i < CANDY_AMOUNT; i++)
        {
            Vector2 movement = Vector2Scale(candy[i].velocity, frameTime);
            candy[i].position = Vector2Add(candy[i].position, movement);
            candy[i].velocity.y += 1000*frameTime;
            candy[i].angle += candy[i].rotationRate*frameTime;
        }
    }
}

void SpawnCandy(void)
{
    for (unsigned int i = 0; i < CANDY_AMOUNT; i++)
    {
        candy[i].position = (Vector2){
            pinata.rect.x + GetRandomValue((int)-pinata.rect.width/8, (int)pinata.rect.width/8),
            pinata.rect.y + GetRandomValue((int)-pinata.rect.height/8, (int)pinata.rect.height/8),
        };
        candy[i].velocity.x = (float)GetRandomValue(100, 1200);
        candy[i].velocity.y = (float)GetRandomValue(-100, -1000);
        candy[i].rotationRate = GetRandomValue(-300,300);
        candy[i].textureId = GetRandomValue(0,7);
    }
}

// Collision
// ----------------------------------------------------------------------------

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

// Draw
// ----------------------------------------------------------------------------

void DrawGameFrame(void)
{
    ClearBackground(ORANGE);

    // Draw pinata
    DrawSpriteRectangle(&pinata.sprite, pinata.rect, pinata.origin, pinata.angle);

    // Draw hand
    if ((currentMode == MODE_HAND) || !hand.grabbed)
        DrawSpriteCircle(&hand.spriteOpen, hand.position, hand.radius, hand.angle);

    // Draw bat
    if (currentMode == MODE_BAT)
    {
        DrawSpriteRectangle(&bat.sprite, bat.rect, bat.origin, bat.angle);
        if (hand.grabbed)
            DrawSpriteCircle(&hand.spriteClosed, hand.position, hand.radius, hand.angle);
    }

    // Draw hint
    int fontSize = 50;
    Color fontColor = RAYWHITE;
    if (showHint)
    {
        const char *scoreText = "Click to drag";
        int textLength = (int)MeasureTextEx(textFont, scoreText, fontSize, 0).x;
        DrawTextEx(textFont, scoreText,
                   (Vector2){ hand.position.x - textLength/2,
                   hand.position.y + fontSize + 100, },
                   fontSize, 0, fontColor);
    }

    // Draw score message
    if (pinata.smashed)
    {
        Color fontColor = ColorBrightness(YELLOW,0.5);
        if (score > 400.0f)
        {
            fontColor = ColorBrightness(RED, 0.1f);
            DrawCenterText("How?!", fontColor, false);
        }
        else if (score > 200.0f)
        {
            fontColor = YELLOW;
            DrawCenterText("Holy Crap!", fontColor, false);
        }
        else DrawCenterText("Swing harder!", fontColor, false);

        DrawCenterText(TextFormat("Score: %.0f", score),
                       fontColor, true);
    }

    // Draw candy
    if (pinata.smashed && (score > 200.0f))
    {
        for (unsigned int i = 0; i < CANDY_AMOUNT; i++)
        {
            DrawSpriteCircle(&candyTexture[candy[i].textureId],
                             candy[i].position,
                             30, candy[i].angle);
        }
    }

    // // Debug
    // const int textSize = 50;
    // int textX = 50;
    // int textY = 50;
    // textY += textSize;
    // DrawText(TextFormat("screen width, height: %i %i", GetScreenWidth(), GetScreenHeight()), textX, textY, textSize, RAYWHITE);
    // textY += textSize;
    // DrawText(TextFormat("zoom: %.3f", camera.zoom), textX, textY, textSize, RAYWHITE);
    // textY += textSize;
    // DrawText(TextFormat("current speed: %.0f", speed), textX, textY, textSize, RAYWHITE);
    // textY += textSize;
    // DrawText(TextFormat("max speed: %.0f", maxSpeed), textX, textY, textSize, RAYWHITE);
    // textY += textSize;
    // DrawText(TextFormat("hand angle: %.0f", hand.angle), textX, textY, textSize, RAYWHITE);
}

void DrawSpriteRectangle(Texture *sprite, Rectangle rect, Vector2 origin, float angle)
{
    Rectangle src = { 0, 0, (float)sprite->width, (float)sprite->height };
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

    DrawTexturePro(*sprite, spriteSrc, spriteDest, spriteOrigin, angle, WHITE);
}

void DrawCenterText(const char* text, Color fontColor, bool nextLine)
{
    const int fontSize = 130;
    float offset = nextLine? fontSize : 0;
    int textLength = (int)MeasureTextEx(textFont, text, fontSize, 0).x;
    DrawTextEx(textFont, text,
               (Vector2){ (VIRTUAL_WIDTH - textLength)/2,
               (VIRTUAL_HEIGHT - fontSize)/2 - 200 + offset, },
               fontSize, 0, fontColor);
}
