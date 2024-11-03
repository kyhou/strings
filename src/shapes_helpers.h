#ifndef shapes_helpers // guardas de cabeçalho, impedem inclusões cíclicas
#define shapes_helpers

#include "raylib.h"
#include "raymath.h"

#include "level.h"

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

bool CheckLineRecColision(Line line, Rectangle rec, LineRecColisions *collisionPoints);
bool CheckLineEnvColision(Line line, EnvItem *envItems, int qtdEnvItems, LineRecColisions *collisionPoints);
Vector2 GetLineEnvItemClosestColisionVector2(Line line, EnvItem *envItems, int qtdEnvItems);
bool lineLine(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, Vector2 *intersectionPoint);

#endif