#include "raylib.h"
#include <cstdlib>
#include <ctime>

enum Choice
{
    NONE,
    ROCK,
    PAPER,
    SCISSORS
};

struct Player
{
    Choice choice;
    Vector2 position;
    float scale;
    bool animating;
    bool fadingOut;
    float animationTimer;
    int alpha;
};

// Глобальные переменные
Texture2D rockTexture, paperTexture, scissorsTexture, questionTexture, bgTexture;
Texture2D rockButtonTexture, paperButtonTexture, scissorsButtonTexture;
Player player, computer;
bool gameStarted = false;
bool animationComplete = false;
const char *resultText = "";

// Функция загрузки текстур
void LoadAssets()
{
    rockTexture = LoadTexture("resources/rock.png");
    paperTexture = LoadTexture("resources/paper.png");
    scissorsTexture = LoadTexture("resources/noj.png");
    questionTexture = LoadTexture("resources/qu.png");
    bgTexture = LoadTexture("resources/wabbit_alpha.png");

    rockButtonTexture = LoadTexture("resources/rock_btn.png");
    paperButtonTexture = LoadTexture("resources/paper_btn.png");
    scissorsButtonTexture = LoadTexture("resources/scissors_btn.png");
}

// Функция освобождения памяти
void UnloadAssets()
{
    UnloadTexture(rockTexture);
    UnloadTexture(paperTexture);
    UnloadTexture(scissorsTexture);
    UnloadTexture(questionTexture);
    UnloadTexture(bgTexture);

    UnloadTexture(rockButtonTexture);
    UnloadTexture(paperButtonTexture);
    UnloadTexture(scissorsButtonTexture);
}

// Случайный выбор компьютера
Choice GetRandomChoice()
{
    return static_cast<Choice>(1 + rand() % 3);
}

// Анимация выбора
void AnimateSelection(Player &p)
{
    p.animating = true;
    p.fadingOut = false;
    p.animationTimer = 0.0f;
    p.alpha = 255;
}

// Определение победителя
const char *DetermineWinner(Choice playerChoice, Choice computerChoice)
{
    if (playerChoice == computerChoice)
        return "Draw!";
    if ((playerChoice == ROCK && computerChoice == SCISSORS) ||
        (playerChoice == SCISSORS && computerChoice == PAPER) ||
        (playerChoice == PAPER && computerChoice == ROCK))
    {
        return "You Win!";
    }
    return "You Lose!";
}

int main()
{
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Rock Paper Scissors");
    SetTargetFPS(60);
    srand(time(NULL));

    LoadAssets();

    // Позиции кнопок
    Rectangle rockButton = {230, 100, 160, 100};
    Rectangle paperButton = {390, 100, 100, 100};
    Rectangle scissorsButton = {490, 100, 160, 100};

    player.choice = NONE;
    player.position = {-50, 100};
    player.scale = 1.0f;
    player.animating = false;
    player.fadingOut = false;
    player.alpha = 255;

    computer.choice = NONE;
    computer.position = {350, 100};
    computer.scale = 1.0f;
    computer.animating = false;
    computer.fadingOut = false;
    computer.alpha = 255;

    while (!WindowShouldClose())
    {
        // Обработка ввода
        if (!gameStarted)
        {
            Vector2 mousePos = GetMousePosition();

            // Проверка нажатий клавиш
            if (IsKeyPressed(KEY_R) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, rockButton)))
            {
                player.choice = ROCK;
                AnimateSelection(player);
            }
            else if (IsKeyPressed(KEY_P) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, paperButton)))
            {
                player.choice = PAPER;
                AnimateSelection(player);
            }
            else if (IsKeyPressed(KEY_S) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, scissorsButton)))
            {
                player.choice = SCISSORS;
                AnimateSelection(player);
            }
        }

        // Анимация выбора
        if (player.animating)
        {
            player.animationTimer += GetFrameTime();
            player.position.x += (200 - player.position.x) * 0.1f;
            if (player.animationTimer > 0.3f)
            {
                player.animating = false;
                gameStarted = true;
                computer.choice = GetRandomChoice();
                AnimateSelection(computer);
            }
        }

        if (computer.animating)
        {
            computer.animationTimer += GetFrameTime();
            computer.position.x += (200 - computer.position.x) * 0.1f;
            if (computer.animationTimer > 0.3f)
            {
                computer.animating = false;
                animationComplete = true;
                resultText = DetermineWinner(player.choice, computer.choice);
                if (resultText == "You Win!")
                {
                    computer.fadingOut = true;
                }
                else if (resultText == "You Lose!")
                {
                    player.fadingOut = true;
                }
            }
        }

        // Анимация исчезновения проигравшего
        if (player.fadingOut)
        {
            player.alpha -= 15;
            if (player.alpha <= 0)
                player.alpha = 0;
        }
        if (computer.fadingOut)
        {
            computer.alpha -= 15;
            if (computer.alpha <= 0)
                computer.alpha = 0;
        }

        // Сброс игры
        if (gameStarted && IsKeyPressed(KEY_SPACE))
        {
            player.choice = NONE;
            computer.choice = NONE;
            player.position = {-50, 100};
            computer.position = {350, 100};
            player.fadingOut = false;
            computer.fadingOut = false;
            gameStarted = false;
            resultText = "";
            player.alpha = 255;
            computer.alpha = 255;
        }

        // Отрисовка
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(bgTexture, 0, 0, WHITE);

        DrawText("Rock Paper Scissors!", 280, 50, 30, BLACK);
        DrawText("Press R (Rock), P (Paper), S (Scissors)", 230, 100, 20, DARKGRAY);

        // Отрисовка кнопок
        DrawTexture(rockButtonTexture, rockButton.x, rockButton.y, WHITE);
        DrawTexture(paperButtonTexture, paperButton.x, paperButton.y, WHITE);
        DrawTexture(scissorsButtonTexture, scissorsButton.x, scissorsButton.y, WHITE);

        // До выбора отображаем знаки вопроса
        if (player.choice == NONE)
        {
            DrawTextureEx(questionTexture, player.position, 0, 1.0f, WHITE);
        }
        else
        {
            Texture2D tex = (player.choice == ROCK) ? rockTexture : (player.choice == PAPER) ? paperTexture
                                                                                             : scissorsTexture;
            DrawTextureEx(tex, player.position, 0, player.scale, Fade(WHITE, player.alpha / 255.0f));
        }

        if (computer.choice == NONE)
        {
            DrawTextureEx(questionTexture, computer.position, 0, 1.0f, WHITE);
        }
        else
        {
            Texture2D tex = (computer.choice == ROCK) ? rockTexture : (computer.choice == PAPER) ? paperTexture
                                                                                                 : scissorsTexture;
            DrawTextureEx(tex, computer.position, 0, computer.scale, Fade(WHITE, computer.alpha / 255.0f));
        }

        DrawText(resultText, 340, 500, 30, RED);

        if (gameStarted)
        {
            DrawText("Press SPACE to restart", 280, 550, 20, GRAY);
        }

        EndDrawing();
    }

    UnloadAssets();
    CloseWindow();
    return 0;
}
