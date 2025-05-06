#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

//==============================================
//                 КОНСТАНТЫ
//==============================================
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MAX_MONSTERS 100
#define MAX_BONUSES 100
#define MAX_HIGHSCORES 5

//==============================================
//               ПЕРЕЧИСЛЕНИЯ
//==============================================
typedef enum
{
    MENU_MAIN,
    MENU_CONTROLS,
    MENU_LEADERBOARD,
    MODE_SURVIVAL,
    MODE_ARENA,
    GAME_PAUSED,
    GAME_OVER
} GameState;

typedef enum
{
    MONSTER_TYPE_BASIC,
    MONSTER_TYPE_FAST,
    MONSTER_TYPE_TANK
} MonsterType;

//==============================================
//               СТРУКТУРЫ
//==============================================
typedef struct
{
    Vector2 position;
    int health;
    bool isAlive;
} Player;

typedef struct
{
    Vector2 position;
    int health;
    bool isAlive;
    MonsterType type;
    bool isHit;
    float hitTimer;
    Vector2 knockbackDir;
} Monster;

typedef struct
{
    Vector2 position;
    bool isActive;
    float timer;
} Bonus;

typedef struct
{
    char name[16];
    int score;
} Highscore;

//==============================================
//            ПРОТОТИПЫ ФУНКЦИЙ
//==============================================
void ResetGame(void);
void SpawnMonster(void);
void SpawnBonus(Vector2 position);
void MaybeSpawnBonusRandom(float delta);
void LoadHighscores(void);
void SaveHighscore(int score);
void DrawMainMenu(void);
void DrawControls(void);
void DrawLeaderboard(void);
void UpdateGameLogic(float delta);
void DrawGame(void);

//==============================================
//            ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
//==============================================
static Player player;
static Monster monsters[MAX_MONSTERS];
static int monsterCount = 0;
static Bonus bonuses[MAX_BONUSES];
static int bonusCount = 0;

static int score = 0;
static Highscore highscores[MAX_HIGHSCORES];
static int highscoreCount = 0;

static GameState gameState = MENU_MAIN;
static int selectedOption = 0;
static bool gamePaused = false;
static float attackCooldown = 0.0f;
static float monsterAttackCooldown = 1.0f;
static float spawnTimer = 0.0f;
static float spawnInterval = 2.0f;
static const float spawnGrowthRate = 0.02f;

static bool playerHit = false;
static float playerHitTimer = 0.0f;
static bool flashScreen = false;
static float flashTimer = 0.0f;

static float bgScrollX = 0.0f;
static const float bgScrollSpeed = 20.0f;

static Texture2D texBackground;
static Texture2D texPlayer;
static Texture2D texMonster;
static Texture2D texBonus;
static Sound sfxAttack;
static Music bgmMusic;

//==============================================
//                  РЕАЛИЗАЦИЯ
//==============================================
void ResetGame(void)
{
    player.position = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
    player.health = 100;
    player.isAlive = true;

    monsterCount = bonusCount = 0;
    score = 0;
    spawnTimer = 0.0f;
    spawnInterval = 2.0f;
}

void SpawnMonster(void)
{
    if (monsterCount >= MAX_MONSTERS)
        return;
    Monster m = {0};
    m.position = (Vector2){(float)(rand() % SCREEN_WIDTH),
                           (float)(rand() % SCREEN_HEIGHT)};
    int r = rand() % 10;
    if (r < 6)
    {
        m.type = MONSTER_TYPE_BASIC;
        m.health = 25;
    }
    else if (r < 9)
    {
        m.type = MONSTER_TYPE_FAST;
        m.health = 15;
    }
    else
    {
        m.type = MONSTER_TYPE_TANK;
        m.health = 40;
    }
    m.isAlive = true;
    m.isHit = false;
    monsters[monsterCount++] = m;
}

