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
typedef enum
{
  SCREEN_LOGO = 0,
  SCREEN_TITLE,
  SCREEN_GAMEPLAY,
  SCREEN_ENDING
} GameScreen;

typedef struct Points
{
  int capacity;
  int last;
  Vector2 *points;
} Points;

typedef struct Player
{
  Vector2 position;
  float speed;
  bool canJump;
  Points redLine;
  Points greenLine;
  Points blueLine;
  int selectedColor;
} Player;

// TODO: create 1 line liked list for each type of line

typedef struct EnvItem
{
  Rectangle rect;
  int blocking;
  Color color;
} EnvItem;

typedef struct Goal
{
  Rectangle rect;
  Color color;
  bool isSet;
} Goal;

// TODO: Define your custom data types here

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

static RenderTexture2D target = {.id = 0}; // Render texture to render our game

static Player player = {0};
static Camera2D camera = {0};
static EnvItem envItems[] = {{{0, 0, 1000, 400}, 0, LIGHTGRAY},
                             {{0, 400, 1000, 200}, 1, GRAY},
                             {{300, 200, 400, 10}, 1, GRAY},
                             {{250, 300, 100, 10}, 1, GRAY},
                             {{650, 300, 100, 10}, 1, GRAY}};
static int envItemsLength;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void); // Update and Draw one frame
void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength,
                  float delta);
