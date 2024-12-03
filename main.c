#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "structs.h"

#define GREEN_COLOR 1
#define RED_COLOR 2
#define BLUE_COLOR 3
#define YELLOW_COLOR 4
#define WHITE_COLOR 5
#define MAGENTA_COLOR 7

#define MAP_HEIGHT 20
#define MAP_WIDTH 30

#define STATUS_HEIGHT 4

#define FRAME_TIME 16.66666 // time interval between frames in ms
#define FBM_PLAYER 12       // frames between movement | FBM_PLAYER*FRAME_TIME => How often can player move
#define FBM_STORK 40
#define GAME_TIME 30

#define MIN_CAR_LENGTH 6
#define MAX_CAR_LENGTH 10

#define END_POINTS 200 // points gained by reaching the end of map and making a single step forward
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
    init_pair(YELLOW_COLOR,COLOR_WHITE,COLOR_YELLOW);
    init_pair(WHITE_COLOR, COLOR_BLACK, COLOR_WHITE);
    init_pair(BLUE_COLOR,COLOR_WHITE,COLOR_BLUE);
    init_pair(MAGENTA_COLOR,COLOR_WHITE,COLOR_MAGENTA);

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

STORK *InitStork(int x, int y, char shape,bool exists)
{
    STORK *stork = (STORK *)malloc(sizeof(STORK));
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

OBSTACLE *InitObstacle(OBSTACLE_TYPE type, WIN *map, bool exists)
{
    OBSTACLE *obstacle = (OBSTACLE *)malloc(sizeof(OBSTACLE));
    obstacle->positions = (int *)calloc(map->width, sizeof(int));

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

LANE *InitLanes(WIN *map, int *settings)
{
    LANE *lanes = (LANE *)malloc(sizeof(LANE) * (map->height - 4));
    for (int i = 0; i < map->height - 4; i++)
    {
        lanes[i].car = InitCar(RA(settings[3], settings[4]), RA(3, map->width - 3), RA(0, 1), '<', '>', 'X', RA(0, 2), !RA(0, 3), RA(0, 2), RA(0, 2));
        lanes[i].obstacle = InitObstacle(RA(0, 2), map, !lanes[i].car->exists && RA(0, 1));
    }
    return lanes;
}

TIMER *InitTimer(float gameTime)
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

void DrawStork(WINDOW*win, STORK*stork)
{
    if(stork->exists){
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
    if (timer->frame_no - player->frame >= FBM_PLAYER)
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

void StorkMovement(PLAYER*player,STORK*stork,TIMER*timer)
{
    if (timer->frame_no - stork->frame >= FBM_STORK &&stork->exists)
    {
        if(stork->x-player->x>0)
        {
            stork->x-=1;
        }
        else if(stork->x-player->x<0)
        {
            stork->x+=1;
        }
        stork->frame = timer->frame_no;

        if(stork->y-player->y>0)
        {
            stork->y-=1;
        }
        else if(stork->y-player->y<0)
        {
            stork->y+=1;
        }
        stork->frame = timer->frame_no;
    }
}

bool CheckCarCollision(PLAYER *player, CAR *car, WIN *map)
{
    if (player->y > 1 && player->y < map->height - 2 && car->exists && player->x >= car->leftX && player->x <= car->rightX)
    {
        return TRUE;
    }
    return FALSE;
}

bool CheckObstacleCollision(PLAYER *player, OBSTACLE *obstacle, WIN *map, int type)
{
    if (player->y > 1 && player->y < map->height - 2 && obstacle->positions[player->x] == type && obstacle->exists == TRUE)
    {
        return TRUE;
    }
    return FALSE;
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
            car->leftX = -car->length + 2;
            car->rightX = 1;
            return TRUE;
        }
    }
    return FALSE;
}

bool MoveDisapearingCar(CAR *car, WIN *map, int *settings)
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
    if (car->stops == TRUE && player->x > car->leftX - 5 && player->x < car->rightX + 5 && player->y == y)
    {
        return FALSE;
    }
    return TRUE;
}

void MoveCar(CAR *car, TIMER *timer, WIN *map, PLAYER *player, int y, int *settings)
{
    if (timer->frame_no - car->frame >= car->speed)
    {
        if (!(MoveBouncingCar(car, map)) && !MoveWrappingCar(car, map) && !MoveDisapearingCar(car, map, settings) && CanCarMove(car, player, y))
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

void ResetPlayerAndStork(PLAYER*player,STORK*stork,WIN*map,TIMER*timer)
{
    if(player->x == stork->x && player->y == stork->y)
    {
        if(player->y <player->prevY)
        {
            timer->points-=STEP_POINTS;
        }
        player->x = map->width/2;
        player->y = map->height-2;
        stork->x = map->width-2;
        stork->y = 1;
    }
}

bool ResetPlayerPosition(PLAYER *player, WIN *map, TIMER *timer, CAR *car, OBSTACLE *obstacle,bool isLevel)
{
    if (player->y == 1 && !isLevel)
    {
        player->y = map->height - 2;
        player->x = map->width / 2;
        timer->points += END_POINTS;
        return TRUE;
    }
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

void Welcome(WINDOW *win, int highscore,char*ch)
{
    mvwprintw(win, 1, 2, "   Use 'wasd' to move around    ");
    mvwprintw(win, 2, 2, "  Press 'x' to close the game   ");
    mvwprintw(win, 3, 2, "Press 'r' to drive on green cars");
    mvwprintw(win, 5, 2, "Press any key to start the game!");
    mvwprintw(win, 7, 2, "    CURRENT HIGHSCORE: %d       ", highscore);
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

void YouLostLevel(WIN*status)
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
    mvwprintw(status->win, 1, status->width / 2 - 5, "Time:%.2fs", timer->timeLeft);
    mvwprintw(status->win, 2, status->width / 2 - 5, "Score: %d", timer->points);
    wrefresh(status->win);
}

void UpdateLevelStatus(WIN*status,TIMER*timer)
{
    CleanWin(status);
    mvwprintw(status->win, 1, status->width / 2 - 2, "Time:");
    mvwprintw(status->win, 2, status->width / 2 - 2, "%.2fs", timer->timeLeft);

    wrefresh(status->win);
}

bool UpdateTimer(TIMER *timer, clock_t startFrame, clock_t endFrame)
{
    usleep((FRAME_TIME - (startFrame - endFrame)) * 1000); // program sleeps FRAME_TIME[ms] - time it takes to generate a single frame, between frames
    timer->frame_no++;
    timer->timeLeft = timer->gameTime - FRAME_TIME * timer->frame_no / 1000; // in seconds
    if (timer->timeLeft <= 0)
    {
        return TRUE;
    }
    return FALSE;
}

void FreeMemory(WIN *map, WIN *status, TIMER *timer, PLAYER *player, LANE *lanes)
{
    free(timer);
    free(player);
    delwin(map->win);
    delwin(status->win);
    free(map);
    free(status);
    free(lanes);
}

void GetSettings(int *settings, char*filename)
{
    FILE *gameSettings;
    gameSettings = fopen(filename, "r");
    if (gameSettings != NULL)
    {
        if (filename =="settings.txt")
        {
        fscanf(gameSettings,"%d %d %d %d %d",&settings[0],&settings[1],&settings[2],&settings[3],&settings[4]);
        }
        else
        {
        fscanf(gameSettings,"%d %d %d",&settings[0],&settings[1],&settings[2]);
        }
    }
    fclose(gameSettings);
}

void HandleSettings(int*settings,char*ch)
{
    if(*ch == '1')
    {
        GetSettings(settings,"level1.txt");
    }
    else if(*ch=='2')
    {
        GetSettings(settings,"level2.txt");
    }
    else if(*ch == '3')
    {
        GetSettings(settings,"level3.txt");
    }
    else
    {
        GetSettings(settings,"settings.txt");
    }
}

void LoadLevel(WIN*map,int*settings,LANE*lanes,char*file)
{
    FILE*level = fopen(file,"r");
    if(level!=NULL)
    {
        int a;
        fscanf(level,"%d %d %d",&a,&a,&a);
        for(int i = 0;i<map->height-4; i++)
        {
            lanes[i].car->exists=FALSE;
            lanes[i].obstacle->exists = TRUE;
            lanes[i].obstacle->color = WHITE_COLOR;
            for(int x = 1;x<=map->width-2;x++)
            {
                fscanf(level,"%d",&lanes[i].obstacle->positions[x]);
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
//------------------------------------------------
//------------------- GAME BODY  -----------------
//------------------------------------------------

bool LevelLoop(PLAYER*player,LANE*lanes,WIN*map,WIN*status,TIMER*timer,char*ch,int*settings)
{
    clock_t startFrame,endFrame;
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
            PlayerMovement (player, *ch, timer);
        }
        ResetPlayerPosition(player, map, timer, lanes[player->y - 2].car, lanes[player->y - 2].obstacle, TRUE);
        DrawPlayer(map->win, player);
        wrefresh(map->win);
        endFrame = time(NULL);
        UpdateLevelStatus(status, timer);
        if(player->y==1)
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

int MainLoop(PLAYER *player, STORK*stork, WIN *map, WIN *status, TIMER *timer, LANE *lanes, char *ch, int *settings)
{
    clock_t startFrame, endFrame;
    while ((*ch = wgetch(map->win)) != 'x')
    {
        startFrame = time(NULL);
        CleanWin(map);
        for (int i = 0; i < map->height - 4; i++)
        {
            MoveCar(lanes[i].car, timer, map, player, i + 2, settings);
            DrawCar(lanes[i].car, map, i + 2);
            DrawObstacle(lanes[i].obstacle, map, i + 2);
        }
        if (*ch == 'r')
        {
            player->travels = !(player->travels);
        }
        else if (*ch != ERR)
        {
            PlayerMovement (player, *ch, timer);
        }
        StorkMovement(player,stork,timer);
        ResetPlayerAndStork(player,stork,map,timer);
        ResetPlayerPosition(player, map, timer, lanes[player->y - 2].car, lanes[player->y - 2].obstacle,FALSE);
        DrawPlayer(map->win, player);
        DrawStork(map->win,stork);
        wrefresh(map->win);
        endFrame = time(NULL);
        UpdateMainStatus(status, timer);
        // if time's up end game
        if (UpdateTimer(timer, startFrame, endFrame))
        {
            break;
        }
    };
}

int main()
{
    WINDOW *stdwin = InitGame();
    int highscore = HighScoreGet();
    char ch;
    Welcome(stdwin, highscore,&ch);

    int settings[5] = {MAP_WIDTH, MAP_HEIGHT, GAME_TIME, MIN_CAR_LENGTH, MAX_CAR_LENGTH};

    HandleSettings(settings,&ch);

    // initializing stuff
    WIN *map = InitWindow(stdwin, settings[0], settings[1], 0, 0, "frogger");
    nodelay(map->win, true);
    WIN *status = InitWindow(stdwin, map->width, STATUS_HEIGHT, map->height, 0, "status");
    mvwprintw(stdwin, map->height + status->height, 1, "Oktawian Bieszke s203557");

    PLAYER *player = InitPlayer(map->height - 2, map->width / 2, 1, map->height - 2, 1, map->width - 2, 'O');
    STORK*stork = InitStork(map->width-2,1,'4',TRUE);
    TIMER *timer = InitTimer(settings[2]);

    // basic gameloop
    LANE *lanes = InitLanes(map, settings);
    if(ch=='1')
    {
        LoadLevel(map,settings,lanes,"level1.txt");
    }
    else if(ch=='2')
    {
        LoadLevel(map,settings,lanes,"level2.txt");
    }
    else if(ch =='3')
    {
        LoadLevel(map,settings,lanes,"level3.txt");
    }
    wrefresh(stdwin);
    if(ch=='1'||ch=='2'||ch=='3')
    {
        if(LevelLoop(player,lanes,map,status,timer,&ch,settings))
        {
            YouWonLevel(status);
        }
        else
        {
            YouLostLevel(status);
        }
    }
    else
    {
        MainLoop(player,stork, map, status, timer, lanes, &ch, settings);
        if (ch == 'x')
        {
            Quit(status);
        }
        else
        {
            GameOver(status, timer);
        }
        HighScoreSet(highscore, timer->points);
    }

    flushinp();
    // freeing allocated memory
    FreeMemory(map, status, timer, player, lanes);
    endwin();
    return 0;
}
