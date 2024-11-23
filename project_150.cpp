#include <bits/stdc++.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define width 720
#define height 720
#define ts 20
#define ml 200

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
    if (TTF_Init() == -1) {
        printf("Fail To Initialize TTF: %s\n", TTF_GetError());
        return 0;
    }

    *window = SDL_CreateWindow("SNAKE GAME", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);

    if (*window == NULL) {
        printf("Failed window: %s\n", SDL_GetError());
        return 0;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (*renderer == NULL) {
        printf("Failed Renderer: %s\n", SDL_GetError());
        return 0;
    }

    return 1;
}

void killwindow(SDL_Window* window, SDL_Renderer* renderer) {

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

 void displayText(SDL_Renderer* renderer, const char* message, int x, int y, int fontSize, SDL_Color color, bool center = false) {
    TTF_Font* font = TTF_OpenFont("8bitOperatorPlus8-Bold.ttf", fontSize);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Solid(font, message, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect dstRect = {x, y, surface->w, surface->h};

    if (center) {
        dstRect.x = x - surface->w / 2;
        dstRect.y = y - surface->h / 2;
    }

    SDL_RenderCopy(renderer, texture, NULL, &dstRect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
}

void scorebar(SDL_Renderer* renderer, int score){
    int scorebarheight = 40;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0); 
    SDL_Rect scoreBarRect = {0, height - scorebarheight, width, scorebarheight};
    SDL_RenderFillRect(renderer, &scoreBarRect);

    SDL_Color textColor = {0, 0, 0, 255}; 
    char scoreText[32];
    snprintf(scoreText, sizeof(scoreText), "Score: %d", score);
    displayText(renderer, scoreText, 10, height - scorebarheight / 2, 24, textColor, false); 

}

void background(SDL_Renderer* renderer, int scoreBarHeight){
    for(int i =0;i<height;i++){
        Uint8 r = 72 + (i * 100 / height); 
        Uint8 g = 97 + (i * 50 / height);    
        Uint8 b = 150 + (i * 30 / height);   
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderDrawLine(renderer, 0, i, width, i);
    }
}


void gameover(SDL_Renderer* renderer, int score) {
    SDL_SetRenderDrawColor(renderer, 105, 105, 105, 255); 
    SDL_RenderClear(renderer);

    SDL_Color textColor = {255, 255, 255, 255}; 

    displayText(renderer, "GAME OVER", width / 2, height / 3, 48, textColor, true);

    char scoreMessage[64];
    snprintf(scoreMessage, sizeof(scoreMessage), "Your Score: %d", score);
    displayText(renderer, scoreMessage, width/ 2, height / 3 + 60, 36, textColor, true);

    displayText(renderer, "Press 'SPACE' to Restart or 'Q' to Quit", width / 2, height / 3 + 120, 24, textColor, true);

    SDL_RenderPresent(renderer);
}

void wall(SDL_Renderer* renderer, int depth, SDL_Color color){
     SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
     //upper wall
     SDL_Rect topWall = {0, 0, width, depth};
     SDL_RenderFillRect(renderer, &topWall);
     // lower wall
     SDL_Rect bottomWall = {0, height - depth, width, depth};
    SDL_RenderFillRect(renderer, &bottomWall);
    // Left wall
    SDL_Rect leftWall = {0, 0, depth, height};
    SDL_RenderFillRect(renderer, &leftWall);
    // Right wall
    SDL_Rect rightWall = {width - depth, 0, depth, height};
    SDL_RenderFillRect(renderer, &rightWall);
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

bool wallcollision(Snake* snake, int depth) {
    int headx = snake->sgmnts[0].x;
    int heady = snake->sgmnts[0].y;

    return headx < depth / ts || headx >= (width - depth) / ts ||
           heady < depth / ts || heady >= (height - depth) / ts;
}

void generateFood(point* food, Snake* snake, int depth) {
    bool validPosition = false;

    while (!validPosition) {
        food->x = rand() % ((width - 2 * depth) / ts) + (depth / ts);
        food->y = rand() % ((height - 2 * depth) / ts) + (depth / ts);

        validPosition = true;
        for (int i = 0; i < snake->l; i++) {
            if (collision(snake->sgmnts[i], *food)) {
                validPosition = false;
                break;
            }
        }
    }
}

void generateBonus(point* bonus, Snake* snake, int depth) {
    bool validPosition = false;

    while (!validPosition) {
       
        bonus->x = rand() % ((width - 2 * depth) / ts) + (depth / ts);
        bonus->y = rand() % ((height - 2 * depth) / ts) + (depth / ts);
        validPosition = true;
        for (int i = 0; i < snake->l; i++) {
            if (collision(snake->sgmnts[i], *bonus)) {
                validPosition = false;
                break;
            }
        }
    }
}

void reset(Snake *snake, point *food, point *bonus, int *score, int *foodCounter, Uint32 *bonusTime, bool *running){
    snake->l=1;
    snake->sgmnts[0] = {width / ts / 2, height / ts / 2};
    snake->dx = 1;
    snake->dy = 0;

     generateFood(food, snake, 10);
    *bonus = {-1, -1};
    *score = 0;
    *foodCounter = 0;
    *bonusTime = 0;
    *running = 1;
}

void directionhandle(bool* run, bool* restart, Snake* snake, point* food, point* bonus, int* score, int* foodCounter, 
                        Uint32* bonusTime, SDL_Window* window, SDL_Renderer* renderer) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            *run = false;
        } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_UP && snake->dy == 0) {
                snake->dx = 0;
                snake->dy = -1;
            } else if (event.key.keysym.sym == SDLK_DOWN && snake->dy == 0) {
                snake->dx = 0;
                snake->dy = 1;
            } else if (event.key.keysym.sym == SDLK_LEFT && snake->dx == 0) {
                snake->dx = -1;
                snake->dy = 0;
            } else if (event.key.keysym.sym == SDLK_RIGHT && snake->dx == 0) {
                snake->dx = 1;
                snake->dy = 0;
            } else if (event.key.keysym.sym == SDLK_SPACE) {
                *restart = true; 
                *run = false;   
            } else if (event.key.keysym.sym == SDLK_q) {
                *run = false;     
            }
        }
    }
}