void UpdateCameraCenterInsideMap(Camera2D *camera, Player *player,
                                 EnvItem *envItems, int envItemsLength,
                                 float delta, int width, int height);

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

  envItemsLength = sizeof(envItems) / sizeof(envItems[0]);

  camera.target = player.position;
  camera.offset = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
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
void UpdateDrawFrame(void)
{
  // Update
  //----------------------------------------------------------------------------------
  float deltaTime = GetFrameTime();

  UpdatePlayer(&player, envItems, envItemsLength, deltaTime);

  camera.zoom += ((float)GetMouseWheelMove() * 0.05f);

  if (camera.zoom > 3.0f)
    camera.zoom = 3.0f;
  else if (camera.zoom < 0.25f)
    camera.zoom = 0.25f;

  if (IsKeyPressed(KEY_R))
  {
    camera.zoom = 1.0f;
    player.position = (Vector2){400, 280};
  }
  UpdateCameraCenterInsideMap(&camera, &player, envItems, envItemsLength,
                              deltaTime, screenWidth, screenHeight);

  //----------------------------------------------------------------------------------

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
  for (int i = 0; i < envItemsLength; i++)
  {
    DrawRectangleRec(envItems[i].rect, envItems[i].color);
  }

  Rectangle recTeste = {
      .x = 600,
      .y = 300,
      .width = 100,
      .height = 100};
  DrawRectangleRec(recTeste, BLUE);

  // char *playerX;
  // asprintf(&playerX, "x = %d\n", player.position.x);
  // char *playerY;
  // asprintf(&playerY, "y = %d\n", player.position.y);

  // DrawText(playerX, 150, 140, 30, BLACK);
  // DrawText(playerY, 150, 180, 30, BLACK);

  Rectangle playerRect = {player.position.x - 20, player.position.y - 40, 40.0f,
                          40.0f};
  DrawRectangleRec(playerRect, RED);

  // DrawCircleV(player.position, 5.0f, GOLD);

  switch (player.selectedColor)
  {
    case 1:
      DrawText("Red", 150, 140, 30, BLACK);
      break;
    case 2:
      DrawText("Green", 150, 140, 30, BLACK);
      break;
    case 3:
      DrawText("Blue", 150, 140, 30, BLACK);
      break;

    default:
      break;
  }

  for (int i = 0; i <= player.redLine.last; i++)
  {
    DrawCircle(player.redLine.points[i].x, player.redLine.points[i].y, 5.0f, GOLD);

    if (!CheckCollisionPointRec((Vector2){player.redLine.points[i].x, player.redLine.points[i].y}, recTeste))
    {
      if (i == player.redLine.last && i < player.redLine.capacity - 1 && player.selectedColor == 1)
      {
        DrawClampedLine(player.redLine.points[i].x, player.redLine.points[i].y, player.position.x, player.position.y, 500, RED);
      }
      else if (i < player.redLine.last)
      {
        DrawClampedLine(player.redLine.points[i].x, player.redLine.points[i].y, player.redLine.points[i + 1].x, player.redLine.points[i + 1].y, 500, RED);
      }
    }
  }

  for (int i = 0; i <= player.greenLine.last; i++)
  {
    DrawCircle(player.greenLine.points[i].x, player.greenLine.points[i].y, 5.0f, GOLD);

    if (!CheckCollisionPointRec((Vector2){player.greenLine.points[i].x, player.greenLine.points[i].y}, recTeste))
    {
      if (i == player.greenLine.last && i < player.greenLine.capacity - 1 && player.selectedColor == 2)
      {
        DrawClampedLine(player.greenLine.points[i].x, player.greenLine.points[i].y, player.position.x, player.position.y, 500, GREEN);
      }
      else if (i < player.greenLine.last)
      {
        DrawClampedLine(player.greenLine.points[i].x, player.greenLine.points[i].y, player.greenLine.points[i + 1].x, player.greenLine.points[i + 1].y, 500, GREEN);
      }
    }
  }

  for (int i = 0; i <= player.blueLine.last; i++)
  {
    DrawCircle(player.blueLine.points[i].x, player.blueLine.points[i].y, 5.0f, GOLD);

    if (!CheckCollisionPointRec((Vector2){player.blueLine.points[i].x, player.blueLine.points[i].y}, recTeste))
    {
      if (i == player.blueLine.last && i < player.blueLine.capacity - 1 && player.selectedColor == 3)
      {
        DrawClampedLine(player.blueLine.points[i].x, player.blueLine.points[i].y, player.position.x, player.position.y, 500, BLUE);
      }
      else if (i < player.blueLine.last)
      {
        DrawClampedLine(player.blueLine.points[i].x, player.blueLine.points[i].y, player.blueLine.points[i + 1].x, player.blueLine.points[i + 1].y, 500, BLUE);
      }
    }
  }
  // DrawLine(player.position.x, player.position.y, player.position.x,
  // player.position.y + playerRect.height, RED);

  EndMode2D();
  EndTextureMode();

  // Render to screen (main framebuffer)
  BeginDrawing();
  ClearBackground(RAYWHITE);

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

void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength,
                  float delta)
{
  if (IsKeyPressed(KEY_ONE))
  {
    player->selectedColor = 1;
  }
  if (IsKeyPressed(KEY_TWO))
  {
    player->selectedColor = 2;
  }
  if (IsKeyPressed(KEY_THREE))
  {
    player->selectedColor = 3;
  }
  if (IsKeyPressed(KEY_Q))
  {
    player->selectedColor = 0;
  }

  if (IsKeyPressed(KEY_C))
  {
    switch (player->selectedColor)
    {
    case 1:
      if (player->redLine.last < player->redLine.capacity - 1)
      {
        player->redLine.points[player->redLine.last + 1] = player->position; // = realloc(playerLines, (playerLines + 1) * sizeof(Line));
        player->redLine.last++;
      }
      break;
    case 2:
      if (player->greenLine.last < player->greenLine.capacity - 1)
      {
        player->greenLine.points[player->greenLine.last + 1] = player->position; // = realloc(playerLines, (playerLines + 1) * sizeof(Line));
        player->greenLine.last++;
      }
      break;
    case 3:
      if (player->blueLine.last < player->blueLine.capacity - 1)
      {
        player->blueLine.points[player->blueLine.last + 1] = player->position; // = realloc(playerLines, (playerLines + 1) * sizeof(Line));
        player->blueLine.last++;
      }
      break;

    default:
      break;
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
    EnvItem *ei = envItems + i;
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

void UpdateCameraCenterInsideMap(Camera2D *camera, Player *player,
                                 EnvItem *envItems, int envItemsLength,
                                 float delta, int width, int height)
{
  camera->target = player->position;
  camera->offset = (Vector2){width / 2.0f, height / 2.0f};
  float minX = 1000, minY = 1000, maxX = -1000, maxY = -1000;

  for (int i = 0; i < envItemsLength; i++)
  {
    EnvItem *ei = envItems + i;
    minX = fminf(ei->rect.x, minX);
    maxX = fmaxf(ei->rect.x + ei->rect.width, maxX);
    minY = fminf(ei->rect.y, minY);
    maxY = fmaxf(ei->rect.y + ei->rect.height, maxY);
  }

  Vector2 max = GetWorldToScreen2D((Vector2){maxX, maxY}, *camera);
  Vector2 min = GetWorldToScreen2D((Vector2){minX, minY}, *camera);

  if (max.x < width)
    camera->offset.x = width - (max.x - width / 2);
  if (max.y < height)
    camera->offset.y = height - (max.y - height / 2);
  if (min.x > 0)
    camera->offset.x = width / 2 - min.x;
  if (min.y > 0)
    camera->offset.y = height / 2 - min.y;
}

// Function to draw a line of fixed length from position1 to a point along the
// direction to position2
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
