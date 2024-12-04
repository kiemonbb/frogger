#include <ncurses.h>
#ifndef STRUCTS
#define STRUCTS

typedef enum
{
    LEFT,
    RIGHT,
} DIR;

typedef enum
{
    RIVER,
    TWO_ROCKS,
    BIG_ROCK,
} OBSTACLE_TYPE;

typedef enum
{
    SUPERFAST,
    FAST,
    SLOW,
} SPEED;

typedef enum
{
    BOUNCING,
    WRAPPING,
    DISAPPEARING,
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
    bool stops;
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
    OBSTACLE_TYPE type;
    int color;
    int *positions;
    bool exists;
} OBSTACLE;

typedef struct
{
    CAR *car;
    OBSTACLE *obstacle;
} LANE;

typedef struct
{
    int x, y, prevX, prevY;
    int color;
    int frame;
    int maxX, maxY, minX, minY;
    bool travels; // does player want to travel on a friendly car
    char shape;
} PLAYER;

typedef struct
{
    int x,y;
    int color;
    int frame;
    char shape;
    bool exists;
}STORK;
typedef struct
{
    int frame_no;
    int points;
    int gameTime;
    float timeLeft;
    int end;
} TIMER;

#endif