#include "raylib.h"
#include "math.h"

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