void SpawnBonus(Vector2 position)
{
    if (bonusCount >= MAX_BONUSES)
        return;
    bonuses[bonusCount++] = (Bonus){
        .position = position,
        .isActive = true,
        .timer = 5.0f};
}

void MaybeSpawnBonusRandom(float delta)
{
    static float timer = 0.0f;
    timer += delta;
    if (timer >= 10.0f)
    {
        timer = 0.0f;
        SpawnBonus((Vector2){
            (float)(rand() % SCREEN_WIDTH),
            (float)(rand() % SCREEN_HEIGHT)});
    }
}

void LoadHighscores(void)
{
    FILE *f = fopen("highscores.txt", "r");
    if (!f)
        return;
    while (highscoreCount < MAX_HIGHSCORES &&
           fscanf(f, "%15s%d",
                  highscores[highscoreCount].name,
                  &highscores[highscoreCount].score) == 2)
        highscoreCount++;
    fclose(f);
}

void SaveHighscore(int sc)
{
    if (highscoreCount < MAX_HIGHSCORES)
    {
        highscores[highscoreCount].score = sc;
        snprintf(highscores[highscoreCount].name, 16, "Player");
        highscoreCount++;
    }
    FILE *f = fopen("highscores.txt", "w");
    for (int i = 0; i < highscoreCount; i++)
        fprintf(f, "%s %d\n",
                highscores[i].name, highscores[i].score);
    fclose(f);
}

void DrawMainMenu(void)
{
    DrawText("MAIN MENU", 320, 100, 30, WHITE);
    const char *opt[] = {
        "Survival Mode", "Arena Mode", "Leaderboard", "Controls", "Exit"};
    for (int i = 0; i < 5; i++)
        DrawText(opt[i], 350, 200 + i * 40, 20,
                 i == selectedOption ? YELLOW : LIGHTGRAY);
}

void DrawControls(void)
{
    DrawText("Controls", 340, 100, 30, WHITE);
    DrawText("WASD - Move", 320, 160, 20, LIGHTGRAY);
    DrawText("SPACE - Attack", 320, 190, 20, LIGHTGRAY);
    DrawText("P - Pause", 320, 220, 20, LIGHTGRAY);
    DrawText("ENTER - Select / Restart", 320, 250, 20, LIGHTGRAY);
    DrawText("ESC - Back / Exit", 320, 280, 20, LIGHTGRAY);
}

void DrawLeaderboard(void)
{
    DrawText("Highscores", 340, 100, 30, WHITE);
    for (int i = 0; i < highscoreCount; i++)
    {
        char buf[64];
        sprintf(buf, "%d. %s - %d",
                i + 1, highscores[i].name, highscores[i].score);
        DrawText(buf, 320, 160 + i * 30, 20, LIGHTGRAY);
    }
    DrawText("ESC to back", 300, 360, 20, GRAY);
}

