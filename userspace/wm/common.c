#include "common.h"

int rect_contains_point(Rect* rect, Point* point){
    return point->x >= rect->x 
        && point->x < rect->x + rect->w
        && point->y >= rect->y
        && point->y < rect->y + rect->h;
}
