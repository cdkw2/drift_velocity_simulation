#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define NUM_ELECTRONS 200
#define NUM_IONS 50
#define ELECTRON_RADIUS 3
#define ION_RADIUS 6
#define MAX_SPEED 3.0
#define COLLISION_DAMPING 0.8

typedef struct {
    float x, y, vx, vy;
    int radius;
    SDL_Color color;
} Particle;

Particle electrons[NUM_ELECTRONS];
Particle ions[NUM_IONS];

float distance(float x1, float y1, float x2, float y2) {
    return sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

void initializeParticles() {
    for (int i = 0; i < NUM_ELECTRONS; i++) {
        electrons[i].x = rand() % WINDOW_WIDTH;
        electrons[i].y = rand() % WINDOW_HEIGHT;
        electrons[i].vx = ((float)rand() / RAND_MAX - 0.5) * MAX_SPEED;
        electrons[i].vy = ((float)rand() / RAND_MAX - 0.5) * MAX_SPEED;
        electrons[i].radius = ELECTRON_RADIUS;
        electrons[i].color = (SDL_Color){0, 0, 255, 255};
    }

    for (int i = 0; i < NUM_IONS; i++) {
        ions[i].x = rand() % WINDOW_WIDTH;
        ions[i].y = rand() % WINDOW_HEIGHT;
        ions[i].vx = ions[i].vy = 0;
        ions[i].radius = ION_RADIUS;
        ions[i].color = (SDL_Color){255, 0, 0, 255}; // Red
    }
}

void handleCollision(Particle* p1, Particle* p2) {
    float dx = p2->x - p1->x;
    float dy = p2->y - p1->y;
    float distance = sqrt(dx*dx + dy*dy);
    
    if (distance < p1->radius + p2->radius) {
        float angle = atan2(dy, dx);
        float sin_angle = sin(angle);
        float cos_angle = cos(angle);

        float vx1 = p1->vx * cos_angle + p1->vy * sin_angle;
        float vy1 = -p1->vx * sin_angle + p1->vy * cos_angle;
        float vx2 = p2->vx * cos_angle + p2->vy * sin_angle;
        float vy2 = -p2->vx * sin_angle + p2->vy * cos_angle;

        float temp = vx1;
        vx1 = vx2 * COLLISION_DAMPING;
        vx2 = temp * COLLISION_DAMPING;

        p1->vx = vx1 * cos_angle - vy1 * sin_angle;
        p1->vy = vx1 * sin_angle + vy1 * cos_angle;
        p2->vx = vx2 * cos_angle - vy2 * sin_angle;
        p2->vy = vx2 * sin_angle + vy2 * cos_angle;

        float overlap = (p1->radius + p2->radius - distance) / 2;
        p1->x -= overlap * cos_angle;
        p1->y -= overlap * sin_angle;
        p2->x += overlap * cos_angle;
        p2->y += overlap * sin_angle;
    }
}

// collision shit

void moveParticles() {
    for (int i = 0; i < NUM_ELECTRONS; i++) {
        electrons[i].x += electrons[i].vx;
        electrons[i].y += electrons[i].vy;

        electrons[i].x = fmodf(electrons[i].x + WINDOW_WIDTH, WINDOW_WIDTH);
        electrons[i].y = fmodf(electrons[i].y + WINDOW_HEIGHT, WINDOW_HEIGHT);

        for (int j = 0; j < NUM_IONS; j++) {
            handleCollision(&electrons[i], &ions[j]);
        }

        for (int j = i + 1; j < NUM_ELECTRONS; j++) {
            handleCollision(&electrons[i], &electrons[j]);
        }
    }
}

void drawParticles(SDL_Renderer* renderer) {
    for (int i = 0; i < NUM_ELECTRONS; i++) {
        SDL_SetRenderDrawColor(renderer, electrons[i].color.r, electrons[i].color.g, electrons[i].color.b, electrons[i].color.a);
        SDL_Rect rect = {
            (int)electrons[i].x - electrons[i].radius,
            (int)electrons[i].y - electrons[i].radius,
            electrons[i].radius * 2,
            electrons[i].radius * 2
        };
        SDL_RenderFillRect(renderer, &rect);
    }

    for (int i = 0; i < NUM_IONS; i++) {
        SDL_SetRenderDrawColor(renderer, ions[i].color.r, ions[i].color.g, ions[i].color.b, ions[i].color.a);
        SDL_Rect rect = {
            (int)ions[i].x - ions[i].radius,
            (int)ions[i].y - ions[i].radius,
            ions[i].radius * 2,
            ions[i].radius * 2
        };
        SDL_RenderFillRect(renderer, &rect);
    }
}

int main(int argc, char* argv[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Electron and Ion Collision Simulation",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        SDL_Log("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    initializeParticles();

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        moveParticles();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        drawParticles(renderer);

        SDL_RenderPresent(renderer);

        SDL_Delay(16);  // Cap at roughly 60 fps
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