void UpdateGameLogic(float delta)
{
    // эффекты
    if (playerHitTimer > 0)
        playerHitTimer -= delta;
    else
        playerHit = false;
    if (flashTimer > 0)
        flashTimer -= delta;
    else
        flashScreen = false;

    if (gameState != MODE_SURVIVAL && gameState != MODE_ARENA)
        return;

    if (IsKeyPressed(KEY_P))
        gamePaused = !gamePaused;
    if (gamePaused)
        return;

    if (player.health <= 0)
    {
        gameState = GAME_OVER;
        SaveHighscore(score);
        return;
    }

    // движение игрока
    if (IsKeyDown(KEY_W))
        player.position.y -= 200 * delta;
    if (IsKeyDown(KEY_S))
        player.position.y += 200 * delta;
    if (IsKeyDown(KEY_A))
        player.position.x -= 200 * delta;
    if (IsKeyDown(KEY_D))
        player.position.x += 200 * delta;

    // таймеры
    attackCooldown = fmaxf(0, attackCooldown - delta);
    monsterAttackCooldown = fmaxf(0, monsterAttackCooldown - delta);
    spawnTimer += delta;
    spawnInterval += spawnGrowthRate * delta;

    // фон
    bgScrollX += bgScrollSpeed * delta;
    if (bgScrollX > texBackground.width - SCREEN_WIDTH)
        bgScrollX = 0;

    // спавн
    if (spawnTimer >= spawnInterval)
    {
        spawnTimer = 0;
        SpawnMonster();
    }
    MaybeSpawnBonusRandom(delta);

    // вычисляем радиусы по texture+scale
    float pScale = 0.6f;
    float mScale = 0.5f;
    float bScale = 0.5f;
    float rPlayer = (texPlayer.width * pScale) / 2.0f;
    float rMonster = (texMonster.width * mScale) / 2.0f;
    float rBonus = (texBonus.width * bScale) / 2.0f;

    // бонусы
    for (int i = 0; i < bonusCount; i++)
    {
        if (!bonuses[i].isActive)
            continue;
        bonuses[i].timer -= delta;
        if (bonuses[i].timer <= 0)
        {
            bonuses[i].isActive = false;
            continue;
        }
        if (CheckCollisionCircles(player.position, rPlayer,
                                  bonuses[i].position, rBonus))
        {
            player.health = fmin(player.health + 20, 100);
            bonuses[i].isActive = false;
            flashScreen = true;
            flashTimer = 0.1f;
        }
    }

    // монстры
    for (int i = 0; i < monsterCount; i++)
    {
        Monster *m = &monsters[i];
        if (!m->isAlive)
            continue;
        // движение
        Vector2 dir = Vector2Subtract(player.position, m->position);
        float speed = (m->type == MONSTER_TYPE_FAST ? 150 : m->type == MONSTER_TYPE_TANK ? 80
                                                                                         : 100);
        float len = Vector2Length(dir);
        if (len > 0)
            dir = Vector2Scale(dir, speed * delta / len);
        m->position = Vector2Add(m->position, dir);
        // атака на игрока
        if (CheckCollisionCircles(player.position, rPlayer,
                                  m->position, rMonster) &&
            monsterAttackCooldown <= 0)
        {
            player.health -= 10;
            monsterAttackCooldown = 1.0f;
            playerHit = true;
            playerHitTimer = 0.2f;
        }
        // удар игроком
        if (CheckCollisionCircles(player.position, rPlayer,
                                  m->position, rMonster) &&
            IsKeyDown(KEY_SPACE) && attackCooldown <= 0)
        {
            m->health -= 10;
            attackCooldown = 0.5f;
            PlaySound(sfxAttack);
            // эффект knockback от игрока
            m->isHit = true;
            m->hitTimer = 0.2f;
            m->knockbackDir = Vector2Normalize(
                Vector2Subtract(m->position, player.position));
        }
        // отбрасывание
        if (m->isHit)
        {
            m->position = Vector2Add(
                m->position,
                Vector2Scale(m->knockbackDir, 200 * delta));
            m->hitTimer -= delta;
            if (m->hitTimer <= 0)
                m->isHit = false;
        }
        // смерть
        if (m->health <= 0 && m->isAlive)
        {
            m->isAlive = false;
            score += 10;
            SpawnBonus(m->position);
        }
    }
}

