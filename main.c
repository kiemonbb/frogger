#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "structs.h"
#define GREEN_COLOR 1
#define RED_COLOR 2
#define BLUE_COLOR 3

#define MAP_HEIGHT 35
#define MAP_WIDTH 100
#define MAP_X 0
#define MAP_Y 0

#define STATUS_HEIGHT 5
#define STATUS_X 0
#define STATUS_Y MAP_HEIGHT

#define FRAME_TIME 16.66 // time interval between frames in ms
#define FBM_PLAYER 10    // frames between movement | FBM_PLAYER*FRAME_TIME => How often can player move

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

CAR *InitCar(int color, int length, int headX, DIR dir, char headShapeLeft, char headShapeRight, char bodyShape, bool isBouncing, bool isVisible, SPEED speed)
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
    tempCar->speed = speed;
    tempCar->dir = dir;
    tempCar->headShapeLeft = headShapeLeft;
    tempCar->headShapeRight = headShapeRight;
    tempCar->bodyShape = bodyShape;
    tempCar->isBouncing = isBouncing;
    tempCar->isVisible = isVisible;
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
    tempWin->win = subwin(parent, width, height, y, x);
    box(tempWin->win, 0, 0);
    mvwprintw(tempWin->win, 0, 3, title);
    return tempWin;
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

// checking if mvaddch x is inside our window
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
    if (car != NULL && car->isVisible == TRUE)
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

void UpdateTimer(TIMER *timer)
{
    usleep(FRAME_TIME * 1000); // program sleeps FRAME_TIME[ms] between frames
    timer->frame_no++;
    timer->time_elapsed += FRAME_TIME;
}

void UpdateStatus(WINDOW *status, TIMER *timer)
{
    mvwprintw(status, STATUS_HEIGHT/2, MAP_WIDTH/2-10, "Time elapsed: %.2fs", timer->time_elapsed / 1000);
}

void PlayerMovement(WINDOW *win, PLAYER *player, char ch, TIMER *timer)
{
    if (timer->frame_no - player->frame >= FBM_PLAYER)
    {
        if (ch == 'w' && player->y > player->minY)
        {
            mvwaddch(win, player->y, player->x, ' ');
            player->y -= 1;
        }
        else if (ch == 's' && player->y < player->maxY)
        {
            mvwaddch(win, player->y, player->x, ' ');
            player->y += 1;
        }
        else if (ch == 'a' && player->x > player->minX)
        {
            mvwaddch(win, player->y, player->x, ' ');
            player->x -= 1;
        }
        else if (ch == 'd' && player->x < player->maxX)
        {
            mvwaddch(win, player->y, player->x, ' ');
            player->x += 1;
        }
        DrawPlayer(win, player);
        player->frame = timer->frame_no;
    }
    flushinp();
}

void MoveCar(CAR*car,TIMER*timer,WINDOW*win,int y)
{
    if (timer->frame_no - car->frame >= car->speed)
    {
        if(car->dir==LEFT){
            car->leftX-=1;
            car->rightX-=1;
        }
        if(car->dir==RIGHT){
            car->leftX+=1;
            car->rightX+=1;
        }
        DrawCar(car,win,y);
        car->frame = timer->frame_no;
    }
}

int main()
{
    // initializing stuff
    WINDOW *stdwin = InitGame();
    WIN *map = InitWindow(stdwin, MAP_HEIGHT, MAP_WIDTH, MAP_Y, MAP_X, "frogger");
    WIN *status = InitWindow(stdwin, STATUS_HEIGHT, MAP_WIDTH, STATUS_Y, STATUS_X, "status");

    PLAYER *player = InitPlayer(MAP_HEIGHT - 2, MAP_WIDTH / 2, 1, MAP_HEIGHT - 2, 1, MAP_WIDTH - 2, 'O');

    TIMER *timer = InitTimer();

    nodelay(map->win, true);

    // basic gameloop
    char ch;

    LANE * lanes = (LANE*)malloc(sizeof(LANE)*MAP_HEIGHT);
    for(int i = 2;i<MAP_HEIGHT-2;i++){
        lanes[i].car = InitCar(rand()%3+1,rand()%5+5,rand()%MAP_WIDTH,rand()%2,'<','>','X',FALSE,rand()%3,SUPERFAST);
        DrawCar(lanes[i].car,map->win,i);
    }

    DrawPlayer(map->win, player);
    while ((ch = wgetch(map->win)) != 'x')
    {
        if (ch != ERR)
        {
            PlayerMovement(map->win, player, ch, timer);
        }
        wrefresh(status->win);
        wrefresh(map->win);
        UpdateTimer(timer);
        UpdateStatus(status->win, timer);
    }

    endwin();
    return 0;
}