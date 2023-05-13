#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

// Portability issues
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

// Image stuff
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

// My opengl functions file
#include "handle_opengl.c"

// USER DEFINITIONS
#define TARGET_IMAGE "res/Image/rina.jpeg"
#define DEFAULT_WINDOW_WIDTH SCREEN_WIDTH/3
#define DEFAULT_WINDOW_HEIGHT SCREEN_HEIGHT/3
// #define DEFAULT_WINDOW_WIDTH 640
// #define DEFAULT_WINDOW_HEIGHT 480

#define VERTEX_SHADER_FILE_PATH "res/shaders/vertex_old.glsl"
#define SHADER_FILE_PATH "res/shaders/fragment_old.glsl"

#define START_PAUSED true

// DISPLAY
typedef enum DisplayMode {
    PLAYING, PAUSED
} DisplayMode;

DisplayMode mode = START_PAUSED;
int window_width = DEFAULT_WINDOW_WIDTH;
int window_height = DEFAULT_WINDOW_HEIGHT;
bool is_fullscreen = false;

// Sand fall simulation variables
typedef enum Particle {
    // AA BB GG RR remember that alpha is unused unless I start making use of blending
    EMPTY = 0xFF000000,
    SAND  = 0xFF00EFEF,
    WATER = 0xFFFF2020,
    SMOKE = 0xFF202020
} Particle;

uint32_t particle_grid[DEFAULT_WINDOW_HEIGHT][DEFAULT_WINDOW_WIDTH];
uint32_t particle_grid_saved[DEFAULT_WINDOW_HEIGHT][DEFAULT_WINDOW_WIDTH];

void swap_cell(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1) {
    int temp = particle_grid[y0][x0];
    particle_grid[y0][x0] = particle_grid[y1][x1];
    particle_grid[y1][x1] = temp;
}

void init_grid() {
    for (size_t j = 0; j < DEFAULT_WINDOW_HEIGHT; j++) {
        for (size_t i = 0; i < DEFAULT_WINDOW_WIDTH; i++) {
            particle_grid[j][i] = EMPTY;
            int dx = i - DEFAULT_WINDOW_WIDTH / 2;
            int dy = j - DEFAULT_WINDOW_HEIGHT / 2;
            // if(dx * dx + dy * dy < 1600) particle_grid[j][i] = SAND;
            // if(dx * dx + dy * dy < 1600) particle_grid[j][i] = (j < DEFAULT_WINDOW_HEIGHT / 2) ? SAND : WATER;
            if(dx < 160 && dx > -160 && dy < 160 && dy > -160) particle_grid[j][i] = (j < DEFAULT_WINDOW_HEIGHT / 2) ? SAND : WATER;
        }
    }
}

void copy_grid(uint32_t *grid_dest, uint32_t *grid_src) {
    for (size_t i = 0; i < DEFAULT_WINDOW_WIDTH * DEFAULT_WINDOW_HEIGHT; i++) {
        grid_dest[i] = grid_src[i];
    }
}

void load_image_texture(int slot) {
    stbi_set_flip_vertically_on_load(1);
    int width, height, channels;
    unsigned char *img = stbi_load(TARGET_IMAGE, &width, &height, &channels, 0);
    // printf("width %d height %d channels %d\n", width, height, channels);
    if (img == NULL) {
        printf("Error loading the image\n");
        exit(1);
    }

    GLuint texture_map;
    glBindTexture(GL_TEXTURE_2D, texture_map);
    glGenTextures(1, &texture_map);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glActiveTexture(GL_TEXTURE0 + slot);
    // Made for jpeg (3 channels)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
    stbi_image_free(img);
}

void init_texture() {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glActiveTexture(GL_TEXTURE0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, particle_grid);
}

/* Updates a singular particle based on its type rules */
void update_particle(size_t c, size_t r) {
    switch (particle_grid[r][c]) {
        case EMPTY:
            break;
        case SAND:
            // FALL
            if (r <= 0) break;
            if (particle_grid[r-1][c] == EMPTY) {                                       // Down
                swap_cell(c, r, c, r - 1);
            } else if (c > 0 && particle_grid[r-1][c-1] == EMPTY) {                     // Down left
                swap_cell(c, r, c - 1, r - 1);
            } else if (c < DEFAULT_WINDOW_WIDTH && particle_grid[r-1][c+1] == EMPTY) {  // Down right
                swap_cell(c, r, c + 1, r - 1);
            }
           break;
        case WATER:
            if (r <= 0) break;
            if (particle_grid[r-1][c] == EMPTY) {                                       // Down
                swap_cell(c, r, c, r - 1);
            } else if (c < DEFAULT_WINDOW_WIDTH && particle_grid[r-1][c+1] == EMPTY) {  // Down right
                swap_cell(c, r, c + 1, r - 1);
            } else if (c > 0 && particle_grid[r - 1][c - 1] == EMPTY) {                 // Down left
                swap_cell(c, r, c - 1, r - 1);
            } else if (c < DEFAULT_WINDOW_WIDTH && particle_grid[r][c+1] == EMPTY) {    // Right
                swap_cell(c, r, c + 1, r);
            } else if (c > 0 && particle_grid[r][c-1] == EMPTY) {                       // Left
                swap_cell(c, r, c - 1, r);
            }
            break;
        case SMOKE:
            break;
    }
}

/* Update everything in simulation */
void update() {
    // for (int j = DEFAULT_WINDOW_HEIGHT - 1; j >= 0; j--) {
    //     for (int i = DEFAULT_WINDOW_WIDTH - 1; i >= 0; i--) {
    for (size_t i = 0; i < DEFAULT_WINDOW_WIDTH; i++) {
        for (size_t j = 0; j < DEFAULT_WINDOW_HEIGHT; j++) {
            update_particle(i, j);
        }
    }
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, particle_grid);
}

void load() {
    copy_grid((uint32_t*)particle_grid, (uint32_t*)particle_grid_saved);
    render_frame();
}

void pause() {
    if (mode == PAUSED) mode = PLAYING;
    else if (mode == PLAYING) mode = PAUSED;
}

void step() {
    mode = PAUSED;
    update();
    render_frame();
}

void save() {
    copy_grid((uint32_t*)particle_grid_saved, (uint32_t*)particle_grid);
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

    init_grid();
    init_texture();

    /* Loop until the user closes the window */
    bool keep_running = true;
    while (keep_running) {
        take_user_input();
        keep_running = render_frame();
        if (mode != PAUSED) update();
    }

    clean_up();
    // glDeleteTextures(1, ???);
    return 0;
}

// Unused plan for steady framerate
// initial
    // int fps = 30;
    // double desiredDelta = 1.0 / fps;
    // time_t time_start, time_end;
    // time(&time_start);
    // double deltaTime;

// in loop
        // time(&time_end);
        // deltaTime = difftime(time_start, time_end);
        // printf("%lf\n", deltaTime);
        // time(&time_start);