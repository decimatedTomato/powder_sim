#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include "handle_opengl.c"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define DEFAULT_WINDOW_WIDTH 100
#define DEFAULT_WINDOW_HEIGHT 100
// #define DEFAULT_WINDOW_WIDTH 640
// #define DEFAULT_WINDOW_HEIGHT 480

// USER DEFINITIONS
#define VERTEX_SHADER_FILE_PATH "res/shaders/vertex.glsl"
#define SHADER_FILE_PATH "res/shaders/fragment.glsl"
#define MAX_COLOR_COUNT 10

// DISPLAY
typedef enum DisplayMode { // As of yet unused
    PAUSED, RENDERING
} DisplayMode;

DisplayMode mode = RENDERING;
int window_width = DEFAULT_WINDOW_WIDTH;
int window_height = DEFAULT_WINDOW_HEIGHT;
bool is_fullscreen = false;

// Sand fall simulation variables
typedef enum Particle {
    EMPTY = 0x000000FF,
    SAND  = 0xEFEF00FF,
    WATER = 0x2020FFFF,
    SMOKE = 0x181818FF
} Particle;

Particle particle_grid[DEFAULT_WINDOW_HEIGHT][DEFAULT_WINDOW_WIDTH];

void init_grid() {
    for (size_t j = 0; j < DEFAULT_WINDOW_HEIGHT; j++) {
        for (size_t i = 0; i < DEFAULT_WINDOW_WIDTH; i++) {
            particle_grid[j][i] = ((i * j) % 2 == 0) ? EMPTY : SAND;
        }
    }
}

void init_texture() {
    GLuint texture_map;
    glGenTextures(1, &texture_map);
    glBindTexture(GL_TEXTURE_2D, texture_map);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, (uint32_t*)particle_grid);
    glBindTexture(GL_TEXTURE_2D, 0);
}

/* Updates a singular particle based on its type rules */
void update_particle(size_t c, size_t r) {
    switch (particle_grid[c][r]) {
        case EMPTY:
            break;
        case SAND:
            break;
        case WATER:
            break;
        case SMOKE:
            break;
    }
}

/* Update everything in simulation */
void update() {
    for (size_t i = 0; i < DEFAULT_WINDOW_HEIGHT; i++) {
        for (size_t j = 0; j < DEFAULT_WINDOW_WIDTH; j++) {
            update_particle(i, j);
        }
    }
}

int main() {
    long long unsigned int t = time(NULL);
    printf("Random seed %llu\n", t);
    srand(t);

    if(init_GLFW(window_width, window_height, "Simulation") == -1) exit(1);
    init_Debug_Callback();
    init_Quad();

    init_Shader(VERTEX_SHADER_FILE_PATH, SHADER_FILE_PATH);
    init_Uniforms();

    determine_palette();
    init_grid();
    init_texture();

    /* Loop until the user closes the window */
    bool keep_running = true;
    while (keep_running) {
        update();
        
        // if (mode != PAUSED) render();
        keep_running = render_frame();
        take_user_input();
    }
    clean_up();
    return 0;
}