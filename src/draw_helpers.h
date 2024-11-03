#ifndef draw_helpers // guardas de cabeçalho, impedem inclusões cíclicas
#define draw_helpers

#include "raylib.h"

// Function to draw a line of fixed length from position1 to a point along the
// direction to position2
void DrawClampedLine(float x1, float y1, float x2, float y2, float length, Color color);

#endif