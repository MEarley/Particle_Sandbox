#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <unistd.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

int xMouse, yMouse, numParticles;
char windowGrid[SCREEN_WIDTH][SCREEN_HEIGHT];
char p_Material = 's';
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Surface *window_surface = NULL;

typedef struct particle_X{
    SDL_Rect sand_D;
    struct particle_X *next_s;
    char material;
} particle;

int init_display(void);
void close_display(void);
particle *makeParticle(int x, int y, char material);
particle *appendParticle(particle *totalParticles, particle *onePar);
void displayParticles(particle *totalParticles);
void Gravity(particle *totalParticles);
int fillGrid(void); 
void occupyPosition(int x, int y, char material);
void unoccupyPosition(int x, int y);
particle *spawnRectange(int xMouse, int yMouse, char material, particle *totalParticles);
int renderPixel(particle *singleParticle);
particle *particleSearch(int x, int y, particle *Particles);

//Based on the particles material, it will be displayed with the appropriate color
int renderPixel(particle *singleParticle){
    switch (singleParticle->material){
        case 's':
            SDL_SetRenderDrawColor(renderer,255,255,0,255);
            break;
        case 'd':
            SDL_SetRenderDrawColor(renderer,160,82,4,255);
            break;
        case 'w':
            SDL_SetRenderDrawColor(renderer,0,40,255,255);
            break;
        default:
            return 1;
            break;
        }
    SDL_RenderFillRect(renderer, &singleParticle->sand_D);
    return 0;
}

//Fills Virtual Grid with empty spaces, so particles know that they are not occupied
int fillGrid(void){
    for(int row = 0;row < SCREEN_HEIGHT;row++){
        for(int column = 0;column < SCREEN_WIDTH;column++){
            windowGrid[column][row] = ' ';
        }
    }

    return 0;
}

//Spawns a 30 x 30 rectangle
particle *spawnRectange(int xMouse, int yMouse, char material, particle *totalParticles){
    int newParticles = 0; // Keeps count of the number of new particles summoned
    //creates a particle within a x * y grid
    for(int row = 0;row < 30;row++){
        for(int column = 0;column < 30;column++){
            if(windowGrid[xMouse + column][yMouse + row] != material){
                totalParticles = appendParticle(totalParticles, makeParticle(xMouse + column, yMouse + row, p_Material));
                newParticles++;
            }
        }
    }
    printf("%d new particles were spawned\n%d particles could not fit...\n",newParticles,900 - newParticles);
    return totalParticles;
}

//Reserves a position on an abstract grid of the entire screen
void occupyPosition(int x, int y, char material){
    windowGrid[x][y] =  material;
    return;
}

//Unreservers a position on an abstract grid to allow other particles to take its place
void unoccupyPosition(int x, int y){
    windowGrid[x][y] = ' ';
    return;
}

//Searches for a specific particle based on its coordinates
particle *particleSearch(int x, int y, particle *Particles){
    while(!(Particles->next_s == NULL)){
        if(Particles->sand_D.x == x && Particles->sand_D.y == y){
            return Particles;
        }
        
        Particles = Particles->next_s;
    }

    printf("Error: Non-exisiting water detected");
}

//The physics engine
void Gravity(particle *Particles){
    int numUpdated = 0;
    particle *totalParticles = Particles;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);   
    SDL_RenderClear(renderer);
    while(!(Particles->next_s == NULL)){

    //Detects Collision with either ground or other particles
        // Basic particle physics (Sand, Dirt)
        if(Particles->sand_D.y < SCREEN_HEIGHT - 1 && windowGrid[Particles->sand_D.x][Particles->sand_D.y + 1] == ' '){
            //Updates grid and moves particle on screen to the new position for each particle
            unoccupyPosition(Particles->sand_D.x,Particles->sand_D.y);
            Particles->sand_D.y++;
            occupyPosition(Particles->sand_D.x,Particles->sand_D.y, Particles->material);
            numUpdated++;

        } else if(windowGrid[Particles->sand_D.x][Particles->sand_D.y + 1] == 'w' && Particles->material != 'w'){
            
            
            particle *foundParticle = particleSearch(Particles->sand_D.x,Particles->sand_D.y + 1, totalParticles);
            unoccupyPosition(foundParticle->sand_D.x,foundParticle->sand_D.y);
            unoccupyPosition(Particles->sand_D.x,Particles->sand_D.y);
            
            foundParticle->sand_D.y = Particles->sand_D.y;
            Particles->sand_D.y++;

            occupyPosition(foundParticle->sand_D.x,foundParticle->sand_D.y,foundParticle->material);
            occupyPosition(Particles->sand_D.x,Particles->sand_D.y,Particles->material);
            
            numUpdated += 2;
        } else if(Particles->sand_D.y < SCREEN_HEIGHT - 1 && Particles->sand_D.x > 0 &&  windowGrid[Particles->sand_D.x - 1][Particles->sand_D.y + 1] == ' '
            ){ //If falling down first fails proceeds to fall to the side [Only if the particle does not collide with the wall or another particle (left side priority)]
            //printf("Moved Left\n");
            //Updates grid and moves particle on screen to the new position for each particle
            unoccupyPosition(Particles->sand_D.x,Particles->sand_D.y);
            Particles->sand_D.x--;
            Particles->sand_D.y++;
            occupyPosition(Particles->sand_D.x,Particles->sand_D.y,Particles->material);
            numUpdated++;
        } else if(Particles->sand_D.y < SCREEN_HEIGHT - 1 && Particles->sand_D.x < SCREEN_WIDTH - 1 && windowGrid[Particles->sand_D.x + 1][Particles->sand_D.y + 1] == ' '
            ){ //If falling down first fails proceeds to fall to the side [Only if the particle does not collide with the wall or another particle (left side priority)]
            //printf("Moved Right\n");
            //Updates grid and moves particle on screen to the new position for each particle
            unoccupyPosition(Particles->sand_D.x,Particles->sand_D.y);
            Particles->sand_D.x++;
            Particles->sand_D.y++;
            occupyPosition(Particles->sand_D.x,Particles->sand_D.y,Particles->material);
            numUpdated++;
        }
        // Additional rules for liquids (Water)
        else if(Particles->sand_D.x > 0 && windowGrid[Particles->sand_D.x - 1][Particles->sand_D.y] == ' ' && Particles->material == 'w'){
            //Moves left if available and all else fails (left side priority)
            unoccupyPosition(Particles->sand_D.x,Particles->sand_D.y);
            Particles->sand_D.x--;
            occupyPosition(Particles->sand_D.x,Particles->sand_D.y,Particles->material);
            numUpdated++;
        }
        else if(Particles->sand_D.x < SCREEN_WIDTH - 1 && windowGrid[Particles->sand_D.x + 1][Particles->sand_D.y] == ' ' && Particles->material == 'w'){
            //Moves right if available and all else fails (left side priority)
            unoccupyPosition(Particles->sand_D.x,Particles->sand_D.y);
            Particles->sand_D.x++;
            occupyPosition(Particles->sand_D.x,Particles->sand_D.y,Particles->material);
            numUpdated++;
        }
        //Draws each indivdual particle
        renderPixel(Particles);
        Particles = Particles->next_s;
    }
    
    if(numUpdated > 0 ){
        printf("Updated %d particles...\n",numUpdated);
    }
}

