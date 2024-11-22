#include <ncurses.h>
#ifndef STRUCTS
#define STRUCTS

typedef enum
{
    LEFT,
    RIGHT,
} DIR;

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
    DIR dir;
    char headShapeLeft;
    char headShapeRight;
    char bodyShape;
    bool isBouncing;
    bool isHidden;
} CAR;

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