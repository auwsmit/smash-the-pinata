// EXPLANATION:
// For the raylib logo animation at start of program
// See logo.h for more documentation/descriptions

#include "logo.h"
#include "config.h"
#include "game.h"

// Global animation state
LogoAnimation logo = { 0 };

void InitRaylibLogo(void)
{
    LogoAnimation startOfAnimation = {
        .positionX = VIRTUAL_WIDTH/2 - RAYLIB_LOGO_WIDTH/2,
        .positionY = VIRTUAL_HEIGHT/2 - RAYLIB_LOGO_WIDTH/2,

        .elapsedTime = 0,
        .lettersCount = 0,

        .topSideRecWidth = RAYLIB_LOGO_OUTLINE,
        .leftSideRecHeight = RAYLIB_LOGO_OUTLINE,
        .bottomSideRecWidth = RAYLIB_LOGO_OUTLINE,
        .rightSideRecHeight = RAYLIB_LOGO_OUTLINE,

        .state = LOGO_START, // State machine
        .alpha = 0.0f, // Useful for fading
    };

    logo = startOfAnimation;
}

void UpdateRaylibLogo(void)
{
    const float growSpeed = RAYLIB_LOGO_WIDTH*0.9375f; // Speed that lines grow
    const float letterDelay = 0.2f; // Time between each letter appearing
    const float fadeSpeed = 1.0f; // Fade out in 1 second
    const float epsilon = 0.0001f;

    // Press key or click or touch to skip
    if ((!IsKeyDown(KEY_LEFT_ALT) && !IsKeyDown(KEY_RIGHT_ALT) &&
         !IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_RIGHT_SHIFT) &&
         !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_RIGHT_CONTROL)) &&
        ((GetKeyPressed() != KEY_NULL) || IsGestureDetected(GESTURE_TAP)))
    {
        if ((logo.lettersCount < 6) && (logo.alpha < epsilon))
        {
            logo.skipped = true;
            logo.topSideRecWidth = RAYLIB_LOGO_WIDTH;
            logo.leftSideRecHeight = RAYLIB_LOGO_WIDTH;
            logo.bottomSideRecWidth = RAYLIB_LOGO_WIDTH;
            logo.rightSideRecHeight = RAYLIB_LOGO_WIDTH;
            logo.lettersCount = 10;
            logo.elapsedTime = 0;
            logo.state = LOGO_TEXT;
        }
        else
        {
            logo.state = LOGO_END;
            logo.elapsedTime = 1.0f;
        }
    }

    // Support raylib!
    // https://github.com/sponsors/raysan5 https://www.patreon.com/raylib :)
    if (logo.skipped == true && logo.elapsedTime < 1.0f)
    {
        logo.elapsedTime += frameTime;
        return;
    }

    switch (logo.state)
    {
        case LOGO_START: // Small box blinking
            logo.elapsedTime += frameTime;
            if (logo.elapsedTime >= 2.0f) // 2 seconds delay
            {
                logo.state = LOGO_GROW1;
                logo.elapsedTime = 0.0f; // Reset counter... will be used later...
            }
            break;

        case LOGO_GROW1: // Top and left bars growing
            logo.topSideRecWidth += growSpeed*frameTime;
            logo.leftSideRecHeight += growSpeed*frameTime;

            if (logo.topSideRecWidth >= RAYLIB_LOGO_WIDTH)
            {
                logo.topSideRecWidth = RAYLIB_LOGO_WIDTH;
                logo.leftSideRecHeight = RAYLIB_LOGO_WIDTH;
                logo.state = LOGO_GROW2;
                logo.elapsedTime = 0.0f;
            }
            break;

        case LOGO_GROW2: // Bottom and right bars growing
            logo.bottomSideRecWidth += growSpeed*frameTime;
            logo.rightSideRecHeight += growSpeed*frameTime;

            if (logo.bottomSideRecWidth >= RAYLIB_LOGO_WIDTH)
            {
                logo.bottomSideRecWidth = RAYLIB_LOGO_WIDTH;
                logo.rightSideRecHeight = RAYLIB_LOGO_WIDTH;
                logo.state = LOGO_TEXT;
                logo.elapsedTime = 0.0f;
            }
            break;

        case LOGO_TEXT: // Letters appearing (one by one)
            logo.elapsedTime += frameTime;

            if (logo.lettersCount < 10 && logo.elapsedTime >= letterDelay)
            {
                logo.lettersCount++;
                logo.elapsedTime = 0.0f;
            }

            // When all letters have appeared, just fade out everything
            if (logo.lettersCount >= 10)
            {
                logo.alpha += fadeSpeed*frameTime;
                if (logo.alpha >= 1.0f)
                {
                    logo.alpha = 1.0f;
                    logo.state = LOGO_PAUSE;
                    logo.elapsedTime = 0.0f;
                }
            }
            break;

        case LOGO_PAUSE: // Pause at end of animation
            logo.elapsedTime += frameTime;
            if (logo.elapsedTime >= 1.5f)
                logo.state = LOGO_END;
            break;

        case LOGO_END: // Animation is finished
            currentScreen++;
            break;
    }
}

