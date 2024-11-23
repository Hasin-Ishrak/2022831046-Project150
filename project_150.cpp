#include<bits/stdc++.h>
#include<SDL2/SDL.h>
#include<time.h>

#define w 720
#define h 720
#define ts 20
#define ml 200

typedef struct{
    int x,y;
}point;

typedef struct{
    point sgmnts[ml];
    int l ,dx,dy;
}Snake;

bool init(SDL_Window** window ,SDL_Renderer** renderer){

    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        printf("Fail To Initialize:%s",SDL_GetError());
        return 0;
    }

    *window = SDL_CreateWindow("SNAKE GAME" ,SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED, w , h , SDL_WINDOW_SHOWN);
    
    if(*window==NULL){
        printf("Failed window:%s",SDL_GetError());
        return 0;
    }

    *renderer =SDL_CreateRenderer(*window ,-1 ,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if(*renderer == NULL){
        printf("Failed Renderer: %s",SDL_GetError());
        return 0;
    }

    return 1;
}

void rectngl(SDL_Renderer* renderer ,int x, int y , int wd ,int hg, SDL_Color color){

  SDL_SetRenderDrawColor (renderer,color.r ,color.g ,color.b ,color.a );
  SDL_Rect rect ={x ,y,wd,hg,};
  SDL_RenderFillRect(renderer ,&rect);
}

void circle(SDL_Renderer* renderer,int xx,int yy,int radi,SDL_Color color){

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for(int i=0;i<radi*2;i++){
        for(int j=0;j<radi*2;j++){
            int  dx=radi-i;
            int dy=radi-j;
            if((dx*dx +dy*dy)<=(radi*radi)){
                SDL_RenderDrawPoint(renderer,xx+dx,yy+dy);
            }
        }
    }
}

void movingsnk(Snake* snake){

    for(int i =snake->l-1; i>0; i--){
       snake->sgmnts[i]=snake->sgmnts[i-1];
    }

    snake->sgmnts[0].x += snake->dx;
    snake->sgmnts[0].y += snake->dy;
}

bool collision(point a ,point b){

    return a.x == b.x && a.y == b.y ;
}

bool selfcollision(Snake* snake){
    for(int i =1; i<snake->l;i++){
        if(collision(snake->sgmnts[0],snake->sgmnts[i])){
            return 1;
        }
    }
    return 0;
}

bool wallcollision(Snake* snake){

    return snake->sgmnts[0].x<0 || snake->sgmnts[0].x >= w/ts || snake->sgmnts[0].y<0 || snake->sgmnts[0].y >= h/ts;
}

void Sfood(point* food){

    food->x = rand() % (w/ts);
    food->y=rand() % (h/ts);
}

void bonus(point* bonus){

    bonus->x =rand() % (w/ts);
    bonus->y =rand() % (h/ts);
}

int main( int argc ,char* argv[]){

    SDL_Window* window =NULL;
    SDL_Renderer* renderer = NULL;

    if(!init(&window, &renderer)){
        return 1;
    }

    srand(time(NULL));

    Snake snake;
    snake.l=1;
    snake.sgmnts[0]={w/ts/2 ,h/ts/2};
    snake.dx=1;
    snake.dy=0;

    point food ,bonus ={-1 ,-1};
    Sfood(&food);

    int score=0;
    int foodcounter=0;

    Uint32 bonustime=0;
    int bonusvalu=10;
    
    bool run=1;
    SDL_Event event;

    while(run){
        Uint32 currenttime=SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            run = 0;
        } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_UP && snake.dy == 0) {
                snake.dx = 0;
                snake.dy = -1;
            } else if (event.key.keysym.sym == SDLK_DOWN && snake.dy == 0) {
                snake.dx = 0;
                snake.dy = 1;
            } else if (event.key.keysym.sym == SDLK_LEFT && snake.dx == 0) {
                snake.dx = -1;
                snake.dy = 0;
            } else if (event.key.keysym.sym == SDLK_RIGHT && snake.dx == 0) {
                snake.dx = 1;
                snake.dy = 0;
            }
        }
    }
        movingsnk(&snake);

    if (wallcollision(&snake) || selfcollision(&snake)) {
        printf("GAME OVER \n");
        printf("YOUR SCORE : %d\n", score);
        break;
    }

    if (collision(snake.sgmnts[0], food)) {
        snake.l++;
        score += 2;
        foodcounter++;
        Sfood(&food);

        if (foodcounter == 3) {
            Sfood(&bonus);
            bonustime = currenttime;
            bonusvalu = 5;
        }
    }

    if (bonus.x != -1) {
        if (currenttime - bonustime <= 5000) {
            if (collision(snake.sgmnts[0], bonus)) {
                score += bonusvalu;
                bonus.x = -1;
                bonus.y = -1;
            }
        } else {
            bonus.x = -1;
            bonus.y = -1;
        }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color snakeColor = {0, 255, 0, 255};
    for (int i = 0; i < snake.l; i++) {
        int radi= (i == 0) ? ts / 2 : ts / 2 - (i * 2 / snake.l);
        circle(renderer, snake.sgmnts[i].x * ts + ts / 2,snake.sgmnts[i].y * ts + ts / 2, radi, snakeColor);
    }

    SDL_Color foodColor = {255, 0, 0, 255};
    rectngl(renderer, food.x * ts, food.y * ts, ts, ts, foodColor);

    if (bonus.x != -1) {
        SDL_Color bonusColor = {255, 255, 0, 255};
        rectngl(renderer, bonus.x * ts, bonus.y * ts, ts, ts, bonusColor);
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(100);
}
  
    SDL_DestroyRenderer(renderer);

    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}