int main (int argc, char* argv[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    if (!init(&window, &renderer)) {
        return 1;
    }

    srand(time(NULL));
    bool running = true;
    bool restart = false;

    Snake snake;
    point food, bonus;
    int score = 0, foodCounter = 0;
    Uint32 bonusTime = 0;

    int depth = 10;  
    SDL_Color wallColor = {0, 0, 0, 255};  

    reset(&snake, &food, &bonus, &score, &foodCounter, &bonusTime, &running);

    while (running) {
        while (running) {
            Uint32 currentTime = SDL_GetTicks();

            
            directionhandle(&running, &restart, &snake, &food, &bonus, &score, &foodCounter, &bonusTime, window, renderer);

            movingsnk(&snake);

            if (wallcollision(&snake,depth) || selfcollision(&snake)) {
                break; 
            }

            if (collision(snake.sgmnts[0], food)) {
                snake.l++;
                score += 5;
                foodCounter++;
                generateFood(&food, &snake, depth);

                if ( foodCounter %3==0) {
                    generateBonus(&bonus, &snake, depth);
                    bonusTime = currentTime;
                }
            }

            if (bonus.x != -1) {
                if (currentTime - bonusTime <= 5000) {
                    if (collision(snake.sgmnts[0], bonus)) {
                        score += 10;
                        bonus = {-1, -1};
                    }
                } else {
                    bonus = {-1, -1};
                }
            }

            int scorebarheight = 50;
            background(renderer, scorebarheight);
            wall(renderer, depth, wallColor);

            SDL_Color snakeColor = {0, 0, 102, 255};
            for (int i = 0; i < snake.l; i++) {
                int radius = (i == 0) ? ts / 2 : ts / 2 - (i * 2 / snake.l);
                circle(renderer, snake.sgmnts[i].x * ts + ts / 2, snake.sgmnts[i].y * ts + ts / 2, radius, snakeColor);
            }

            SDL_Color foodColor = {255, 0, 0, 255};
            circle(renderer, food.x * ts + ts / 2, food.y * ts + ts / 2, ts / 2, foodColor);

            if (bonus.x != -1) {
                SDL_Color bonusColor = {255, 255, 0, 255};
                circle(renderer, bonus.x * ts + ts / 2,bonus.y * ts + ts / 2, ts / 2,bonusColor);
            }

            scorebar(renderer, score);
            SDL_RenderPresent(renderer);
            SDL_Delay(100);
        }

        gameover(renderer, score);

        while (!restart && running) {
            directionhandle(&running, &restart, &snake, &food, &bonus, &score, &foodCounter, &bonusTime, window, renderer);
        }

      
        if (restart) {
            reset(&snake, &food, &bonus, &score, &foodCounter, &bonusTime, &running);
            restart = false;
        }
    }

    
    killwindow(window, renderer);
    return 0;
}
// fafter scoring 60 there is no food comming

// want humanise food places 