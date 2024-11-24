#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "structs.h"
#define GREEN_COLOR 1
#define RED_COLOR 2
#define BLUE_COLOR 3
#define BLACK_COLOR 4
#define WHITE_COLOR 5

#define MAP_HEIGHT 35
#define MAP_WIDTH 30

#define STATUS_HEIGHT 5
#define STATUS_Y MAP_HEIGHT

#define FRAME_TIME 16.66 // time interval between frames in ms
#define FBM_PLAYER 12    // frames between movement | FBM_PLAYER*FRAME_TIME => How often can player move

#define SLOW_CAR 10
#define FAST_CAR 5
#define SUPERFAST_CAR 3

#define RA(min, max) ( (min) + rand() % ((max) - (min) + 1) )	

//------------------------------------------------
//----------------- INIT FUNCTIONS ---------------
//------------------------------------------------

// function initializng main window
WINDOW *InitGame()
{
    WINDOW *stdwin;
    stdwin = initscr();

    start_color();
    init_pair(GREEN_COLOR, COLOR_WHITE, COLOR_GREEN);
    init_pair(RED_COLOR, COLOR_WHITE, COLOR_RED);
    init_pair(BLUE_COLOR, COLOR_WHITE, COLOR_BLUE);
    init_pair(BLACK_COLOR, COLOR_WHITE, COLOR_BLACK);
    init_pair(WHITE_COLOR, COLOR_BLACK, COLOR_WHITE);

    srand(time(NULL));
    cbreak();
    curs_set(0);
    noecho();
    return stdwin;
}

PLAYER *InitPlayer(int y, int x, int minY, int maxY, int minX, int maxX, char shape)
{
    PLAYER *tempPlayer = (PLAYER *)malloc(sizeof(PLAYER));
    tempPlayer->y = y;
    tempPlayer->x = x;

    // min and max of frogs position
    tempPlayer->frame = 1;
    tempPlayer->minY = minY;
    tempPlayer->maxY = maxY;
    tempPlayer->maxX = maxX;
    tempPlayer->minX = minX;
    tempPlayer->shape = shape;
    return tempPlayer;
}

CAR *InitCar(int color, int length, int headX, DIR dir, char headShapeLeft, char headShapeRight, char bodyShape, bool isBouncing, bool exists, SPEED speed)
{
    CAR *tempCar = (CAR *)malloc(sizeof(CAR));
    tempCar->color = color;
    tempCar->length = length;
    if (dir == RIGHT)
    {
        tempCar->rightX = headX;
        tempCar->leftX = headX - length + 1;
    }
    else
    {
        tempCar->leftX = headX;
        tempCar->rightX = headX + length - 1;
    }
    tempCar->frame = 1;
    switch (speed)
    {
    case 0:
        tempCar->speed = SUPERFAST_CAR;
        break;
    case 1:
        tempCar->speed = FAST_CAR;
        break;
    case 2:
        tempCar->speed = SLOW_CAR;
        break;
    }
    tempCar->dir = dir;
    tempCar->headShapeLeft = headShapeLeft;
    tempCar->headShapeRight = headShapeRight;
    tempCar->bodyShape = bodyShape;
    tempCar->isBouncing = isBouncing;
    tempCar->exists = exists;
    return tempCar;
}
// function initializng a subwindow
WIN *InitWindow(WINDOW *parent, int width, int height, int y, int x, char *title)
{
    WIN *tempWin = (WIN *)malloc(sizeof(WIN));
    tempWin->height = height;
    tempWin->width = width;
    tempWin->x = x;
    tempWin->y = y;
    tempWin->win = subwin(parent, height, width, y, x);
    box(tempWin->win, 0, 0);
    mvwprintw(tempWin->win, 0, 3, title);
    return tempWin;
}

LANE *InitLanes()
{
    LANE *lanes = (LANE *)malloc(sizeof(LANE) * (MAP_HEIGHT - 4));
    for (int i = 0; i <= MAP_HEIGHT - 4; i++)
    {
        lanes[i].car = InitCar(rand() % 2 + 2, rand() % 5 + 5, rand() % MAP_WIDTH, rand() % 2, '<', '>', 'X', rand() % 2, rand() % 3, rand() % 3);
    }
    return lanes;
}

TIMER *InitTimer()
{
    TIMER *timer = (TIMER *)malloc(sizeof(TIMER));
    timer->frame_no = 1;
    timer->time_elapsed = 0;
    return timer;
}

//------------------------------------------------
//---------------- DRAW FUNCTIONS ----------------
//------------------------------------------------

// checking if mvwaddch x is inside our window
void MvAddCharCheck(WINDOW *win, int y, int x, char ch)
{
    if (x > 0 && x < MAP_WIDTH - 1)
    {
        mvwaddch(win, y, x, ch);
    }
}

void DrawPlayer(WINDOW *win, PLAYER *player)
{
    wattron(win, COLOR_PAIR(GREEN_COLOR) | A_BOLD);
    mvwaddch(win, player->y, player->x, player->shape);
    wattroff(win, COLOR_PAIR(GREEN_COLOR) | A_BOLD);
}

