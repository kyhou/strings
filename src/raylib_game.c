/*******************************************************************************************
 *
 *   raylib gamejam template
 *
 *   Template originally created with raylib 4.5-dev, last time updated with
 *raylib 5.0
 *
 *   Template licensed under an unmodified zlib/libpng license, which is an
 *OSI-certified, BSD-like license that allows static linking with closed source
 *software
 *
 *   Copyright (c) 2022-2024 Ramon Santamaria (@raysan5)
 *
 ********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#if defined(PLATFORM_WEB)
#define CUSTOM_MODAL_DIALOGS       // Force custom modal dialogs usage
#include <emscripten/emscripten.h> // Emscripten library - LLVM to JavaScript compiler
#endif

#include <math.h>
#include <stdio.h>  // Required for: printf()
#include <stdlib.h> // Required for:
#include <string.h> // Required for:
#include <stdbool.h>

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

#define G 800
#define PLAYER_JUMP_SPD 400.0f
#define PLAYER_HOR_SPD 200.0f

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum GameScreen
{
  LOGO = 0,
  TITLE,
  GAMEPLAY,
  ENDING
} GameScreen;

typedef struct EnvItem
{
  Rectangle rect;
  int blocking;
  Color color;
} EnvItem;

typedef struct Points
{
  int capacity;
  int last;
  Vector2 *points;
} Points;

typedef struct Player
{
  Rectangle rect;
  float size;
  Vector2 position;
  float speed;
  bool canJump;
  Points redLine;
  Points greenLine;
  Points blueLine;
  int selectedColor;
} Player;

typedef struct Goal
{
  Rectangle rect;
  Color color;
  bool isSet;
} Goal;

typedef struct LineSpawner
{
  Rectangle rect;
  Color color;
  bool activated;
} LineSpawner;

typedef struct Level
{
  int id;
  int qtdSpawners;
  LineSpawner *lineSpawners;
  int qtdGoals;
  Goal *goals;
  int qtdEnvItems;
  EnvItem *envItems;
} Level;

typedef struct Line
{
  Vector2 start;
  Vector2 end;
} Line;

typedef struct LineRecColisions
{
  Vector2 leftColisionPoint;
  Vector2 rightColisionPoint;
  Vector2 topColisionPoint;
  Vector2 bottomColisionPoint;
} LineRecColisions;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

static RenderTexture2D target = {0}; // Render texture to render our game

static Player player = {0};
static Camera2D camera = {0};
static Level level1 = {
    .id = 1,
    .qtdEnvItems = 5,
    .qtdGoals = 1,
    .qtdSpawners = 1};

static Level level2 = {
    .id = 2,
    .qtdEnvItems = 5};
static int envItemsLength;
static int spawnersLength;
static int goalsLength;

GameScreen currentScreen = TITLE;
int framesCounter = 0;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void); // Update and Draw one frame
void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength,
                  float delta);
void UpdateCameraCenterInsideMap(Camera2D *camera, Player *player,
                                 EnvItem *envItems, int envItemsLength,
                                 float delta, int width, int height);
bool ColorIsEqual(Color col1, Color col2);
void NewLinePoint(Vector2 playerCenter, Player *player, EnvItem *envItems, int envItemsLength);
static void Reset();
bool CheckLineRecColision(Line line, Rectangle rec, LineRecColisions *collisionPoints);
bool CheckLineEnvColision(Line line, EnvItem *envItems, int qtdEnvItems, LineRecColisions *collisionPoints);
Vector2 GetLineEnvItemClosestColisionVector2(Line line, EnvItem *envItems, int qtdEnvItems);
bool lineLine(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, Vector2 *intersectionPoint);
void DrawClampedLine(float x1, float y1, float x2, float y2, float length, Color color);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
#if !defined(_DEBUG)
  SetTraceLogLevel(LOG_NONE); // Disable raylib trace log messages
#endif

  // Initialization
  //--------------------------------------------------------------------------------------
  InitWindow(screenWidth, screenHeight, "Strings");

  // TODO: Load resources / Initialize variables at this point

  // Render texture to draw full screen, enables screen scaling
  // NOTE: If screen is scaled, mouse input should be scaled proportionally
  target = LoadRenderTexture(screenWidth, screenHeight);
  SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

  player.position = (Vector2){400, 280};
  player.speed = 0;
  player.canJump = false;
  player.size = 40;

  level1.envItems = (EnvItem *)malloc(level1.qtdEnvItems * sizeof(EnvItem));
  level1.envItems[0] = (EnvItem){{0, 0, 1000, 400}, 0, LIGHTGRAY};
  level1.envItems[1] = (EnvItem){{0, 400, 1000, 200}, 1, GRAY};
  level1.envItems[2] = (EnvItem){{300, 200, 400, 10}, 1, GRAY};
  level1.envItems[3] = (EnvItem){{250, 300, 100, 10}, 1, GRAY};
  level1.envItems[4] = (EnvItem){{650, 300, 100, 10}, 1, GRAY};

  level1.lineSpawners = (LineSpawner *)malloc(level1.qtdSpawners * sizeof(LineSpawner));
  level1.lineSpawners[0] = (LineSpawner){.color = RED, .activated = false, .rect = {.x = 200, .y = 375, .width = 10, .height = 25}};

  level1.goals = (Goal *)malloc(level1.qtdGoals * sizeof(Goal));
  level1.goals[0] = (Goal){.color = RED, .isSet = false, .rect = {.x = 600, .y = 300, .width = 50, .height = 100}};

  level2.envItems = (EnvItem *)malloc(level2.qtdEnvItems * sizeof(EnvItem));
  level2.envItems[0] = (EnvItem){{0, 0, 1000, 400}, 0, LIGHTGRAY};
  level2.envItems[1] = (EnvItem){{0, 400, 1000, 200}, 1, GREEN};
  level2.envItems[2] = (EnvItem){{300, 200, 400, 10}, 1, GREEN};
  level2.envItems[3] = (EnvItem){{250, 300, 100, 10}, 1, GREEN};
  level2.envItems[4] = (EnvItem){{650, 300, 100, 10}, 1, GREEN};

  Points redLine;
  redLine.last = -1;
  redLine.capacity = 5;
  redLine.points = (Vector2 *)malloc(redLine.capacity * sizeof(Vector2));
  player.redLine = redLine;

  Points greenLine;
  greenLine.last = -1;
  greenLine.capacity = 5;
  greenLine.points = (Vector2 *)malloc(greenLine.capacity * sizeof(Vector2));
  player.greenLine = greenLine;

  Points blueLine;
  blueLine.last = -1;
  blueLine.capacity = 5;
  blueLine.points = (Vector2 *)malloc(blueLine.capacity * sizeof(Vector2));
  player.blueLine = blueLine;

  envItemsLength = level1.qtdEnvItems;
  spawnersLength = level1.qtdSpawners;
  goalsLength = level1.qtdGoals;

  camera.target = Vector2Zero();
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;

  //   playerLines = (Line *)malloc(0 * sizeof(Line));

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, 60, true);
#else
  SetTargetFPS(60); // Set our game frames-per-second
  //--------------------------------------------------------------------------------------
  // Main game loop
  while (!WindowShouldClose()) // Detect window close button
  {
    UpdateDrawFrame();
  }
#endif

  // De-Initialization
  //--------------------------------------------------------------------------------------
  UnloadRenderTexture(target);

  // TODO: Unload all loaded resources at this point

  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}

//--------------------------------------------------------------------------------------------
// Module functions definition
//--------------------------------------------------------------------------------------------
// Update and draw frame
static void UpdateDrawFrame(void)
{
  // Update
  //----------------------------------------------------------------------------------

  float deltaTime = GetFrameTime();
  Vector2 topLeft;
  topLeft = GetScreenToWorld2D((Vector2){0, 0}, camera);

  switch (currentScreen)
  {
  case LOGO:
  {
    // TODO: Update LOGO screen variables here!

    framesCounter++; // Count frames

    // Wait for 2 seconds (120 frames) before jumping to TITLE screen
    if (framesCounter > 120)
    {
      currentScreen = TITLE;
    }
  }
  break;
  case TITLE:
  {
    // TODO: Update TITLE screen variables here!

    // Press enter to change to GAMEPLAY screen
    if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP))
    {
      currentScreen = GAMEPLAY;
    }
  }
  break;
  case GAMEPLAY:
  {
    // TODO: Update GAMEPLAY screen variables here!

    UpdatePlayer(&player, level1.envItems, envItemsLength, deltaTime);

    if (IsKeyPressed(KEY_R))
    {
      Reset();
    }
    UpdateCameraCenterInsideMap(&camera, &player, level1.envItems, envItemsLength,
                                deltaTime, screenWidth, screenHeight);

    //----------------------------------------------------------------------------------

    // Press enter to change to ENDING screen
    // if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP))
    // {
    //   currentScreen = ENDING;
    // }
  }
  break;
  case ENDING:
  {
    // TODO: Update ENDING screen variables here!

    // Press enter to return to TITLE screen
    if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP))
    {
      currentScreen = TITLE;
    }
  }
  break;
  default:
    break;
  }
  // Draw
  //----------------------------------------------------------------------------------
  // Render game screen to a texture,
  // it could be useful for scaling or further shader postprocessing
  BeginTextureMode(target);
  ClearBackground(RAYWHITE);

  // TODO: Draw your game screen here
  // DrawText("Welcome to raylib NEXT gamejam!", 150, 140, 30, BLACK);
  // DrawRectangleLinesEx((Rectangle){0, 0, screenWidth, screenHeight}, 16,
  // BLACK);
  BeginMode2D(camera);

  switch (currentScreen)
  {
  case LOGO:
  case TITLE:
  {
    // TODO: Draw TITLE screen here!
    DrawRectangle(0, 0, screenWidth, screenHeight, GREEN);
    DrawText("Strings", 20, 20, 40, DARKGREEN);
    DrawText("PRESS ENTER", 120, 220, 20, DARKGREEN);
  }
  break;
  case GAMEPLAY:
  {
    DrawFPS(100, 100);

    for (int i = 0; i < envItemsLength; i++)
    {
      DrawRectangleRec(level1.envItems[i].rect, level1.envItems[i].color);
    }

    for (int i = 0; i < spawnersLength; i++)
    {
      if (CheckCollisionRecs(player.rect, level1.lineSpawners[i].rect))
      {
        DrawText("E", (int)(level1.lineSpawners[i].rect.x + 15), (int)(level1.lineSpawners[i].rect.y - 35), 30, BLACK);
      }
      DrawRectangleRec(level1.lineSpawners[i].rect, level1.lineSpawners[i].color);
    }

    for (int i = 0; i < goalsLength; i++)
    {
      if (level1.goals[i].isSet)
      {
        DrawRectangleRec(level1.goals[i].rect, level1.goals[i].color);
      }
      else
      {
        if (CheckCollisionRecs(player.rect, level1.goals[i].rect))
        {
          DrawText("C", (int)(level1.goals[i].rect.x + 15), (int)(level1.goals[i].rect.y - 35), 30, BLACK);
        }
        DrawRectangleLinesEx(level1.goals[i].rect, 1.0f, level1.goals[i].color);
      }
    }

    // char *playerX;
    // asprintf(&playerX, "x = %d\n", player.position.x);
    // char *playerY;
    // asprintf(&playerY, "y = %d\n", player.position.y);

    // DrawText(playerX, 150, 140, 30, BLACK);
    // DrawText(playerY, 150, 180, 30, BLACK);

    player.rect = (Rectangle){player.position.x - (player.size / 2.0f), player.position.y - player.size, player.size,
                              player.size};
    DrawRectangleRec(player.rect, RED);

    // DrawCircleV(player.position, 5.0f, GOLD);
    DrawText("Press C to create a point of the selected Color", (int)(topLeft.x + 10), (int)(topLeft.y + 10), 20, BLACK);

    switch (player.selectedColor)
    {
    case 1:
      DrawText("Red", (int)(topLeft.x + 10), (int)(topLeft.y + 30), 30, BLACK);
      break;
    case 2:
      DrawText("Green", (int)(topLeft.x + 10), (int)(topLeft.y + 30), 30, BLACK);
      break;
    case 3:
      DrawText("Blue", (int)(topLeft.x + 10), (int)(topLeft.y + 30), 30, BLACK);
      break;

    default:
      break;
    }

    for (int i = 0; i <= player.redLine.last; i++)
    {
      bool allGoalsReached = true;
      for (int j = 0; j < level1.qtdGoals; j++)
      {
        if (ColorIsEqual(level1.goals[j].color, RED))
        {
          allGoalsReached &= CheckCollisionPointRec(player.redLine.points[i], level1.goals[j].rect);
        }
      }

      if (!allGoalsReached)
      {
        if (i == player.redLine.last && i < player.redLine.capacity - 1 && player.selectedColor == 1)
        {
          DrawClampedLine(player.redLine.points[i].x, player.redLine.points[i].y, player.position.x, player.position.y - (player.size / 2), 500, RED);
        }
        else if (i < player.redLine.last)
        {
          DrawClampedLine(player.redLine.points[i].x, player.redLine.points[i].y, player.redLine.points[i + 1].x, player.redLine.points[i + 1].y, 500, RED);
        }
      }

      DrawCircleV(player.redLine.points[i], 5.0f, GOLD);
    }

    for (int i = 0; i <= player.greenLine.last; i++)
    {
      bool allGoalsReached = true;
      for (int j = 0; j < level1.qtdGoals; j++)
      {

        if (ColorIsEqual(level1.goals[j].color, RED))
        {
          allGoalsReached &= CheckCollisionPointRec(player.greenLine.points[i], level1.goals[j].rect);
        }
      }

      if (!allGoalsReached)
      {
        if (i == player.greenLine.last && i < player.greenLine.capacity - 1 && player.selectedColor == 2)
        {
          DrawClampedLine(player.greenLine.points[i].x, player.greenLine.points[i].y, player.position.x, player.position.y - (player.size / 2), 500, GREEN);
        }
        else if (i < player.greenLine.last)
        {
          DrawClampedLine(player.greenLine.points[i].x, player.greenLine.points[i].y, player.greenLine.points[i + 1].x, player.greenLine.points[i + 1].y, 500, GREEN);
        }
      }

      DrawCircleV(player.greenLine.points[i], 5.0f, GOLD);
    }

    for (int i = 0; i <= player.blueLine.last; i++)
    {
      bool allGoalsReached = true;
      for (int j = 0; j < level1.qtdGoals; j++)
      {

        if (ColorIsEqual(level1.goals[j].color, RED))
        {
          allGoalsReached &= CheckCollisionPointRec(player.blueLine.points[i], level1.goals[j].rect);
        }
      }

      if (!allGoalsReached)
      {
        if (i == player.blueLine.last && i < player.blueLine.capacity - 1 && player.selectedColor == 3)
        {
          DrawClampedLine(player.blueLine.points[i].x, player.blueLine.points[i].y, player.position.x, player.position.y - (player.size / 2), 500, BLUE);
        }
        else if (i < player.blueLine.last)
        {
          DrawClampedLine(player.blueLine.points[i].x, player.blueLine.points[i].y, player.blueLine.points[i + 1].x, player.blueLine.points[i + 1].y, 500, BLUE);
        }
      }

      DrawCircleV(player.blueLine.points[i], 5.0f, GOLD);
    }

    bool allGoalsReached = true;
    for (int i = 0; i < level1.qtdGoals; i++)
    {
      allGoalsReached &= level1.goals[i].isSet;
    }

    if (allGoalsReached)
    {
      currentScreen = ENDING;
    }
  }
  break;
  case ENDING:
  {
    DrawRectangle(0, 0, screenWidth, screenHeight, GREEN);
    DrawText("The End", 20, 20, 40, DARKBLUE);
    DrawText("PRESS ENTER", 120, 220, 20, DARKBLUE);
    Reset();
    camera.target = Vector2Zero();
    camera.offset = Vector2Zero();
    break;
  }
  }
  EndMode2D();
  EndTextureMode();

  // Render to screen (main framebuffer)
  // BeginDrawing();
  // ClearBackground(RAYWHITE);

  // Draw render texture to screen, scaled if required
  DrawTexturePro(target.texture,
                 (Rectangle){0, 0, (float)target.texture.width,
                             -(float)target.texture.height},
                 (Rectangle){0, 0, (float)target.texture.width,
                             (float)target.texture.height},
                 (Vector2){0, 0}, 0.0f, WHITE);

  // TODO: Draw everything that requires to be drawn at this point, maybe UI?

  EndDrawing();
  //----------------------------------------------------------------------------------
}

void Reset()
{
  camera.zoom = 1.0f;
  player.position = (Vector2){400, 280};

  for (int i = 0; i < level1.qtdGoals; i++)
  {
    level1.goals[i].isSet = false;
  }

  for (int i = 0; i < level1.qtdSpawners; i++)
  {
    level1.lineSpawners[i].activated = false;
  }

  player.redLine.last = -1;
  free(player.redLine.points);
  player.redLine.points = (Vector2 *)malloc(5 * sizeof(Vector2));

  player.greenLine.last = -1;
  free(player.greenLine.points);
  player.greenLine.points = (Vector2 *)malloc(5 * sizeof(Vector2));

  player.blueLine.last = -1;
  free(player.blueLine.points);
  player.blueLine.points = (Vector2 *)malloc(5 * sizeof(Vector2));
}

void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength,
                  float delta)
{
  Vector2 playerCenter = (Vector2){player->position.x, player->position.y - player->size / 2};

  if (IsKeyPressed(KEY_Q))
  {
    player->selectedColor = 0;
    for (int i = 0; i < level1.qtdSpawners; i++)
    {
      level1.lineSpawners[i].activated = false;
    }
  }

  if (IsKeyPressed(KEY_E))
  {
    for (int i = 0; i < level1.qtdSpawners; i++)
    {
      if (CheckCollisionRecs(player->rect, level1.lineSpawners[i].rect))
      {
        if (ColorIsEqual(level1.lineSpawners[i].color, RED))
        {
          player->selectedColor = 1;
        }

        if (ColorIsEqual(level1.lineSpawners[i].color, GREEN))
        {
          player->selectedColor = 2;
        }

        if (ColorIsEqual(level1.lineSpawners[i].color, BLUE))
        {
          player->selectedColor = 3;
        }

        if (player->selectedColor != 0 && !level1.lineSpawners[i].activated)
        {
          level1.lineSpawners[i].activated = true;
          NewLinePoint((Vector2){level1.lineSpawners[i].rect.x + level1.lineSpawners[i].rect.width / 2, level1.lineSpawners[i].rect.y + level1.lineSpawners[i].rect.height / 2}, player, envItems, envItemsLength);
          break;
        }
      }
    }
  }

  if (IsKeyPressed(KEY_C))
  {
    bool setGoal = false;
    for (int i = 0; i < level1.qtdGoals; i++)
    {
      LineRecColisions colisions;
      if ((player->selectedColor == 1 && ColorIsEqual(level1.goals[i].color, RED) && !CheckLineEnvColision((Line){player->redLine.points[player->redLine.last], playerCenter}, envItems, envItemsLength, &colisions)) ||
          (player->selectedColor == 2 && ColorIsEqual(level1.goals[i].color, GREEN) && !CheckLineEnvColision((Line){player->greenLine.points[player->greenLine.last], playerCenter}, envItems, envItemsLength, &colisions)) ||
          (player->selectedColor == 3 && ColorIsEqual(level1.goals[i].color, BLUE) && !CheckLineEnvColision((Line){player->blueLine.points[player->blueLine.last], playerCenter}, envItems, envItemsLength, &colisions)))
      {
        if (CheckCollisionRecs(player->rect, level1.goals[i].rect))
        {
          level1.goals[i].isSet = true;
          NewLinePoint((Vector2){level1.goals[i].rect.x + level1.goals[i].rect.width / 2, level1.goals[i].rect.y + level1.goals[i].rect.height / 2}, player, envItems, envItemsLength);
          setGoal = true;
          player->selectedColor = 0;
        }
      }
    }

    if (!setGoal)
    {
      NewLinePoint(playerCenter, player, envItems, envItemsLength);
    }
  }

  if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
    player->position.x -= PLAYER_HOR_SPD * delta;
  if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
    player->position.x += PLAYER_HOR_SPD * delta;
  if (IsKeyDown(KEY_SPACE) && player->canJump)
  {
    player->speed = -PLAYER_JUMP_SPD;
    player->canJump = false;
  }

  bool hitObstacle = false;
  for (int i = 0; i < envItemsLength; i++)
  {
    EnvItem const *ei = envItems + i;
    Vector2 *p = &(player->position);
    if (ei->blocking && ei->rect.x <= p->x &&
        ei->rect.x + ei->rect.width >= p->x && ei->rect.y >= p->y &&
        ei->rect.y <= p->y + player->speed * delta)
    {
      hitObstacle = true;
      player->speed = 0.0f;
      p->y = ei->rect.y;
      break;
    }
  }

  if (!hitObstacle)
  {
    player->position.y += player->speed * delta;
    player->speed += G * delta;
    player->canJump = false;
  }
  else
    player->canJump = true;
}

void NewLinePoint(Vector2 lineEndPoint, Player *player, EnvItem *envItems, int envItemsLength)
{
  printf("NewLinePoint\n");
  LineRecColisions colisions;

  switch (player->selectedColor)
  {
  case 1:
    printf("selectedColor = 1\n");
    if (player->redLine.last < player->redLine.capacity - 1 && (player->redLine.last == -1 || !CheckLineEnvColision((Line){player->redLine.points[player->redLine.last], lineEndPoint}, envItems, envItemsLength, &colisions)))
    {
      player->redLine.points[player->redLine.last + 1] = lineEndPoint;
      player->redLine.last++;
    }
    break;
  case 2:
    printf("selectedColor = 2\n");
    if (player->greenLine.last < player->greenLine.capacity - 1 && (player->greenLine.last == -1 || !CheckLineEnvColision((Line){player->greenLine.points[player->greenLine.last], lineEndPoint}, envItems, envItemsLength, &colisions)))
    {
      player->greenLine.points[player->greenLine.last + 1] = lineEndPoint;
      player->greenLine.last++;
    }
    break;
  case 3:
    printf("selectedColor = 3\n");
    if (player->blueLine.last < player->blueLine.capacity - 1 && (player->blueLine.last == -1 || !CheckLineEnvColision((Line){player->blueLine.points[player->blueLine.last], lineEndPoint}, envItems, envItemsLength, &colisions)))
    {
      player->blueLine.points[player->blueLine.last + 1] = lineEndPoint;
      player->blueLine.last++;
    }
    break;

  default:
    break;
  }
}

void UpdateCameraCenterInsideMap(Camera2D *camera, Player *player,
                                 EnvItem *envItems, int envItemsLength,
                                 float delta, int width, int height)
{
  camera->target = player->position;
  camera->offset = (Vector2){(float)width / 2.0f, (float)height / 2.0f};
  float minX = 1000;
  float minY = 1000;
  float maxX = -1000;
  float maxY = -1000;

  for (int i = 0; i < envItemsLength; i++)
  {
    EnvItem const *ei = envItems + i;
    minX = fminf(ei->rect.x, minX);
    maxX = fmaxf(ei->rect.x + ei->rect.width, maxX);
    minY = fminf(ei->rect.y, minY);
    maxY = fmaxf(ei->rect.y + ei->rect.height, maxY);
  }

  Vector2 max = GetWorldToScreen2D((Vector2){maxX, maxY}, *camera);
  Vector2 min = GetWorldToScreen2D((Vector2){minX, minY}, *camera);

  if (max.x < (float)width)
    camera->offset.x = (float)width - (max.x - (float)width / 2.0f);
  if (max.y < (float)height)
    camera->offset.y = (float)height - (max.y - (float)height / 2.0f);
  if (min.x > 0)
    camera->offset.x = (float)width / 2.0f - min.x;
  if (min.y > 0)
    camera->offset.y = (float)height / 2.0f - min.y;
}

bool ColorIsEqual(Color col1, Color col2)
{
  bool result = false;

  if ((col1.r == col2.r) && (col1.g == col2.g) && (col1.b == col2.b) && (col1.a == col2.a))
    result = true;

  return result;
}

bool CheckLineRecColision(Line line, Rectangle rec, LineRecColisions *collisionPoints)
{
  bool colision = false;

  float recX1 = rec.x;
  float recX2 = rec.x + rec.width;
  float recY1 = rec.y;
  float recY2 = rec.y + rec.height;

  colision = colision || CheckCollisionLines(line.start, line.end, (Vector2){recX1, recY1}, (Vector2){recX1, recY2}, &collisionPoints->leftColisionPoint);
  colision = colision || CheckCollisionLines(line.start, line.end, (Vector2){recX2, recY1}, (Vector2){recX2, recY2}, &collisionPoints->rightColisionPoint);
  colision = colision || CheckCollisionLines(line.start, line.end, (Vector2){recX1, recY1}, (Vector2){recX2, recY1}, &collisionPoints->topColisionPoint);
  colision = colision || CheckCollisionLines(line.start, line.end, (Vector2){recX1, recY2}, (Vector2){recX2, recY2}, &collisionPoints->bottomColisionPoint);

  printf("colision = %d\n", colision);

  return colision;
}

bool CheckLineEnvColision(Line line, EnvItem *envItems, int qtdEnvItems, LineRecColisions *collisionPoints)
{
  bool colision = false;

  for (int i = 0; i < qtdEnvItems; i++)
  {
    if (envItems[i].blocking)
    {
      colision = colision || CheckLineRecColision(line, envItems[i].rect, collisionPoints);
    }
  }

  return colision;
}

Vector2 GetLineEnvItemClosestColisionVector2(Line line, EnvItem *envItems, int qtdEnvItems)
{
  Vector2 lineEndPoint = line.end;

  for (int i = 0; i < qtdEnvItems; i++)
  {
    if (envItems[i].blocking)
    {
      Rectangle rec = envItems[i].rect;
      LineRecColisions colisions;

      if (CheckLineRecColision(line, rec, &colisions))
      {
        printf("leftColisionPoint = (%f, %f)\n", colisions.leftColisionPoint.x, colisions.leftColisionPoint.y);
        printf("rightColisionPoint = (%f, %f)\n", colisions.rightColisionPoint.x, colisions.rightColisionPoint.y);
        printf("topColisionPoint = (%f, %f)\n", colisions.topColisionPoint.x, colisions.topColisionPoint.y);
        printf("bottomColisionPoint = (%f, %f)\n", colisions.bottomColisionPoint.x, colisions.bottomColisionPoint.y);

        float leftDistance = Vector2Distance(line.start, colisions.leftColisionPoint);
        float rightDistance = Vector2Distance(line.start, colisions.rightColisionPoint);
        float topDistance = Vector2Distance(line.start, colisions.topColisionPoint);
        float bottomDistance = Vector2Distance(line.start, colisions.bottomColisionPoint);

        float minDistance1 = fminf(leftDistance, rightDistance);
        float minDistance2 = fminf(topDistance, bottomDistance);
        float minDistance = fminf(minDistance1, minDistance2);

        if (minDistance < Vector2Distance(line.start, lineEndPoint))
        {
          if (FloatEquals(minDistance, leftDistance))
          {
            printf("left\n");
            lineEndPoint = colisions.leftColisionPoint;
          }
          else if (FloatEquals(minDistance, rightDistance))
          {
            printf("right\n");
            lineEndPoint = colisions.rightColisionPoint;
          }
          else if (FloatEquals(minDistance, topDistance))
          {
            printf("top\n");
            lineEndPoint = colisions.topColisionPoint;
          }
          else if (FloatEquals(minDistance, bottomDistance))
          {
            printf("bottom\n");
            lineEndPoint = colisions.bottomColisionPoint;
          }
        }
      }
    }
  }

  return lineEndPoint;
}

void DrawClampedLine(float x1, float y1, float x2, float y2, float length, Color color)
{
  // Calculate the difference in x and y between the points
  float dx = x2 - x1;
  float dy = y2 - y1;

  // Calculate the distance between the points
  float distance = sqrtf(dx * dx + dy * dy);

  // Avoid division by zero in case the points are the same
  if (distance == 0)
  {
    return;
  }

  if (distance > length)
  {
    // Normalize the direction vector and scale it by the desired length
    float scale = length / distance;
    float clampedX = x1 + dx * scale;
    float clampedY = y1 + dy * scale;

    // Call the function to draw the line with clamped endpoint
    DrawLineEx((Vector2){x1, y1}, (Vector2){clampedX, clampedY}, 2.0f, color);
  }
  else
  {
    DrawLineEx((Vector2){x1, y1}, (Vector2){x2, y2}, 2.0f, color);
  }
}
