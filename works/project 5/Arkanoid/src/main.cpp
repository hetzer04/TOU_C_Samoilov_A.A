#include "raylib.h"
#include <cmath>

int main()
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Arkanoid на raylib");
    SetTargetFPS(60);

    Vector2 ballPosition = {(float)screenWidth / 2, (float)screenHeight / 2};
    Vector2 ballSpeed = {5.0f, 4.0f};
    int ballRadius = 20;

    Vector2 paddlePosition = {50.0f, (float)screenHeight / 2 - 50.0f};
    Vector2 paddleSize = {20.0f, 100.0f};

    int score = 0;

    while (!WindowShouldClose())
    {
        ballPosition.x += ballSpeed.x;
        ballPosition.y += ballSpeed.y;

        if ((ballPosition.y >= (screenHeight - ballRadius)) || (ballPosition.y <= ballRadius))
        {
            ballSpeed.y *= -1.0f;
        }

        if (ballPosition.x >= (screenWidth - ballRadius))
        {
            ballSpeed.x *= -1.0f;
        }

        if (ballPosition.x < 0)
        {
            ballPosition = {(float)screenWidth / 2, (float)screenHeight / 2};
            ballSpeed = {5.0f, 4.0f};
            score = 0;
        }

        paddlePosition.y = GetMouseY() - paddleSize.y / 2;
        if (paddlePosition.y < 0)
            paddlePosition.y = 0;
        if (paddlePosition.y > screenHeight - paddleSize.y)
            paddlePosition.y = screenHeight - paddleSize.y;

        if (CheckCollisionCircleRec(ballPosition, ballRadius, {paddlePosition.x, paddlePosition.y, paddleSize.x, paddleSize.y}))
        {
            if (ballSpeed.x < 0)
            {
                ballSpeed.x *= -1.1f;
                score++;
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawCircleV(ballPosition, ballRadius, MAROON);
        DrawRectangleV(paddlePosition, paddleSize, BLACK);
        DrawText(TextFormat("Score: %i", score), 10, 10, 20, DARKGRAY);
        DrawText("Управляйте ракеткой с помощью мыши", screenWidth - 350, screenHeight - 30, 15, GRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
