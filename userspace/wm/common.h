#ifndef _COMMON_H_
#define _COMMON_H_

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} Rect;

/**
 * Return 1 if a point is inside a rectangle
 **/
int rect_contains_point(Rect* rect, Point* point);

#endif
