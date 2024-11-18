#include <ncurses.h>
#include <stdlib.h>

#define MAIN_COLOR 1

#define mapHeight 30
#define mapWidth 59
#define mapX 0
#define mapY 0

#define carCount 

typedef enum{
    player,
    car,
}OBJECTTYPE;

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
    OBJECTTYPE type;
    bool isHidden;
    bool isOverlapping;
}OBJECT;


//function initializng main window
WINDOW * Start(){
    WINDOW * stdwin;
    stdwin = initscr();


    start_color();
    init_pair(1,COLOR_WHITE,COLOR_GREEN);

    
    cbreak();
    curs_set(0);
    noecho();
    return stdwin;
}

OBJECT* InitPlayer(int y, int x,int maxY,int minX,int maxX,char shape){
    OBJECT * tempPlayer = (OBJECT*)malloc (sizeof(OBJECT));
    tempPlayer->y =y;
    tempPlayer->x =x;
    tempPlayer->maxY =maxY;
    tempPlayer->maxX =maxY;
    tempPlayer->minX =minX;
    tempPlayer->type = player;
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


int main(){
    WINDOW*stdwin = Start();
    WIN * map = InitWindow(stdwin,mapHeight,mapWidth,mapY,mapX);
    OBJECT * player = InitPlayer(mapHeight-2,mapWidth/2,mapHeight,0,mapWidth,'O');
    
    char ch;
    mvwprintw(map->win,0,3,"frogger nigger");

    wattron(map->win, COLOR_PAIR(1));
    mvwaddch(map->win,player->y,player->x,player->shape);
    wattroff(map->win,COLOR_PAIR(1));
    keypad(stdscr,TRUE);
    while(1){
        wrefresh(map->win);
        while (ch=wgetch(map->win))
        {
            mvwaddch(map->win,player->y,player->x,' ');
            if(ch == 'w'){
                player->y-=1;       
            }
            if(ch == 's'){
                player->y+=1;       
            }
            if(ch == 'a'){
                player->x-=1;       
            }
            if(ch == 'd'){
                player->x+=1;        
            }
            wattron(map->win, COLOR_PAIR(1));
            mvwaddch(map->win,player->y,player->x,player->shape);
            wattroff(map->win,COLOR_PAIR(1)); 
        }
    }


    endwin();
    return 0;
}