void DrawRaylibLogo(void)
{
    int lineWidth = (int)(RAYLIB_LOGO_OUTLINE); // DrawRectangle() takes ints, so all this casting is just to remove warnings
    int offsetA   = (int)(RAYLIB_LOGO_WIDTH*0.9375f);
    int offsetB   = (int)(lineWidth*2);
    int offsetC   = (int)(RAYLIB_LOGO_WIDTH*0.171875f);
    int offsetD   = (int)(RAYLIB_LOGO_WIDTH*0.1875f);
    int fontSize  = (int)(RAYLIB_LOGO_FONT_SIZE);

    int rectPosX    = (int)logo.positionX;
    int rectPosY    = (int)logo.positionY;
    int topWidth    = (int)logo.topSideRecWidth;
    int leftHeight  = (int)logo.leftSideRecHeight;
    int rightHeight = (int)logo.rightSideRecHeight;
    int bottomWidth = (int)logo.bottomSideRecWidth;

    ClearBackground(RAYLIB_LOGO_BACKGROUND);

    if (logo.state < LOGO_PAUSE)
        DrawText("powered by",
                 (int)((VIRTUAL_WIDTH/2) - (RAYLIB_LOGO_WIDTH/2)),
                 (int)((VIRTUAL_HEIGHT/2) - (RAYLIB_LOGO_WIDTH/2) - offsetB - lineWidth/4),
                 (int)(fontSize/2), RAYLIB_LOGO_COLOR);

    switch (logo.state)
    {
        case LOGO_START:
            if (((int)(logo.elapsedTime*4)) % 2)
                DrawRectangle(rectPosX, rectPosY, lineWidth, lineWidth, RAYLIB_LOGO_COLOR);
            else
                DrawRectangle(rectPosX, rectPosY, lineWidth, lineWidth, RAYLIB_LOGO_BACKGROUND);
            break;
        case LOGO_GROW1:
            DrawRectangle(rectPosX, rectPosY, topWidth, lineWidth, RAYLIB_LOGO_COLOR);
            DrawRectangle(rectPosX, rectPosY, lineWidth, leftHeight, RAYLIB_LOGO_COLOR);
            break;
        case LOGO_GROW2:
            DrawRectangle(rectPosX, rectPosY, topWidth, lineWidth, RAYLIB_LOGO_COLOR);
            DrawRectangle(rectPosX, rectPosY, lineWidth, leftHeight, RAYLIB_LOGO_COLOR);

            DrawRectangle(rectPosX + offsetA, rectPosY, lineWidth, rightHeight, RAYLIB_LOGO_COLOR);
            DrawRectangle(rectPosX, rectPosY + offsetA, bottomWidth, lineWidth, RAYLIB_LOGO_COLOR);
            break;
        case LOGO_TEXT:
            DrawRectangle(rectPosX, rectPosY, topWidth, lineWidth, RAYLIB_LOGO_COLOR);
            DrawRectangle(rectPosX, rectPosY + lineWidth, lineWidth, leftHeight - offsetB, RAYLIB_LOGO_COLOR);

            DrawRectangle(rectPosX + offsetA, rectPosY + lineWidth, lineWidth, rightHeight - offsetB, RAYLIB_LOGO_COLOR);
            DrawRectangle(rectPosX, rectPosY + offsetA, bottomWidth, lineWidth, RAYLIB_LOGO_COLOR);

            DrawText(TextSubtext("raylib", 0, logo.lettersCount),
                     VIRTUAL_WIDTH/2 - offsetC, VIRTUAL_HEIGHT/2 + offsetD,
                     fontSize, RAYLIB_LOGO_COLOR);

            DrawRectangle(0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, Fade(RAYLIB_LOGO_BACKGROUND, logo.alpha));
            break;
        case LOGO_PAUSE:
            break;
        case LOGO_END:
            break;
    }
}