void DrawGame(void)
{
    // фон
    Rectangle src = {bgScrollX, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    Rectangle dst = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    DrawTexturePro(texBackground, src, dst, (Vector2){0}, 0, WHITE);

    // вспышка
    if (flashScreen)
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(WHITE, 0.5f));

    // игрок
    float pScale = 0.6f;
    Color pCol = playerHit ? RED : WHITE;
    Vector2 pDraw = {
        player.position.x - (texPlayer.width * pScale) / 2,
        player.position.y - (texPlayer.height * pScale) / 2};
    DrawTextureEx(texPlayer, pDraw, 0, pScale, pCol);

    // монстры
    float mScale = 0.5f;
    for (int i = 0; i < monsterCount; i++)
    {
        Monster *m = &monsters[i];
        if (!m->isAlive)
            continue;
        Color mCol = m->isHit ? RED : WHITE;
        Vector2 mDraw = {
            m->position.x - (texMonster.width * mScale) / 2,
            m->position.y - (texMonster.height * mScale) / 2};
        DrawTextureEx(texMonster, mDraw, 0, mScale, mCol);
    }

    // бонусы
    float bScale = 0.5f;
    for (int i = 0; i < bonusCount; i++)
    {
        if (!bonuses[i].isActive)
            continue;
        Vector2 bDraw = {
            bonuses[i].position.x - (texBonus.width * bScale) / 2,
            bonuses[i].position.y - (texBonus.height * bScale) / 2};
        DrawTextureEx(texBonus, bDraw, 0, bScale, WHITE);
    }

    // HUD
    DrawText(TextFormat("HP: %d", player.health), 10, 10, 20, WHITE);
    DrawText(TextFormat("Score: %d", score), 10, 40, 20, WHITE);
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Monster Battle Game");
    InitAudioDevice();
    SetTargetFPS(60);
    srand(time(NULL));

    texBackground = LoadTexture("resources/background.png");
    texPlayer = LoadTexture("resources/player.png");
    texMonster = LoadTexture("resources/monster.png");
    texBonus = LoadTexture("resources/bonus.png");
    sfxAttack = LoadSound("resources/attack.wav");
    bgmMusic = LoadMusicStream("resources/music.mp3");
    PlayMusicStream(bgmMusic);

    LoadHighscores();
    ResetGame();

    while (!WindowShouldClose())
    {
        float delta = GetFrameTime();

        // меню и режимы
        BeginDrawing();
        ClearBackground(BLACK);
        switch (gameState)
        {
        case MENU_MAIN:
            DrawMainMenu();
            if (IsKeyPressed(KEY_DOWN))
                selectedOption = (selectedOption + 1) % 5;
            if (IsKeyPressed(KEY_UP))
                selectedOption = (selectedOption + 4) % 5;
            if (IsKeyPressed(KEY_ENTER))
            {
                if (selectedOption == 0)
                {
                    gameState = MODE_SURVIVAL;
                    ResetGame();
                }
                if (selectedOption == 1)
                {
                    gameState = MODE_ARENA;
                    ResetGame();
                }
                if (selectedOption == 2)
                {
                    gameState = MENU_LEADERBOARD;
                }
                if (selectedOption == 3)
                {
                    gameState = MENU_CONTROLS;
                }
                if (selectedOption == 4)
                {
                    CloseWindow();
                }
            }
            break;
        case MENU_CONTROLS:
            DrawControls();
            if (IsKeyPressed(KEY_ESCAPE))
                gameState = MENU_MAIN;
            break;
        case MENU_LEADERBOARD:
            DrawLeaderboard();
            if (IsKeyPressed(KEY_ESCAPE))
                gameState = MENU_MAIN;
            break;
        case MODE_SURVIVAL:
        case MODE_ARENA:
            UpdateGameLogic(delta);
            UpdateMusicStream(bgmMusic);
            DrawGame();
            break;
        case GAME_OVER:
            DrawText("GAME OVER",
                     SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 - 20, 30, RED);
            DrawText("ENTER to return",
                     SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 2 + 20, 20, WHITE);
            if (IsKeyPressed(KEY_ENTER))
                gameState = MENU_MAIN;
            break;
        default:
            break;
        }
        EndDrawing();
    }

    UnloadTexture(texBackground);
    UnloadTexture(texPlayer);
    UnloadTexture(texMonster);
    UnloadTexture(texBonus);
    UnloadSound(sfxAttack);
    UnloadMusicStream(bgmMusic);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