void displayParticles(particle *Particles){
    int numDisplayed = 0;
    if(Particles->next_s == NULL){return;}
    while(!(Particles->next_s == NULL)){
        //Uses the renderPixel function to draw each pixel
        renderPixel(Particles);
        Particles = Particles->next_s;
        numDisplayed++;
    }
    printf("Displayed %d particles...\n", numDisplayed);
    return;
}

particle *appendParticle(particle *Particles, particle *onePar){
    //Connects a particle to the linked list by putting it at the front
    onePar->next_s = Particles;
    return onePar;
}

particle *makeParticle(int x, int y, char material){
    //Occupies a position on the grid, then creates a 1x1 pixel
    occupyPosition(x, y, material);
    particle *Particle = malloc(sizeof(particle));
    Particle->sand_D = (SDL_Rect){x, y, 1, 1};
    Particle->next_s = NULL;
    Particle->material = material;
    numParticles++;
    return Particle;
}

int init_display(void){
    //Initialize
    
    if( SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("SDL could not be initialized. SDL_Error: %s\n",SDL_GetError());
        return 1;
    }
    //Create an SDL Window
    window = SDL_CreateWindow("Sandbox Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    if( window == NULL){
        printf("Window could not be created. SDL_Error: %s\n",SDL_GetError());
        return 1;
    }
    //Create a renderer
    renderer= SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    window_surface = SDL_GetWindowSurface(window);

    SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
    return 0;

}

void close_display(void){
    //Close display
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    window = NULL;
    SDL_Quit();
}

int main(int argc, char **argv){
    int Update = 0;
    int clk = 0;
    
    windowGrid[2][2] = 's';
    printf("%c | %d WINDOW\n",windowGrid[2][2], windowGrid[2][2] == 's');
    fillGrid();
    printf("%c | %d WINDOW\n",windowGrid[2][2], windowGrid[2][2] == 's');

    particle *totalParticles = malloc(sizeof(particle));
    totalParticles->sand_D = (SDL_Rect){0, 0, 0, 0};
    totalParticles->next_s = NULL;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);   
    SDL_RenderClear(renderer);
    
    if(init_display() == 1){ 
        sleep(3); 
        return 0;
    }

    int running = 1;
    SDL_Event event;
    while(running == 1){
        
        //Process Events
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT){
                running = 0;
            }

            if(event.type == SDL_MOUSEBUTTONDOWN){
                SDL_GetMouseState(&xMouse, &yMouse);
                printf("x: %d, y: %d\n",xMouse, yMouse);
                
                if(windowGrid[xMouse][yMouse] == ' '){
                    totalParticles = appendParticle(totalParticles, makeParticle(xMouse, yMouse, p_Material));
                    Update = 1;
                } else { printf("Occupied Space\n");}
            }

            if(event.type == SDL_KEYDOWN){
                //Checks for 's' input on keyboard to summon a square of pixels
                SDL_GetMouseState(&xMouse, &yMouse);
                printf("x: %d, y: %d\n",xMouse, yMouse);
                switch(event.key.keysym.sym){
                    case SDLK_s :
                        printf("Spawning block of particles...\n");
                        totalParticles = spawnRectange(xMouse, yMouse, p_Material, totalParticles);
                        Update = 1;
                        break;
                    case SDLK_d :
                        p_Material = 'd';
                        break;
                    case SDLK_p :
                        p_Material = 's';
                        break;
                    case SDLK_w :
                        p_Material = 'w';
                        break;
                    default:
                        break;
                }
            }
        }

        //Display on update
        if(Update == 1){
            displayParticles(totalParticles);
            Update = 0;
        }
        SDL_RenderPresent(renderer); 
        
        SDL_Delay(100);
        
        //Updates Gravity based on delay separate from the system
        clk += 1000;
        if(clk >= 1000){
            Gravity(totalParticles);
            clk = 0;
        }
    }
    printf("Closing Program...\n");
    close_display();  
    return 0;
}

