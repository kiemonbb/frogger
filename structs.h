#include <ncurses.h>
#ifndef STRUCTS
#define STRUCTS

typedef enum
{
    LEFT = 0,
    RIGHT = 1,
} DIR;

typedef enum
{
    RIVER,
    TWO_ROCKS,
    PONDS,
    BIG_ROCK,
} OBSTACLE_TYPE;

typedef enum
{
    SUPERFAST = 0,
    FAST = 1,
    SLOW = 2,
} SPEED;

typedef enum
{
    BOUNCING = 0,
    WRAPPING = 1,
    DISAPPEARING = 2,
} CARTYPE;
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
    bool isFriendly;
    bool exists;
    CARTYPE carType;
} CAR;

typedef struct
{
    int color;
    int *positions;
} OBSTACLE;

typedef struct
{
    CAR *car;
    OBSTACLE *obstacle;
} LANE;

typedef struct
{
    int x, y;
    int color;
    int frame;
    int maxX, maxY, minX, minY;
    int height, width;
    bool travels; // does player want to travel on a friendly car
    char shape;
} PLAYER;

typedef struct
{
    int frame_no;
    int points;
    int gameTime;
    float timeElapsed;
    float timeLeft;
} TIMER;

#endif