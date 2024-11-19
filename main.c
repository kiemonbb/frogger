#include <ncurses.h>
#include <stdlib.h>

#define FROG_COLOR 1

#define mapHeight 30
#define mapWidth 30
#define mapX 0
#define mapY 0



//------------------------------------------------
//-------------------- STRUCTS -------------------
//------------------------------------------------

typedef struct{
    WINDOW * win;
    int height,width,x,y;
}WIN;

typedef struct{
    int x,y;
    int color;
    int velocity;
    int maxX, maxY, minX, minY;
    int height,width;
    char shape;
    int dir;
    bool isHidden;
    bool isOverlapping;
}OBJECT;

typedef struct{
    int x,y;
    int color;
    int maxX, maxY, minX, minY;
    int height,width;
    char shape;
}PLAYER;


//------------------------------------------------
//----------------- INIT FUNCTIONS ---------------
//------------------------------------------------

//function initializng main window
WINDOW * InitGame(){
    WINDOW * stdwin;
    stdwin = initscr();


    start_color();
    init_pair(FROG_COLOR,COLOR_WHITE,COLOR_GREEN);

    
    cbreak();
    curs_set(0);
    noecho();
    return stdwin;
}

PLAYER* InitPlayer(int y, int x,int minY,int maxY,int minX,int maxX,char shape){
    PLAYER * tempPlayer = (PLAYER*)malloc (sizeof(PLAYER));
    tempPlayer->y =y;
    tempPlayer->x =x;
    //min and max of frogs position
    tempPlayer->minY =minY;
    tempPlayer->maxY =maxY;
    tempPlayer->maxX =maxX;
    tempPlayer->minX =minX;
    tempPlayer->shape = shape;
    return tempPlayer;
}

//function initializng a subwindow
WIN * InitWindow(WINDOW*parent, int width,int height,int y, int x){
    WIN * tempWin = (WIN*)malloc(sizeof(WIN));
    tempWin->height=height;
    tempWin->width=width;
    tempWin->x=x;
    tempWin->y=y;
    tempWin->win= subwin(parent,width,height,y,x);
    box(tempWin->win,0,0);
    return tempWin;
}

void PlayerMovement(WINDOW*win,PLAYER*player, int ch,bool *isRunning){
        if(ch=getch()){
            mvwaddch(win,player->y,player->x,' ');
            if(ch == 'w' && player->y>player->minY){
                player->y-=1;       
            }
            if(ch == 's'&& player->y<player->maxY){
                player->y+=1;       
            }
            if(ch == 'a'&& player->x>player->minX){
                player->x-=1;       
            }
            if(ch == 'd' && player->x<player->maxX){
                player->x+=1;        
            }
            if(ch=='x'){
                *isRunning = false;
            }
            wattron(win, COLOR_PAIR(FROG_COLOR) | A_BOLD);
            mvwaddch(win,player->y,player->x,player->shape);
            wattroff(win,COLOR_PAIR(FROG_COLOR) | A_BOLD); 
        }
}


int main(){
    
    WINDOW*stdwin = InitGame();
    WIN * map = InitWindow(stdwin,mapHeight,mapWidth,mapY,mapX);
    PLAYER * player = InitPlayer(mapHeight-2,mapWidth/2,1,mapHeight-2,1,mapWidth-2,'O');
    

    mvwprintw(map->win,0,3,"frogger");
    wattron(map->win, COLOR_PAIR(FROG_COLOR) | A_BOLD);
    mvwaddch(map->win,player->y,player->x,player->shape);
    wattroff(map->win,COLOR_PAIR(FROG_COLOR) | A_BOLD); 

    //basic gameloop
    int ch;
    bool isRunning = true;
    while(isRunning){
        PlayerMovement(map->win,player,ch,&isRunning);
        wrefresh(map->win);

    }

    endwin();
    return 0;
}