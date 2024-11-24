#include <ncurses.h>
#ifndef STRUCTS
#define STRUCTS

typedef enum
{
    LEFT = 0,
    RIGHT = 1,
} DIR;

typedef enum{
    SUPERFAST = 0,
    FAST = 1,
    SLOW = 2,
}SPEED;

typedef struct
{
    WINDOW *win;
    int height, width, x, y;
} WIN;

typedef struct
{
    int color;
    int length;
    int leftX;
    int rightX;
    int frame;
    int speed;
    DIR dir;
    char headShapeLeft;
    char headShapeRight;
    char bodyShape;
    bool exists;
    bool isBouncing;
} CAR;

typedef struct{
    CAR*car;
} LANE;

typedef struct
{
    int x, y;
    int color;
    int frame;
    int maxX, maxY, minX, minY;
    int height, width;
    char shape;
} PLAYER;

typedef struct {
	int frame_no;
    float time_elapsed;
} TIMER;

#endif