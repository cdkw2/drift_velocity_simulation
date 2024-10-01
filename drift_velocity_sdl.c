#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 700
#define SIMULATION_WIDTH 800
#define SIMULATION_HEIGHT 600
#define NUM_ELECTRONS 200
#define NUM_IONS 50
#define ELECTRON_RADIUS 3
#define ION_RADIUS 6
#define MAX_SPEED 8.0
#define COLLISION_DAMPING 0.8

#define CONTROL_PANEL_WIDTH 190
#define CONTROL_PANEL_HEIGHT 230
#define CONTROL_PANEL_X (WINDOW_WIDTH - CONTROL_PANEL_WIDTH )
#define CONTROL_PANEL_Y (WINDOW_HEIGHT - CONTROL_PANEL_HEIGHT )

typedef struct {
    float x, y, vx, vy;
    int radius;
    SDL_Color color;
} Particle;

typedef struct {
    SDL_Rect rect;
    float min, max, value;
    const char* label;
} Slider;

typedef struct {
    SDL_Rect rect;
    const char* label;
} Button;

Particle electrons[NUM_ELECTRONS];
Particle ions[NUM_IONS];
Slider sliders[3];
Button resetButton;
TTF_Font* font;

float maxSpeed = MAX_SPEED;
float collisionDamping = COLLISION_DAMPING;
int numElectrons = NUM_ELECTRONS;

float calculateAverageVelocity() {
    float totalVelocity = 0;
    for (int i = 0; i < numElectrons; i++) {
        totalVelocity += sqrt(electrons[i].vx * electrons[i].vx + electrons[i].vy * electrons[i].vy);
    }
    return totalVelocity / numElectrons;
}

void initializeParticles() {
    for (int i = 0; i < NUM_ELECTRONS; i++) {
        electrons[i].x = rand() % SIMULATION_WIDTH;
        electrons[i].y = rand() % SIMULATION_HEIGHT;
        electrons[i].vx = ((float)rand() / RAND_MAX - 0.5) * maxSpeed;
        electrons[i].vy = ((float)rand() / RAND_MAX - 0.5) * maxSpeed;
        electrons[i].radius = ELECTRON_RADIUS;
        electrons[i].color = (SDL_Color){0, 0, 255, 255};
    }

    for (int i = 0; i < NUM_IONS; i++) {
        ions[i].x = rand() % SIMULATION_WIDTH;
        ions[i].y = rand() % SIMULATION_HEIGHT;
        ions[i].vx = ions[i].vy = 0;
        ions[i].radius = ION_RADIUS;
        ions[i].color = (SDL_Color){255, 0, 0, 255};
    }
}

void initializeSliders() {
    sliders[0] = (Slider){
        .rect = {CONTROL_PANEL_X + 5, CONTROL_PANEL_Y + 30, 180, 20},
        .min = 1.0, .max = 15.0, .value = MAX_SPEED,
        .label = "Max Speed"
    };
    sliders[1] = (Slider){
        .rect = {CONTROL_PANEL_X + 5, CONTROL_PANEL_Y + 80, 180, 20},
        .min = 0.1, .max = 1.0, .value = COLLISION_DAMPING,
        .label = "Collision Damping"
    };
    sliders[2] = (Slider){
        .rect = {CONTROL_PANEL_X + 5, CONTROL_PANEL_Y + 130, 180, 20},
        .min = 50, .max = 500, .value = NUM_ELECTRONS,
        .label = "Number of Electrons"
    };
}