void DrawCar(CAR *car, WINDOW *win, int y)
{
    if (car->exists)
    {
        wattron(win, COLOR_PAIR(car->color));
        if (car->dir == RIGHT)
        {
            int x = car->rightX;
            MvAddCharCheck(win, y, x, car->headShapeRight);
            for (x = x - 1; x >= car->leftX; x--)
            {
                MvAddCharCheck(win, y, x, car->bodyShape);
            }
        }
        else
        {
            int x = car->leftX;
            MvAddCharCheck(win, y, x, car->headShapeLeft);
            for (x = x + 1; x <= car->rightX; x++)
            {
                MvAddCharCheck(win, y, x, car->bodyShape);
            }
        }
        wattroff(win, COLOR_PAIR(car->color));
    }
}

//------------------------------------------------
//---------------- OTHER FUNCTIONS ---------------
//------------------------------------------------

void CleanWin(WIN *window)
{
    for (int i = 1; i < window->height - 1; i++)  // Start from 1 to avoid clearing the border
    {
        for (int j = 1; j < window->width - 1; j++)  // Start from 1 to avoid clearing the border
        {
            mvwaddch(window->win, i, j, ' ');
        }
    }
}


void UpdateTimer(TIMER *timer)
{
    usleep(FRAME_TIME * 1000); // program sleeps FRAME_TIME[ms] between frames
    timer->frame_no++;
    timer->time_elapsed += FRAME_TIME;
}

void UpdateStatus(WINDOW *status, TIMER *timer)
{
    mvwprintw(status, STATUS_HEIGHT / 2, MAP_WIDTH / 2 - 10, "Time elapsed: %.2fs", timer->time_elapsed / 1000);
    wrefresh(status);
}

void PlayerMovement(WINDOW *win, PLAYER *player, char ch, TIMER *timer)
{
    if (timer->frame_no - player->frame >= FBM_PLAYER)
    {
        if (ch == 'w' && player->y > player->minY)
        {
            player->y -= 1;
        }
        else if (ch == 's' && player->y < player->maxY)
        {
            player->y += 1;
        }
        else if (ch == 'a' && player->x > player->minX)
        {
            player->x -= 1;
        }
        else if (ch == 'd' && player->x < player->maxX)
        {
            player->x += 1;
        }
        player->frame = timer->frame_no;
    }
    flushinp();
}

void MoveCar(CAR *car, TIMER *timer, WIN *win, int y)
{
    if (timer->frame_no - car->frame >= car->speed)
    {
        if (car->dir == LEFT)
        {
            if (car->isBouncing && car->leftX == win->x + 1)
            {
                car->dir = RIGHT;
            }
            else
            {
                car->leftX -= 1;
                car->rightX -= 1;
            }
        }
        else if (car->dir == RIGHT)
        {
            if (car->isBouncing && car->rightX == win->width - 2)
            {
                car->dir = LEFT;
            }
            car->leftX += 1;
            car->rightX += 1;
        }
        car->frame = timer->frame_no;
    }
}
void Welcome(WINDOW *win)
{
    mvwprintw(win, 2, 2, "Press any key to start the game");
    wgetch(win);
    wclear(win);
    wrefresh(win);
}

bool CheckCarCollision(PLAYER *player, LANE *lanes,int laneY)
{
    if (player->y < MAP_HEIGHT - 2 && player->y > 1 && lanes[laneY].car->exists && player->x >= lanes[laneY].car->leftX && player->x <= lanes[laneY].car->rightX)
    {
        return true;
    }
    return false;
}

void GameOver(WIN*status){
    CleanWin(status);
    wattron(status->win,A_BOLD);
    mvwprintw(status->win, STATUS_HEIGHT/2,MAP_WIDTH/2-4 , "GAME OVER");
    wattroff(status->win,A_BOLD);
    wrefresh(status->win);
    sleep(3);
}
 
 int MainLoop(PLAYER*player,WIN*map,WIN*status,TIMER*timer,LANE*lanes)
 {
    char ch;
    while ((ch = wgetch(map->win)) != 'x' && !CheckCarCollision(player, lanes,player->y-2))
    {
        CleanWin(map);

        if (ch != ERR)
        {
            PlayerMovement(map->win, player, ch, timer);
        }
        for (int i = 0; i < MAP_HEIGHT - 4; i++)
        {
            MoveCar(lanes[i].car, timer, map, i + 2);
            DrawCar(lanes[i].car, map->win, i + 2);
        }
        DrawPlayer(map->win, player);
        wrefresh(map->win);
        UpdateTimer(timer);
        UpdateStatus(status->win, timer);
    };
 }

int main()
{
    // initializing stuff
    WINDOW *stdwin = InitGame();
    Welcome(stdwin);

    WIN *map = InitWindow(stdwin, MAP_WIDTH, MAP_HEIGHT, 0, 0, "frogger");
    nodelay(map->win, true);

    WIN *status = InitWindow(stdwin, MAP_WIDTH, STATUS_HEIGHT, STATUS_Y, 0, "status");

    PLAYER *player = InitPlayer(MAP_HEIGHT - 2, MAP_WIDTH / 2, 1, MAP_HEIGHT - 2, 1, MAP_WIDTH - 2, 'O');

    TIMER *timer = InitTimer();

    // basic gameloop
    LANE *lanes = InitLanes();

    MainLoop(player,map,status,timer,lanes);
    GameOver(status);
    endwin();
    
    return 0;
}
