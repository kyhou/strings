#include "raylib.h"
#include "raymath.h"

#include "shapes_helpers.h"
#include "level.h"

#include <math.h>
#include <stdio.h>
#include <stdbool.h>

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