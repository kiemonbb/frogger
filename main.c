#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "structs.h"
#define GREEN_COLOR 1
#define RED_COLOR 2
#define BLUE_COLOR 3
#define BLACK_COLOR 4
#define WHITE_COLOR 5

#define MAP_HEIGHT 20
#define MAP_WIDTH 30

#define STATUS_HEIGHT 5

#define FRAME_TIME 16.66666 // time interval between frames in ms
#define FBM_PLAYER 12       // frames between movement | FBM_PLAYER*FRAME_TIME => How often can player move
#define GAME_TIME 30

#define MIN_CAR_LENGTH 6
#define MAX_CAR_LENGTH 10

#define END_POINTS  200   //points gained by reaching the end of map and making a single step forward
#define STEP_POINTS 5

#define SLOW_CAR 9
#define FAST_CAR 6
#define SUPERFAST_CAR 3

#define RA(min, max) ((min) + rand() % ((max) - (min) + 1))

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
    mvwprintw(tempWin->win, 0, 1, title);
    return tempWin;
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
    tempPlayer->travels = FALSE;
    tempPlayer->shape = shape;
    return tempPlayer;
}

CAR *InitCar(int length, int headX, DIR dir, char headShapeLeft, char headShapeRight, char bodyShape, CARTYPE carType, bool isFriendly, bool exists, SPEED speed)
{
    CAR *tempCar = (CAR *)malloc(sizeof(CAR));
    if (isFriendly)
    {
        tempCar->color = GREEN_COLOR;
    }
    else
    {
        tempCar->color = RED_COLOR;
    }
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
    tempCar->carType = carType;
    tempCar->isFriendly = isFriendly;
    tempCar->exists = exists;
    return tempCar;
}

LANE *InitLanes(WIN *map,int *settings)
{
    LANE *lanes = (LANE *)malloc(sizeof(LANE) * (map->height - 4));
    for (int i = 0; i < map->height - 4; i++)
    {
        lanes[i].car = InitCar(RA(settings[3], settings[4]), RA(3, map->width - 3), RA(0, 1), '<', '>', 'X', RA(0,2), !RA(0, 3), RA(0, 2), RA(0, 2));
    }
    return lanes;
}

TIMER *InitTimer(int gameTime)
{
    TIMER *timer = (TIMER *)malloc(sizeof(TIMER));
    timer->frame_no = 1;
    timer->timeElapsed = 0;
    timer->points = 0;
    timer->gameTime = gameTime;
    return timer;
}

//------------------------------------------------
//---------------- DRAW FUNCTIONS ----------------
//------------------------------------------------

// checking if mvwaddch x is inside our window
void MvAddCharCheck(WIN *map, int y, int x, char ch)
{
    if (x > 0 && x < map->width - 1)
    {
        mvwaddch(map->win, y, x, ch);
    }
}

void DrawPlayer(WINDOW *win, PLAYER *player)
{
    wattron(win, COLOR_PAIR(GREEN_COLOR) | A_BOLD);
    mvwaddch(win, player->y, player->x, player->shape);
    wattroff(win, COLOR_PAIR(GREEN_COLOR) | A_BOLD);
}

void DrawCar(CAR *car, WIN *map, int y)
{
    if (car->exists)
    {
        wattron(map->win, COLOR_PAIR(car->color));
        if (car->dir == RIGHT)
        {
            int x = car->rightX;
            MvAddCharCheck(map, y, x, car->headShapeRight);
            for (x = x - 1; x >= car->leftX; x--)
            {
                MvAddCharCheck(map, y, x, car->bodyShape);
            }
        }
        else
        {
            int x = car->leftX;
            MvAddCharCheck(map, y, x, car->headShapeLeft);
            for (x = x + 1; x <= car->rightX; x++)
            {
                MvAddCharCheck(map, y, x, car->bodyShape);
            }
        }
        wattroff(map->win, COLOR_PAIR(car->color));
    }
}

//------------------------------------------------
//---------------- OTHER FUNCTIONS ---------------
//------------------------------------------------

void CleanWin(WIN *window)
{
    for (int i = 1; i < window->height - 1; i++) // Start from 1 to avoid clearing the border
    {
        for (int j = 1; j < window->width - 1; j++) // Start from 1 to avoid clearing the border
        {
            mvwaddch(window->win, i, j, ' ');
        }
    }
}

