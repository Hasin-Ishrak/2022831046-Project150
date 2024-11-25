#include <bits/stdc++.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define width 720
#define height 720
#define ts 25
#define ml 225

int highscore=0;

typedef struct {
    int x, y;
} point;

typedef struct {
    point sgmnts[ml];
    int l, dx, dy;
} Snake;

bool init(SDL_Window** window, SDL_Renderer** renderer) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Fail To Initialize: %s\n", SDL_GetError());
        return 0;
    }
    // to initialize ttf
    if (TTF_Init() == -1) {
        printf("Fail To Initialize TTF: %s\n", TTF_GetError());
        return 0;
    }
    //window creation
    *window = SDL_CreateWindow("The Serpent's Path", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);

    if (*window == NULL) {
        printf("Failed window: %s\n", SDL_GetError());
        return 0;
    }
     // to create snake,food,graphics
    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (*renderer == NULL) {
        printf("Failed Renderer: %s\n", SDL_GetError());
        return 0;
    }

    return 1;
}
 // destroy render ,ttf,window and  quit sdl
void killwindow(SDL_Window* window, SDL_Renderer* renderer) {

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}
 // to display score and texts in the game
 void displayText(SDL_Renderer* renderer,const char* message,int x,int y,int fontSize,SDL_Color color,bool center=0){
    TTF_Font* font =TTF_OpenFont("8bitOperatorPlus8-Bold.ttf",fontSize);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return;
    }
     // surface uses for raw pixel representation
    SDL_Surface* surface=TTF_RenderText_Solid(font,message,color);
    //convert the surface into texture
    SDL_Texture* texture=SDL_CreateTextureFromSurface(renderer,surface);

    SDL_Rect dstRect= {x,y,surface->w,surface->h};
    // define where will be the text
    // x ,y axis and w ,h height and width which is calculated from the surface
    //center ensures that is at center and half of w and half of h
    if (center) {
        dstRect.x= x-surface->w/2;
        dstRect.y= y-surface->h/2;
    }

    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
    // uses to frees the memory used
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
}
 // used to show current score
void drawscore(SDL_Renderer* renderer, int score, int depth){

    SDL_Color textColor = {0, 0, 0, 255};
    char scoretxt[32];
    //string for score and snprintf to handle overflow
    snprintf(scoretxt, sizeof(scoretxt), "Score: %d", score);
     
    int txtwidth =200; 
    //centered the text horizontally by substractinh half of width
    int x=(width/2)-(txtwidth/2);
    int y=depth/2+7;

    displayText(renderer,scoretxt,x,y,25,textColor,0);
}
 // gradiant effect on the background
void background(SDL_Renderer* renderer, int scoreBarHeight){
    for(int i =0;i<height;i++){
        //rgb increasing for effect and value controls the change of th color
        Uint8 r =75+(i*100/height); 
        Uint8 g =100+(i*50/height);    
        Uint8 b =150+(i*30/height);   
        SDL_SetRenderDrawColor(renderer,r,g,b,255);
        SDL_RenderDrawLine(renderer,0,i,width, i);
    }
}

// to show game over at the last of the game
void gameover(SDL_Renderer* renderer, int score) {
    SDL_SetRenderDrawColor(renderer,0,0,50, 255); 
    SDL_RenderClear(renderer);

    SDL_Color textColor = {255, 255, 255, 255}; 
    //horizontally centered and one third down on the screen
    displayText(renderer, "GAME OVER", width/2,height/3,48,textColor, 1);

    char scoreMessage[64];
    snprintf(scoreMessage, sizeof(scoreMessage), "Your Score: %d", score);
    // horizontally centered and under the game over
    displayText(renderer,scoreMessage, width/2,height/3+60,35,textColor,1);
    // to track high score and to show high score at the game over window
    char highestScoreMessage[64];
    snprintf(highestScoreMessage, sizeof(highestScoreMessage), "Highest Score: %d", highscore);
    displayText(renderer, highestScoreMessage, width/2,height/3 +120,30, textColor, 1);
    // to show ending message and centered the text 120 pixel below game over 
    displayText(renderer,"Press 'SPACE' to Restart or 'Shift' to Quit",width/2,height/3+180, 25, textColor,1);

    SDL_RenderPresent(renderer);
}
 // draw four wall as boundary 