void initializeResetButton() {
    resetButton = (Button){
        .rect = {CONTROL_PANEL_X + 5, CONTROL_PANEL_Y + 180, 180, 30},
        .label = "Reset Simulation"
    };
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
        vx1 = vx2 * collisionDamping;
        vx2 = temp * collisionDamping;

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

void moveParticles() {
    for (int i = 0; i < numElectrons; i++) {
        electrons[i].x += electrons[i].vx;
        electrons[i].y += electrons[i].vy;

        electrons[i].x = fmodf(electrons[i].x + SIMULATION_WIDTH, SIMULATION_WIDTH);
        electrons[i].y = fmodf(electrons[i].y + SIMULATION_HEIGHT, SIMULATION_HEIGHT);

        for (int j = 0; j < NUM_IONS; j++) {
            handleCollision(&electrons[i], &ions[j]);
        }

        for (int j = i + 1; j < numElectrons; j++) {
            handleCollision(&electrons[i], &electrons[j]);
        }
    }
}

void drawParticles(SDL_Renderer* renderer) {
    for (int i = 0; i < numElectrons; i++) {
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

void drawSliders(SDL_Renderer* renderer) {
    SDL_Color textColor = {255, 255, 255, 255};
    for (int i = 0; i < 3; i++) {
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderFillRect(renderer, &sliders[i].rect);

        SDL_Rect handle = {
            sliders[i].rect.x + (int)((sliders[i].value - sliders[i].min) / (sliders[i].max - sliders[i].min) * sliders[i].rect.w) - 5,
            sliders[i].rect.y - 5,
            10,
            sliders[i].rect.h + 10
        };
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderFillRect(renderer, &handle);

        SDL_Surface* textSurface = TTF_RenderText_Solid(font, sliders[i].label, textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = {sliders[i].rect.x, sliders[i].rect.y - 25, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        char valueText[20];
        snprintf(valueText, 20, "%.2f", sliders[i].value);
        textSurface = TTF_RenderText_Solid(font, valueText, textColor);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        textRect = (SDL_Rect){sliders[i].rect.x + sliders[i].rect.w + 10, sliders[i].rect.y, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }
}

void drawResetButton(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderFillRect(renderer, &resetButton.rect);

    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, resetButton.label, textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    
    SDL_Rect textRect = {
        resetButton.rect.x + (resetButton.rect.w - textSurface->w) / 2,
        resetButton.rect.y + (resetButton.rect.h - textSurface->h) / 2,
        textSurface->w,
        textSurface->h
    };
    
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void drawControlPanel(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 200);
    SDL_Rect controlPanel = {CONTROL_PANEL_X, CONTROL_PANEL_Y, CONTROL_PANEL_WIDTH, CONTROL_PANEL_HEIGHT};
    SDL_RenderFillRect(renderer, &controlPanel);

    drawSliders(renderer);

    drawResetButton(renderer);

    char avgVelText[50];
    snprintf(avgVelText, 50, "Avg Velocity: %.2f", calculateAverageVelocity());
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, avgVelText, textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {CONTROL_PANEL_X - 30, CONTROL_PANEL_Y - 30, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}


void updateSliders(SDL_Event* e) {
    if (e->type == SDL_MOUSEBUTTONDOWN || (e->type == SDL_MOUSEMOTION && e->motion.state & SDL_BUTTON_LMASK)) {
        int mouseX = e->button.x;
        int mouseY = e->button.y;
        for (int i = 0; i < 3; i++) {
            if (mouseX >= sliders[i].rect.x && mouseX <= sliders[i].rect.x + sliders[i].rect.w &&
                mouseY >= sliders[i].rect.y && mouseY <= sliders[i].rect.y + sliders[i].rect.h) {
                float newValue = sliders[i].min + (mouseX - sliders[i].rect.x) * (sliders[i].max - sliders[i].min) / sliders[i].rect.w;
                sliders[i].value = fmaxf(sliders[i].min, fminf(sliders[i].max, newValue));
                
                if (i == 0) maxSpeed = sliders[i].value;
                else if (i == 1) collisionDamping = sliders[i].value;
                else if (i == 2) numElectrons = (int)sliders[i].value;
            }
        }
    }
}

bool isMouseOverButton(int mouseX, int mouseY, SDL_Rect* buttonRect) {
    return mouseX >= buttonRect->x && mouseX <= buttonRect->x + buttonRect->w &&
           mouseY >= buttonRect->y && mouseY <= buttonRect->y + buttonRect->h;
}

void handleResetButton(SDL_Event* e) {
    if (e->type == SDL_MOUSEBUTTONDOWN) {
        int mouseX = e->button.x;
        int mouseY = e->button.y;
        if (isMouseOverButton(mouseX, mouseY, &resetButton.rect)) {
            initializeParticles();
            maxSpeed = MAX_SPEED;
            collisionDamping = COLLISION_DAMPING;
            numElectrons = NUM_ELECTRONS;
            sliders[0].value = MAX_SPEED;
            sliders[1].value = COLLISION_DAMPING;
            sliders[2].value = NUM_ELECTRONS;
        }
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

    if (TTF_Init() == -1) {
        SDL_Log("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Interactive Electron and Ion Collision Simulation",
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

    font = TTF_OpenFont("/usr/share/fonts/TTF/OpenSans-Bold.ttf", 16);
    if (font == NULL) {
        SDL_Log("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return 1;
    }

    initializeParticles();
    initializeSliders();
    initializeResetButton();

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            updateSliders(&e);
            handleResetButton(&e);
        }

        moveParticles();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        drawParticles(renderer);
        drawControlPanel(renderer);

        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