void Welcome(WINDOW *win)
{
    mvwprintw(win, 1, 2, "   Use 'wasd' to move around    ");
    mvwprintw(win, 2, 2, "  Press 'x' to close the game   ");
    mvwprintw(win, 3, 2, "Press 'r' to drive on green cars");
    mvwprintw(win, 5, 2, "Press any key to start the game!");
    wgetch(win);
    wclear(win);
    wrefresh(win);
}

void GameOver(WIN *status,TIMER*timer)
{
    CleanWin(status);
    wattron(status->win, A_BOLD);
    mvwprintw(status->win, status->height / 2, status->width / 2 - 13, "GAME OVER  Your Score: %d",timer->points);
    wattroff(status->win, A_BOLD);
    wrefresh(status->win);
    sleep(3);
}

void Quit(WIN *status)
{
    CleanWin(status);
    wattron(status->win, A_BOLD);
    mvwprintw(status->win, status->height / 2, status->width / 2 - 4, "YOU QUIT");
    wattroff(status->win, A_BOLD);
    wrefresh(status->win);
    sleep(3);
}

void UpdateStatus(WIN *status, TIMER *timer)
{
    CleanWin(status);
    mvwprintw(status->win, status->height / 2, status->width / 2 - 13, "Time: %.2fs   Points: %d", timer->timeLeft, timer->points);
    wrefresh(status->win);
}

bool UpdateTimer(TIMER *timer, time_t startFrame, time_t endFrame)
{
    usleep((FRAME_TIME - (startFrame - endFrame)) * 1000);                 // program sleeps FRAME_TIME[ms] - time it takes to generate a single frame, between frames
    timer->frame_no++;                                     
    timer->timeLeft = timer->gameTime - FRAME_TIME*timer->frame_no/1000;   // in seconds
    if (timer->timeLeft <= 0)
    {
        return TRUE;
    }
    return FALSE;
}

//------------------------------------------------
//--------------- MOVEMENT FUNCTIONS -------------
//------------------------------------------------

