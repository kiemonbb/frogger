#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "structs.h"

#define GREEN_COLOR 1
#define RED_COLOR 2
#define BLUE_COLOR 3
#define MAGENTA_COLOR 4
#define WHITE_COLOR 5

#define MAP_HEIGHT 20   //default game settings, when settings.txt file is empty or nonexistent
#define MAP_WIDTH 30
#define GAME_TIME 30
#define MIN_CAR_LENGTH 6
#define MAX_CAR_LENGTH 10


#define FRIENDLY_CAR_CHANCE 3 //1 in x+1 cars is like that

#define MIN_CAR_SPEED 0     //slowcar
#define MAX_CAR_SPEED 2     //fastcar
#define STORK_EXISTS 1      //superfastcar

#define STATUS_HEIGHT 4

#define FRAME_TIME 16.66666 // time interval between frames in ms

#define FBM_PLAYER 12       // frames between movement | FBM_PLAYER*FRAME_TIME => How often can player move
#define FBM_STORK 40        
#define SLOW_CAR 9          //every how many frames can player/car/stork move
#define FAST_CAR 6
#define SUPERFAST_CAR 3

#define END_POINTS 200 // points gained by reaching the end of map and making a single step forward
#define STEP_POINTS 5

#define RA(min, max) ((min) + rand() % ((max) - (min) + 1))  //generating random number between min and max

//used some ideas from demo game CATCH THE BALL written by MM

//------------------------------------------------
//----------------- INIT FUNCTIONS ---------------
//------------------------------------------------

WINDOW *InitGame()
{
    WINDOW *stdwin;
    stdwin = initscr(); //initializing our main ncurses window that will contain any other windows

    start_color();                                      //initializing colors
    init_pair(GREEN_COLOR, COLOR_WHITE, COLOR_GREEN);
    init_pair(RED_COLOR, COLOR_WHITE, COLOR_RED);
    init_pair(WHITE_COLOR, COLOR_BLACK, COLOR_WHITE);
    init_pair(BLUE_COLOR, COLOR_WHITE, COLOR_BLUE);
    init_pair(MAGENTA_COLOR, COLOR_WHITE, COLOR_MAGENTA);

    cbreak();       //makes user input immediately available to the program without having to click enter
    curs_set(0);    //no cursor
    noecho();
    return stdwin;
}
//initializing WIN struct containing a window along with its properties
WIN *InitWindow(WINDOW *parent, int width, int height, int y, int x, char *title)
{
    WIN *tempWin = (WIN *)malloc(sizeof(WIN));
    if(tempWin == NULL)
    {
        fprintf(stderr,"Failed to allocate memory");
        exit(1);
    }
    tempWin->height = height;
    tempWin->width = width;
    tempWin->x = x;
    tempWin->y = y;
    tempWin->win = subwin(parent, height, width, y, x);
    box(tempWin->win, 0, 0);
    nodelay(tempWin->win, true);
    mvwprintw(tempWin->win, 0, 1, title);
    return tempWin;
}