void wall(SDL_Renderer* renderer,int depth,SDL_Color color){
     SDL_SetRenderDrawColor(renderer,color.r,color.g,color.b,color.a);

     SDL_Rect topWall={0,0,width,depth};
     SDL_RenderFillRect(renderer,&topWall);

    SDL_Rect bottomWall={0,height-depth,width,depth};
    SDL_RenderFillRect(renderer,&bottomWall);
    
    SDL_Rect leftWall={0,0,depth,height};
    SDL_RenderFillRect(renderer,&leftWall);
    
    SDL_Rect rightWall={width-depth,0,depth,height};
    SDL_RenderFillRect(renderer,&rightWall);
}
// for drawing filled rectangles for different purposes 
void rectngl(SDL_Renderer* renderer,int x,int y,int wd,int hg,SDL_Color color){

  SDL_SetRenderDrawColor (renderer,color.r ,color.g ,color.b ,color.a );
  SDL_Rect rect ={x,y,wd,hg,};
  SDL_RenderFillRect(renderer,&rect);
}
//for filled circle 
void circle(SDL_Renderer* renderer,int xx,int yy,int radi,SDL_Color color){

    SDL_SetRenderDrawColor(renderer,color.r,color.g,color.b,color.a);
    // nested loop for circle draw and formula is x^2 + y^2 = r^2
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
// creates static predifined rectangle blocks in the window 
void obstacles(SDL_Renderer* renderer, SDL_Color color, int depth){
    
    std::vector<point>obs={{10,10},{11,10},{12,10},{13,10},{14,10}};
    SDL_SetRenderDrawColor(renderer,color.r,color.g,color.b,color.a);
    for(const auto& i:obs){
        SDL_Rect rect = {i.x*ts,i.y*ts,ts,ts}; 
        SDL_RenderFillRect(renderer, &rect);
    }
}
// updates the snakes position after every movement of the snake
void movingsnk(Snake* snake){
    // moves every segments of the snake starting from the tail
    for(int i =snake->l-1; i>0; i--){
       snake->sgmnts[i]=snake->sgmnts[i-1];
    }
    // update the head position of the snake
    snake->sgmnts[0].x+=snake->dx;
    snake->sgmnts[0].y+=snake->dy;
}
// this checks that two points are occupy the same grid or not for(food,wall,obstacle,self,bonus)
bool collision(point a ,point b){

    return a.x == b.x && a.y == b.y ;
}
//checks if the snake bites it self or not
bool selfcollision(Snake* snake){
    for(int i =1;i<snake->l;i++){
        if(collision(snake->sgmnts[0],snake->sgmnts[i])){
            return 1;
        }
    }
    return 0;
}
// checks if the snake hits the wall or not 
bool wallcollision(Snake* snake, int depth) {
    int headx=snake->sgmnts[0].x;
    int heady=snake->sgmnts[0].y;
    // checks the head is less then or below the wall thickness or not
    return headx<depth/ts || headx>=(width-depth)/ts ||
           heady<depth/ts || heady>=(height-depth)/ts;
}
//checks if the snake hits any of the obstacles or not
bool obscollision(Snake* snake){

    std::vector<point>obs={{10,10},{11,10},{12,10},{13,10},{14,10}};
    for(const auto& i:obs){
        if(collision(snake->sgmnts[0],i)){
            return 1;
        }
    }
    return 0;
}
// this functions generate the food and bonus position for the snake 
void foodandbonus(point* item, Snake* snake, int depth){
    //finds the best position for food and do not collide with any other obstacles or boundary 
    std::vector<point>obs= {{10,10},{11,10},{12,10},{13,10},{14,10}};
    std::vector<point>freesp;
    // stores all the free locations of the grid 
    for(int i =depth/ts;i<(height-depth)/ts;i++){
        for(int j=depth/ts;j<(width-depth)/ts;j++){
            bool occu=0;
            // checks if the snake occupied any grid for the food 
            for(int k=0;k<snake->l;k++){
               if (snake->sgmnts[k].x==i && snake->sgmnts[k].y== j){
                    occu = 1;
                    break;
            }
        }
        for(const auto& o:obs){
            if(o.x==i && o.y==j){
                occu=1;
                break;
            }
        }
        if (!occu) {
                freesp.push_back({i,j});
         }
     }
  }
  // radnomly select free grid for food
   if (!freesp.empty()) {
        int ind = rand() % freesp.size();
        *item = freesp[ind];
    } else {
        item->x=-1;
        item->y=-1;
    }
}

// after one game if user press space then this function fully reset the game as new  
void reset(Snake *snake,point *food,point *bonus,int *score,int *foodCounter,Uint32 *bonusTime,bool *running){
    snake->l=2; //initialize the snake length
    snake->sgmnts[0]={width/ts/2,height/ts/2}; // head of the snake
    snake->sgmnts[1]={width/ts/2-1,height/ts/2};// tail of the snake
    snake->dx=1; //initial movement direction to right
    snake->dy=0;

     foodandbonus(food,snake,10); // for the first food in the new game
    *bonus={-1 -1}; // because no bonus is corrently active
    *score= 0;       // reset score
    *foodCounter=0; //reset food counter
    *bonusTime=0; //reset bonus time
    *running=1; 
}
// checks movement of the snake and if the user wants to resart or quit the game or not 
void directionhandle(bool* run, bool* restart, Snake* snake, point* food, point* bonus, int* score, int* foodCounter, 
                        Uint32* bonusTime, SDL_Window* window, SDL_Renderer* renderer){
    SDL_Event event;
    while (SDL_PollEvent(&event)){
        if (event.type==SDL_QUIT){
            *run=0;
        } 
        else if (event.type==SDL_KEYDOWN){
            if (event.key.keysym.sym==SDLK_UP && snake->dy==0){
                snake->dx=0;
                snake->dy=-1;
            } 
            else if (event.key.keysym.sym==SDLK_DOWN && snake->dy==0){
                snake->dx=0;
                snake->dy=1;
            } 
            else if (event.key.keysym.sym== SDLK_LEFT && snake->dx==0){
                snake->dx=-1;
                snake->dy=0;
            } 
            else if (event.key.keysym.sym==SDLK_RIGHT && snake->dx==0){
                snake->dx=1;
                snake->dy=0;
            } 
            else if (event.key.keysym.sym==SDLK_SPACE){
                *restart=1; 
                *run=0;   
            } 
            else if (event.key.keysym.sym==SDLK_LSHIFT || event.key.keysym.sym==SDLK_RSHIFT){
                *run=0;     
            }
        }
    }
}

int main (int argc, char* argv[]){
    SDL_Window* window=NULL;
    SDL_Renderer* renderer=NULL;

    if (!init(&window, &renderer)){
        return 1;
    }

    srand(time(NULL));
    bool running=1;
    bool restart=0;

    Snake snake;
    point food,bonus;
    int score=0, foodCounter=0;
    Uint32 bonusTime= 0;

    int depth = 10;  
    SDL_Color wallColor={0,51,51,255};  
    SDL_Color obstacleColor={32,32,32,255};

    reset(&snake,&food,&bonus,&score,&foodCounter,&bonusTime,&running);
   
   //outer loop keep the game running unless player quits 
    //inner loop for single game until the player losses or restart
    while (running) {
        while (running) {
            Uint32 currentTime=SDL_GetTicks(); //current time in millisecond 

            directionhandle(&running,&restart,&snake,&food,&bonus,&score,&foodCounter,&bonusTime,window,renderer);

            movingsnk(&snake);

            if (wallcollision(&snake,depth) || selfcollision(&snake) || obscollision(&snake)) {
                break; 
            }
            // checks if the head of the snake i collide with the food or not 
            //and increases the length and generate food and bonus for logic
            if (collision(snake.sgmnts[0], food)){
                snake.sgmnts[snake.l]=snake.sgmnts[snake.l-1];
                snake.l++;
                score += 10;
                foodCounter++;
                foodandbonus(&food,&snake,depth);

                if (foodCounter%3==0) {
                    foodandbonus(&bonus,&snake,depth);
                    bonusTime=currentTime;
                }
            }

            if (bonus.x!=-1) {
                //checks if the bonus is for 5 sec  since it comes 
                if (currentTime-bonusTime<=5000){
                    if (collision(snake.sgmnts[0],bonus)){
                        score+=50;
                        bonus={-1,-1};
                    }
                } 
                // if bonus exceeded the time then it will disappear after 5 sec
                else 
                {
                    bonus ={-1,-1};
                }
            }
            // render backgound ,wall and obstacles of the game
            background(renderer, depth);
            wall(renderer, depth, wallColor);
            obstacles(renderer, obstacleColor, depth);
           // making the snake 
            SDL_Color snakeColor={0,0,105,255};
            for (int i = 0;i<snake.l;i++){
                int radi=ts/2+3; // ensures the circle fits in the grid
                //i=0 is the head of the snake and it has different color then the body
                if(i==0){
                    SDL_Color head={75,0,155,255};
                    circle(renderer,snake.sgmnts[i].x*ts+ts/2,snake.sgmnts[i].y*ts+ts/2,radi,head);
                } 
                else{
                    SDL_Color body={48,95,0,255};
                    circle(renderer,snake.sgmnts[i].x*ts+ts/2,snake.sgmnts[i].y*ts+ts/2,radi,body);
                }
            }

            SDL_Color foodColor = {255,0,0,255};

            rectngl(renderer,food.x *ts,food.y *ts,ts,ts,foodColor);

           // circle(renderer,food.x*ts+ts/2,food.y*ts+ts/2,ts/2,foodColor);

            if (bonus.x !=-1) {
                SDL_Color bonusColor = {255,255,0,255};

                //circle(renderer,bonus.x*ts+ts/2,bonus.y*ts+ts/2,ts/2,bonusColor);

                rectngl(renderer,bonus.x*ts,bonus.y*ts,ts,ts,bonusColor);
            }

            drawscore(renderer,score,depth);
            SDL_RenderPresent(renderer);
            SDL_Delay(100);
        }
        if(score>highscore){
            highscore=score;
        }
        gameover(renderer,score);

        while (!restart && running) {
            directionhandle(&running,&restart,&snake,&food,&bonus,&score,&foodCounter,&bonusTime,window,renderer);
        }

        if (restart) {
            reset(&snake,&food,&bonus,&score,&foodCounter,&bonusTime,&running);
            restart=0;
        }
    }
    
    killwindow(window,renderer);
    return 0;
}