void PlayerMovement(WINDOW *win, PLAYER *player, char ch, TIMER *timer)
{
    if (timer->frame_no - player->frame >= FBM_PLAYER)
    {
        if (ch == 'w' && player->y > player->minY)
        {
            player->y -= 1;
            timer->points+=STEP_POINTS;
        }
        else if (ch == 's' && player->y < player->maxY)
        {
            player->y += 1;
            timer->points-=STEP_POINTS;
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
}

bool CheckCarCollision(PLAYER *player, CAR *car, WIN *map)
{
    if (player->y > 1 && player->y < map->height - 2 && car->exists && player->x >= car->leftX && player->x <= car->rightX)
    {
        return true;
    }
    return false;
}

bool MoveBouncingCar(CAR *car, WIN *map)
{
    if (car->carType == BOUNCING)
    {
        if (car->dir == LEFT && car->leftX == 1)
        {
            car->dir = RIGHT;
            return TRUE;
        }
        else if (car->dir == RIGHT && car->rightX == map->width - 2)
        {
            car->dir = LEFT;
            return TRUE;
        }
    }
    return FALSE;
}

bool MoveWrappingCar(CAR *car, WIN *map)
{
    if (car->carType == WRAPPING)
    {
        if (car->dir == LEFT && car->rightX == 0)
        {
            car->leftX = map->width - 2;
            car->rightX = map->width - 2 + car->length - 1;
            return TRUE;
        }
        else if (car->dir == RIGHT && car->leftX == map->width - 1)
        {
            car->leftX = -car->length+2;
            car->rightX = 1;
            return TRUE;
        }
    }
    return FALSE;
}

bool MoveDissapearingCar(CAR *car, WIN *map,int *settings)
{
    if (car->carType == DISAPPEARING)
    {
        if ((car->dir == LEFT && car->rightX == 0) || (car->dir == RIGHT && car->leftX == map->width - 1))
        {
            car->length = RA(settings[3], settings[4]);
            switch (car->speed = RA(0, 2))
            {
            case 0:
                car->speed = SUPERFAST_CAR;
                break;
            case 1:
                car->speed = FAST_CAR;
                break;
            case 2:
                car->speed = SLOW_CAR;
                break;
            }

            if (car->isFriendly = !RA(0, 3))
            {
                car->color = GREEN_COLOR;
            }
            else
            {
                car->color = RED_COLOR;
            }

            if (car->dir = RA(0, 1))
            {
                car->rightX = RA(-12, -5);
                car->leftX = car->rightX - car->length + 1;
            }
            else
            {
                car->leftX = RA(map->width + 3, map->width + 10);
                car->rightX = car->leftX + car->length - 1;
            }
            return TRUE;
        }
    }
    return FALSE;
}

void MoveCar(CAR *car, TIMER *timer, WIN *map, PLAYER *player, int y, int*settings)
{
    if (timer->frame_no - car->frame >= car->speed)
    {
        if (!(MoveBouncingCar(car, map)) && !MoveWrappingCar(car, map) && !MoveDissapearingCar(car, map,settings))
        {
            if (car->dir == LEFT)
            {
                if (CheckCarCollision(player, car, map) && car->isFriendly && player->x > player->minX && player->y == y && player->travels)
                {
                    player->x -= 1;
                }
                car->leftX -= 1;
                car->rightX -= 1;
            }
            else
            {
                if (CheckCarCollision(player, car, map) && car->isFriendly && player->x < player->maxX && player->y == y && player->travels)
                {
                    player->x += 1;
                }
                car->leftX += 1;
                car->rightX += 1;
            }
        }
        car->frame = timer->frame_no;
    }
}

void ResetPlayerPosition(PLAYER *player, WIN *map, TIMER *timer,LANE*lanes)
{
    if (player->y == 1)
    {
        player->y = map->height - 2;
        player->x = map->width / 2;
        timer->points+=END_POINTS;
    }
    else if(!(!CheckCarCollision(player, lanes[player->y - 2].car, map) || lanes[player->y - 2].car->isFriendly))
    {
        player->y = map->height - 2;
        player->x = map->width / 2;
    }
}

//------------------------------------------------
//------------------- GAME BODY  -----------------
//------------------------------------------------

int MainLoop(PLAYER *player, WIN *map, WIN *status, TIMER *timer, LANE *lanes, char *ch, int*settings)
{
    clock_t startFrame, endFrame;
    while ((*ch = wgetch(map->win)) != 'x')
    {
        startFrame = time(NULL);
        CleanWin(map);
        if (*ch == 'r')
        {
            player->travels = !(player->travels);
        }
        else if (*ch != ERR)
        {
            PlayerMovement(map->win, player, *ch, timer);
        }
        for (int i = 0; i < map->height - 4; i++)
        {
            MoveCar(lanes[i].car, timer, map, player, i + 2,settings);
            DrawCar(lanes[i].car, map, i + 2);
        }
        ResetPlayerPosition(player,map,timer,lanes);
        DrawPlayer(map->win, player);
        wrefresh(map->win);
        endFrame = time(NULL);
        UpdateStatus(status, timer);
        // if time's up end game
        if (UpdateTimer(timer, startFrame, endFrame))
        {
            break;
        }
        
    };
}

void FreeMemory(WIN*map,WIN*status,TIMER*timer,PLAYER*player,LANE*lanes)
{
    free(timer);
    free(player);
    delwin(map->win);
    delwin(status->win);
    free(map);
    free(status);
    for (int x = 0; x < MAP_HEIGHT - 4; x++)
    {
        free(lanes[x].car);
    }
    free(lanes);
}

void FileHandling(int*settings)
{
    FILE*gameSettings;
    gameSettings = fopen("settings.txt","r");
    if(gameSettings!=NULL)
    {
        fscanf(gameSettings,"%d %d %d %d %d",&settings[0],&settings[1],&settings[2],&settings[3],&settings[4]);
    }
    fclose(gameSettings);
}

int main()
{
    WINDOW *stdwin = InitGame();
    Welcome(stdwin);

    int settings[5] = {MAP_WIDTH,MAP_HEIGHT,GAME_TIME,MIN_CAR_LENGTH,MAX_CAR_LENGTH};
    FileHandling(settings);
    
    WIN *map = InitWindow(stdwin, settings[0], settings[1], 0, 0, "frogger");        
    nodelay(map->win, true);
    // initializing stuff
    WIN *status = InitWindow(stdwin, map->width, STATUS_HEIGHT, map->height, 0, "status");

    mvwprintw(stdwin, map->height+status->height, 1, "Oktawian Bieszke s203557");
    wrefresh(stdwin);
    PLAYER *player = InitPlayer(map->height - 2, map->width / 2, 1, map->height - 2, 1, map->width - 2, 'O');

    TIMER *timer = InitTimer(settings[2]);

    // basic gameloop
    LANE *lanes = InitLanes(map,settings);
    char ch;
    MainLoop(player, map, status, timer, lanes, &ch,settings);
    if (ch == 'x')
    {
        Quit(status);
    }
    else
    {
        GameOver(status,timer);
    }

    flushinp();
    //freeing allocated memory
    FreeMemory(map,status,timer,player,lanes);

    endwin();

    return 0;
}