PLAYER *InitPlayer(int y, int x, int minY, int maxY, int minX, int maxX, char shape)
{
    PLAYER *tempPlayer = (PLAYER *)malloc(sizeof(PLAYER));
    if(tempPlayer == NULL)
    {
        fprintf(stderr,"Failed to allocate memory");
        exit(1);
    }
    tempPlayer->y = y;
    tempPlayer->x = x;
    tempPlayer->prevY = y;
    tempPlayer->prevX = x;

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

STORK *InitStork(int x, int y, char shape, bool exists)
{
    STORK *stork = (STORK *)malloc(sizeof(STORK));
    if(stork == NULL)
    {
        fprintf(stderr,"Failed to allocate memory");
        exit(1);
    }
    stork->x = x;
    stork->y = y;
    stork->shape = shape;
    stork->frame = 1;
    stork->exists = exists;
    return stork;
}

CAR *InitCar(int length, int headX, DIR dir, char headShapeLeft, char headShapeRight, char bodyShape, CARTYPE carType, bool isFriendly, bool exists, SPEED speed)
{
    CAR *tempCar = (CAR *)malloc(sizeof(CAR));
    if(tempCar == NULL)
    {
        fprintf(stderr,"Failed to allocate memory");
        exit(1);
    }
    if (isFriendly)
    {
        tempCar->color = GREEN_COLOR;
        tempCar->stops = FALSE;
    }
    else
    {
        tempCar->color = RED_COLOR;
        tempCar->stops = !RA(0, 3);
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
    case 2:
        tempCar->speed = SUPERFAST_CAR;
        break;
    case 1:
        tempCar->speed = FAST_CAR;
        break;
    case 0:
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

OBSTACLE *InitObstacle(OBSTACLE_TYPE type, WIN *map, bool exists)
{
    OBSTACLE *obstacle = (OBSTACLE *)malloc(sizeof(OBSTACLE));
    if(obstacle == NULL)
    {
        fprintf(stderr,"Failed to allocate memory");
        exit(1);
    }
    obstacle->positions = (int *)calloc(map->width, sizeof(int));
    if(obstacle->positions == NULL)
    {
        fprintf(stderr,"Failed to allocate memory");
        exit(1);
    }

    obstacle->exists = exists;

    // positions meaning => 0: normal tile 1:obstacle that moves you to the start 2: obstacle that you cant move through
    if (type == RIVER)
    {
        obstacle->color = BLUE_COLOR;
        for (int i = 0; i < map->width / 3; i++)
        {
            obstacle->positions[i] = 1;
        }
        for (int i = map->width - map->width / 3; i < map->width; i++)
        {
            obstacle->positions[i] = 1;
        }
    }
    else if (type == TWO_ROCKS)
    {
        obstacle->color = WHITE_COLOR;
        for (int i = map->width / 4; i < (map->width / 5) * 2; i++)
        {
            obstacle->positions[i] = 2;
        }
        for (int i = (map->width / 5) * 3; i < (map->width / 5) * 4; i++)
        {
            obstacle->positions[i] = 2;
        }
    }
    else
    {
        obstacle->color = WHITE_COLOR;
        for (int i = map->width / 2; i < (map->width / 6) * 5; i++)
        {
            obstacle->positions[i] = 2;
        }
    }
    return obstacle;
}
//each lane contains both a car and an obstacle, however only one of them will be present during the game
LANE *InitLanes(WIN *map, long int *settings)
{
    LANE *lanes = (LANE *)malloc(sizeof(LANE) * (map->height - 4));
    if(lanes == NULL)
    {
        fprintf(stderr,"Failed to allocate memory");
        exit(1);
    }
    for (int i = 0; i < map->height - 4; i++)
    {
        lanes[i].car = InitCar(RA(settings[3], settings[4]), RA(3, map->width - 3), RA(0, 1), '<', '>', 'X', RA(0, 2), !RA(0, settings[6]), RA(0, 2), RA(settings[7], settings[8]));
        lanes[i].obstacle = InitObstacle(RA(0, 2), map, !lanes[i].car->exists && RA(0, 1));
    }
    return lanes;
}

TIMER *InitTimer(float gameTime)
{
    TIMER *timer = (TIMER *)malloc(sizeof(TIMER));
    if(timer == NULL)
    {
        fprintf(stderr,"Failed to allocate memory");
        exit(1);
    }
    timer->frame_no = 1;
    timer->points = 0;
    timer->gameTime = gameTime;
    return timer;
}

//------------------------------------------------
//---------------- DRAW FUNCTIONS ----------------
//------------------------------------------------

//erasing content inside of window
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

void DrawStork(WINDOW *win, STORK *stork)
{
    if (stork->exists)
    {
        wattron(win, COLOR_PAIR(MAGENTA_COLOR) | A_BOLD);
        mvwaddch(win, stork->y, stork->x, stork->shape);
        wattroff(win, COLOR_PAIR(MAGENTA_COLOR) | A_BOLD);
    }
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

void DrawObstacle(OBSTACLE *obstacle, WIN *map, int y)
{
    if (obstacle->exists)
    {
        wattron(map->win, COLOR_PAIR(obstacle->color));
        for (int x = 1; x <= map->width - 2; x++)
        {
            if (obstacle->positions[x] != 0)
            {
                wattron(map->win, COLOR_PAIR(obstacle->color));
                mvwaddch(map->win, y, x, ' ');
            }
            wattroff(map->win, COLOR_PAIR(obstacle->color));
        }
    }
}

//------------------------------------------------
//--------------- MOVEMENT FUNCTIONS -------------
//------------------------------------------------

void PlayerMovement(PLAYER *player, char ch, TIMER *timer)
{
    if (timer->frame_no - player->frame >= FBM_PLAYER)  //player can move only if FBM_PLAYER*FRAME_TIME ms have passed since his last move
    {
        if (ch == 'w' && player->y > player->minY)
        {
            player->prevY = player->y;
            player->prevX = player->x;
            player->y -= 1;
            timer->points += STEP_POINTS;
        }
        else if (ch == 's' && player->y < player->maxY)
        {
            player->prevY = player->y;
            player->prevX = player->x;
            player->y += 1;
        }
        else if (ch == 'a' && player->x > player->minX)
        {
            player->prevY = player->y;
            player->prevX = player->x;
            player->x -= 1;
        }
        else if (ch == 'd' && player->x < player->maxX)
        {
            player->prevY = player->y;
            player->prevX = player->x;
            player->x += 1;
        }
        player->frame = timer->frame_no;
    }
}

void StorkMovement(PLAYER *player, STORK *stork, TIMER *timer)
{
    if (timer->frame_no - stork->frame >= FBM_STORK && stork->exists)
    {
        if (stork->x - player->x > 0)
        {
            stork->x -= 1;
        }
        else if (stork->x - player->x < 0)
        {
            stork->x += 1;
        }
        stork->frame = timer->frame_no;

        if (stork->y - player->y > 0)
        {
            stork->y -= 1;
        }
        else if (stork->y - player->y < 0)
        {
            stork->y += 1;
        }
        stork->frame = timer->frame_no;
    }
}

//checking if the player is coliding with provided car
bool CheckCarCollision(PLAYER *player, CAR *car, WIN *map)
{
    if (player->y > 1 && player->y < map->height - 2 && car->exists && player->x >= car->leftX && player->x <= car->rightX)
    {
        return TRUE;
    }
    return FALSE;
}

//checking if the player is coliding with provided obstacle of a specific type
bool CheckObstacleCollision(PLAYER *player, OBSTACLE *obstacle, WIN *map, int type)
{
    if (player->y > 1 && player->y < map->height - 2 && obstacle->positions[player->x] == type && obstacle->exists == TRUE)
    {
        return TRUE;
    }
    return FALSE;
}

void ChangeCarSpeed(CAR *car,long int*settings)
{
    switch (car->speed = RA(settings[7],settings[8]))
    {
    case 2:
        car->speed = SUPERFAST_CAR;
        break;
    case 1:
        car->speed = FAST_CAR;
        break;
    case 0:
        car->speed = SLOW_CAR;
        break;
    }
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

bool MoveWrappingCar(CAR *car, WIN *map, long int *settings)
{
    if (car->carType == WRAPPING)
    {
        if (car->dir == LEFT && car->rightX == 0)
        {
            ChangeCarSpeed(car,settings);
            car->leftX = map->width - 2;
            car->rightX = map->width - 2 + car->length - 1;
            return TRUE;
        }
        else if (car->dir == RIGHT && car->leftX == map->width - 1)
        {
            ChangeCarSpeed(car,settings);
            car->leftX = -car->length + 2;
            car->rightX = 1;
            return TRUE;
        }
    }
    return FALSE;
}

//essentialy generates a whole new car outside of game window
bool MoveDisapearingCar(CAR *car, WIN *map, long int *settings)
{
    if (car->carType == DISAPPEARING)
    {
        if ((car->dir == LEFT && car->rightX == 0) || (car->dir == RIGHT && car->leftX == map->width - 1))
        {
            car->length = RA(settings[3], settings[4]);
            ChangeCarSpeed(car, settings);
            if (car->isFriendly = !RA(0, 3))
            {
                car->color = GREEN_COLOR;
                car->stops = FALSE;
            }
            else
            {
                car->color = RED_COLOR;
                car->stops = !RA(0, 3);
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

bool CanCarMove(CAR *car, PLAYER *player, int y)
{
    if (car->stops == TRUE && player->x > car->leftX - 4 && player->x < car->rightX + 4 && player->y == y)
    {
        return FALSE;
    }
    return TRUE;
}

void MoveCar(CAR *car, TIMER *timer, WIN *map, PLAYER *player, int y, long int *settings)
{
    if (timer->frame_no - car->frame >= car->speed) 
    {
        if (!MoveBouncingCar(car, map) && !MoveWrappingCar(car, map,settings) && !MoveDisapearingCar(car, map, settings) && CanCarMove(car, player, y))
        {
            if (car->dir == LEFT)
            {
                
                if (CheckCarCollision(player, car, map) && car->isFriendly && player->x > player->minX && player->y == y && player->travels)
                {
                    //making player ride along with a friendly car
                    player->x -= 1;
                }
                car->leftX -= 1;
                car->rightX -= 1;
            }
            else
            {
                if (CheckCarCollision(player, car, map) && car->isFriendly && player->x < player->maxX && player->y == y && player->travels)
                {
                    //making player ride along with a friendly car
                    player->x += 1;
                }
                car->leftX += 1;
                car->rightX += 1;
            }
        }
        car->frame = timer->frame_no;
    }
}

//if player collides with stork send them to their starting positions
void ResetPlayerAndStork(PLAYER *player, STORK *stork, WIN *map, TIMER *timer)
{
    if (player->x == stork->x && player->y == stork->y)
    {
        if (player->y < player->prevY)
        {
            timer->points -= STEP_POINTS;
        }
        player->x = map->width / 2;
        player->y = map->height - 2;
        stork->x = map->width - 2;
        stork->y = 1;
    }
}

bool ResetPlayerPosition(PLAYER *player, WIN *map, TIMER *timer, CAR *car, OBSTACLE *obstacle, bool isLevel)
{
    if (player->y == 1 && !isLevel)
    {
        player->y = map->height - 2;
        player->x = map->width / 2;
        timer->points += END_POINTS;
        return TRUE;
    }
    //checking if encountered obstacle/car can reset players position, and if yes doing so
    else if (!(!CheckCarCollision(player, car, map) || car->isFriendly) || CheckObstacleCollision(player, obstacle, map, 1))
    {
        if (player->y < player->prevY)
        {
            timer->points -= STEP_POINTS;
        }
        player->y = map->height - 2;
        player->x = map->width / 2;
        return TRUE;
    }
    else if (CheckObstacleCollision(player, obstacle, map, 2))
    {
        if (player->y < player->prevY)
        {
            timer->points -= STEP_POINTS;
        }
        player->y = player->prevY;
        player->x = player->prevX;
        return FALSE;
    }
}

void DrawAndMoveLanes(WIN *map, WIN *status, PLAYER *player, LANE *lanes, TIMER *timer, long int *settings)
{
    for (int i = 0; i < map->height - 4; i++)
    {
        MoveCar(lanes[i].car, timer, map, player, i + 2, settings);
        DrawCar(lanes[i].car, map, i + 2);
        DrawObstacle(lanes[i].obstacle, map, i + 2);
    }
}

//------------------------------------------------
//------------ FILE HANDLING FUNCTIONS -----------
//------------------------------------------------

void GetSettings(long int *settings, char *filename)
{
    FILE *gameSettings;
    gameSettings = fopen(filename, "r");
    if (gameSettings != NULL)
    {
        if (filename == "settings.txt" || filename == "replay.txt")
        {
            fscanf(gameSettings, "%d %d %d %d %d %ld", &settings[0], &settings[1], &settings[2], &settings[3], &settings[4], &settings[5]);
        }
        else
        {
            fscanf(gameSettings, "%d %d %d", &settings[0], &settings[1], &settings[2]);
        }
    }
    fclose(gameSettings);
}

void HandleSettings(long int *settings, char *ch)
{
    if (*ch == '1')
    {
        GetSettings(settings, "level1.txt");
    }
    else if (*ch == '2')
    {
        GetSettings(settings, "level2.txt");
    }
    else if (*ch == '3')
    {
        GetSettings(settings, "level3.txt");
    }
    else if (*ch == 'm')
    {
        GetSettings(settings, "replay.txt");
    }
    //setting max/min car length, chance of a car being a friendly car
    //min/max car speed
    //and if stork exists
    else if(*ch =='4')
    {
        settings[3] = 3;
        settings[4] = 6;
        settings[6] = 2;
        settings[7] = 0;
        settings[8] = 1;
        settings[9] = 0;
    }
    else if(*ch =='5')
    {
        settings[3] = 4;
        settings[4] = 8;
        settings[6] = 3;
        settings[7] = 0;
        settings[8] = 2;
        settings[9] = 0;
    }
    else if(*ch =='6')
    {
        settings[3] = 5;
        settings[4] = 10;
        settings[6] = 4;
        settings[7] = 1;
        settings[8] = 2;
        settings[9] = 1;

    }
    else
    {
        GetSettings(settings, "settings.txt");
    }
}

void LoadLevel(WIN *map, long int *settings, LANE *lanes, char *file)
{
    FILE *level = fopen(file, "r");
    if (level != NULL)
    {
        int a;
        fscanf(level, "%d %d %d", &a, &a, &a);
        for (int i = 0; i < map->height - 4; i++)
        {
            lanes[i].car->exists = FALSE;
            lanes[i].obstacle->exists = TRUE;
            lanes[i].obstacle->color = WHITE_COLOR;
            for (int x = 1; x <= map->width - 2; x++)
            {
                fscanf(level, "%d", &lanes[i].obstacle->positions[x]);
            }
        }
    }
    fclose(level);
}

int HighScoreGet()
{
    FILE *top;
    top = fopen("top.txt", "r");
    int highscore = 0;
    if (top != NULL)
    {
        fscanf(top, "%d", &highscore);
        fclose(top);
    }

    return highscore;
}

void HighScoreSet(int highscore, int points)
{
    if (points > highscore)
    {
        FILE *top;
        top = fopen("top.txt", "w");
        fprintf(top, "%d", points);
        fclose(top);
    }
}

void CheckLoadLevel(WIN *map, long int *settings, LANE *lanes, char *ch)
{
    if (*ch == '1')
    {
        LoadLevel(map, settings, lanes, "level1.txt");
    }
    else if (*ch == '2')
    {
        LoadLevel(map, settings, lanes, "level2.txt");
    }
    else if (*ch == '3')
    {
        LoadLevel(map, settings, lanes, "level3.txt");
    }
}

//------------------------------------------------
//---------------- OTHER FUNCTIONS ---------------
//------------------------------------------------

bool UpdateTimer(TIMER *timer, clock_t startFrame, clock_t endFrame)
{
    usleep((FRAME_TIME - (endFrame - startFrame) / CLOCKS_PER_SEC / 1000) * 1000); // program sleeps FRAME_TIME[ms] - time it takes to generate a single frame, between frames
    timer->frame_no++;
    timer->timeLeft = timer->gameTime - FRAME_TIME * timer->frame_no / 1000; // in seconds
    if (timer->timeLeft <= 0)
    {
        return TRUE;
    }
    return FALSE;
}

//freeing allocated memory duhh
void FreeMemory(WIN *map, WIN *status, TIMER *timer, PLAYER *player, LANE *lanes, STORK*stork)
{
    free(timer);
    free(player);
    for(int i = 0; i<map->height-4;i++)
    {
        free(lanes[i].car);
        free(lanes[i].obstacle->positions);
        free(lanes[i].obstacle);
    }
    free(lanes);
    delwin(status->win);
    free(map);
    free(status);
}

//------------------------------------------------
//------------- MENU/STATUS WRITING --------------
//------------------------------------------------

void Welcome(WINDOW *win, int highscore, char *ch)
{
    mvwprintw(win, 1, 2, "                Press n to record game and m to replay it                  ");
    mvwprintw(win, 2, 2, "             Use 1/2/3 to play one of three maze-like levels               ");
    mvwprintw(win, 3, 2, "     Use 4/5/6 to play one of three progressively more difficult level     ");
    mvwprintw(win, 4, 2, "           Use any other key to start the game in normal mode!             ");
    mvwprintw(win, 6, 2, "              Use w/s/a/d to go up/down/left/right respectively            ");
    mvwprintw(win, 7, 2, "                Use r to mount/unmount friendly (green) cars               ");
    mvwprintw(win, 8, 2, "                       Use x to exit the game anytime                      ");
    mvwprintw(win, 10, 2, "                          CURRENT HIGHSCORE: %d                            ", highscore);
    *ch = wgetch(win);
    wclear(win);
    wrefresh(win);
}

void YouWonLevel(WIN *status)
{
    CleanWin(status);
    wattron(status->win, A_BOLD);
    mvwprintw(status->win, 1, status->width / 2 - 3, "YOU WON");
    mvwprintw(status->win, 2, status->width / 2 - 1, "!!!");
    wattroff(status->win, A_BOLD);
    wrefresh(status->win);
    sleep(3);
}

void YouLostLevel(WIN *status)
{
    CleanWin(status);
    wattron(status->win, A_BOLD);
    mvwprintw(status->win, 1, status->width / 2 - 4, "YOU LOST");
    mvwprintw(status->win, 2, status->width / 2 - 1, ":(");
    wattroff(status->win, A_BOLD);
    wrefresh(status->win);
    sleep(3);
}

void GameOver(WIN *status, TIMER *timer)
{
    CleanWin(status);
    wattron(status->win, A_BOLD);
    mvwprintw(status->win, 1, status->width / 2 - 4, "GAME OVER", timer->points);
    mvwprintw(status->win, 2, status->width / 2 - 7, "Your Score: %d", timer->points);
    wattroff(status->win, A_BOLD);
    wrefresh(status->win);
    sleep(3);
}

void StartReplay(WIN *status)
{
    wattron(status->win, A_BOLD);
    mvwprintw(status->win, 1, status->width / 2 - 4, "STARTING");
    mvwprintw(status->win, 2, status->width / 2 - 3, "REPLAY");
    wattroff(status->win, A_BOLD);
    wrefresh(status->win);
    sleep(2);
}

void EndOfReplay(WIN *status)
{
    CleanWin(status);
    wattron(status->win, A_BOLD);
    mvwprintw(status->win, 1, status->width / 2 - 3, "END OF");
    mvwprintw(status->win, 2, status->width / 2 - 3, "REPLAY");
    wattroff(status->win, A_BOLD);
    wrefresh(status->win);
    sleep(2);
}

void Quit(WIN *status)
{
    CleanWin(status);
    wattron(status->win, A_BOLD);
    mvwprintw(status->win, 1, status->width / 2 - 4, "YOU QUIT");
    mvwprintw(status->win, 2, status->width / 2 - 2, "BYE!");
    wattroff(status->win, A_BOLD);
    wrefresh(status->win);
    sleep(3);
}

void UpdateMainStatus(WIN *status, TIMER *timer)
{
    CleanWin(status);
    mvwprintw(status->win, 1, status->width / 2 - 5, "Time: %.2fs", timer->timeLeft);
    mvwprintw(status->win, 2, status->width / 2 - 5, "Score: %d", timer->points);
    wrefresh(status->win);
}

void UpdateLevelStatus(WIN *status, TIMER *timer)
{
    CleanWin(status);
    mvwprintw(status->win, 1, status->width / 2 - 2, "Time:");
    mvwprintw(status->win, 2, status->width / 2 - 2, "%.2fs", timer->timeLeft);
    wrefresh(status->win);
}

//------------------------------------------------
//------------------- GAME BODY  -----------------
//------------------------------------------------

void ReplayLoop(PLAYER *player, STORK *stork, WIN *map, WIN *status, TIMER *timer, LANE *lanes, char *ch, long int *settings)
{
    FILE *replayFile = fopen("replay.txt", "r");
    if (replayFile == NULL)
    {
        return;
    }
    else
    {
        int a;
        fscanf(replayFile, "%d%d%d%d%d%ld", &a, &a, &a, &a, &a, &a);
    }
    clock_t startFrame, endFrame;
    StartReplay(status);
    while (*ch = wgetch(map->win) != 'x')
    {
        startFrame = clock();
        CleanWin(map);
        DrawAndMoveLanes(map, status, player, lanes, timer, settings);
        fscanf(replayFile, "%d%d", &player->x, &player->y);
        StorkMovement(player, stork, timer);
        ResetPlayerAndStork(player, stork, map, timer);
        ResetPlayerPosition(player, map, timer, lanes[player->y - 2].car, lanes[player->y - 2].obstacle, FALSE);
        DrawPlayer(map->win, player);
        DrawStork(map->win, stork);
        wrefresh(map->win);
        endFrame = clock();
        fscanf(replayFile, "%d", &timer->points);
        UpdateMainStatus(status, timer);
        // if time's up end game
        if (UpdateTimer(timer, startFrame, endFrame))
        {
            break;
        }
    };
    fclose(replayFile);
}

bool LevelLoop(PLAYER *player, LANE *lanes, WIN *map, WIN *status, TIMER *timer, char *ch, long int *settings)
{
    clock_t startFrame, endFrame;
    while ((*ch = wgetch(map->win)) != 'x')
    {
        startFrame = time(NULL);
        CleanWin(map);
        for (int i = 0; i < map->height - 4; i++)
        {
            DrawObstacle(lanes[i].obstacle, map, i + 2);
        }
        if (*ch != ERR)
        {
            PlayerMovement(player, *ch, timer);
        }
        ResetPlayerPosition(player, map, timer, lanes[player->y - 2].car, lanes[player->y - 2].obstacle, TRUE);
        DrawPlayer(map->win, player);
        wrefresh(map->win);
        endFrame = time(NULL);
        UpdateLevelStatus(status, timer);
        if (player->y == 1)
        {
            return TRUE;
        }
        // if time's up end game
        else if (UpdateTimer(timer, startFrame, endFrame))
        {
            return FALSE;
        }
    };
    return FALSE;
}

void MainLoop(PLAYER *player, STORK *stork, WIN *map, WIN *status, TIMER *timer, LANE *lanes, char *ch, long int *settings, bool isRecording)
{
    FILE *replayFile = fopen("replay.txt", "w");
    if (isRecording)
    {
        fprintf(replayFile, "%d %d %d %d %d %d ", settings[0], settings[1], settings[2], settings[3], settings[4], settings[5]);
    }
    clock_t startFrame, endFrame;
    while ((*ch = wgetch(map->win)) != 'x')
    {
        startFrame = clock();
        CleanWin(map);
        DrawAndMoveLanes(map, status, player, lanes, timer, settings);
        if (*ch == 'r')
        {
            player->travels = !(player->travels);
        }
        else if (*ch != ERR)
        {
            PlayerMovement(player, *ch, timer);
        }
        if (isRecording)
        {
            fprintf(replayFile, "%d %d %d ", player->x, player->y, timer->points);
        }
        StorkMovement(player, stork, timer);
        ResetPlayerAndStork(player, stork, map, timer);
        ResetPlayerPosition(player, map, timer, lanes[player->y - 2].car, lanes[player->y - 2].obstacle, FALSE);
        DrawPlayer(map->win, player);
        DrawStork(map->win, stork);
        wrefresh(map->win);
        endFrame = clock();
        UpdateMainStatus(status, timer);
        // if time's up end game
        if (UpdateTimer(timer, startFrame, endFrame))
        {
            break;
        }
    };
    fclose(replayFile);
}

void PlayGame(WIN *map, WIN *status, PLAYER *player, STORK *stork, TIMER *timer, LANE *lanes, long int *settings, char *ch, int highscore)
{
    if (*ch == '1' || *ch == '2' || *ch == '3')
    {
        if (LevelLoop(player, lanes, map, status, timer, ch, settings))
        {
            YouWonLevel(status);
        }
        else
        {
            YouLostLevel(status);
        }
    }
    else if (*ch == 'm')
    {
        ReplayLoop(player, stork, map, status, timer, lanes, ch, settings);
        if (*ch == 'x')
        {
            Quit(status);
        }
        else
        {
            GameOver(status, timer);
        }
        EndOfReplay(status);
    }
    else
    {
        MainLoop(player, stork, map, status, timer, lanes, ch, settings, *ch == 'n' ? TRUE : FALSE);
        if (*ch == 'x')
        {
            Quit(status);
        }
        else
        {
            GameOver(status, timer);
        }
        HighScoreSet(highscore, timer->points);
    }
}

int main()
{
    WINDOW *stdwin = InitGame();
    int highscore = HighScoreGet();
    char ch;
    Welcome(stdwin, highscore, &ch);

    long int settings[10] = {MAP_WIDTH, MAP_HEIGHT, GAME_TIME, MIN_CAR_LENGTH, MAX_CAR_LENGTH, time(NULL),FRIENDLY_CAR_CHANCE,MIN_CAR_SPEED,MAX_CAR_SPEED,STORK_EXISTS};

    HandleSettings(settings, &ch);

    srand(settings[5]);

    // initializing stuff
    WIN *map = InitWindow(stdwin, settings[0], settings[1], 0, 0, "frogger");
    WIN *status = InitWindow(stdwin, map->width, STATUS_HEIGHT, map->height, 0, "status");
    PLAYER *player = InitPlayer(map->height - 2, map->width / 2, 1, map->height - 2, 1, map->width - 2, 'O');
    STORK *stork = InitStork(map->width - 2, 1, '4', settings[9]);
    TIMER *timer = InitTimer(settings[2]);
    LANE *lanes = InitLanes(map, settings);

    CheckLoadLevel(map, settings, lanes, &ch);
    mvwprintw(stdwin, map->height + status->height, 1, "Oktawian Bieszke s203557");
    wrefresh(stdwin);
    PlayGame(map, status, player, stork, timer, lanes, settings, &ch, highscore);
    flushinp();
    
    FreeMemory(map, status, timer, player, lanes, stork);
    endwin();
    return 0;
}