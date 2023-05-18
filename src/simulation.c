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

#define GRID_WIDTH 50
#define GRID_HEIGHT 50

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

uint32_t grid1[GRID_HEIGHT][GRID_WIDTH];
uint32_t grid2[GRID_HEIGHT][GRID_WIDTH];

uint32_t (*grid_current)[GRID_HEIGHT][GRID_WIDTH] = &grid1;
#define current_grid (*grid_current)
uint32_t (*grid_next)[GRID_HEIGHT][GRID_WIDTH] = &grid2;
#define next_grid (*grid_next)

uint32_t grid_saved[GRID_HEIGHT][GRID_WIDTH];

void swap_cell(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1) {
    int temp = current_grid[y0][x0];
    current_grid[y0][x0] = current_grid[y1][x1];
    current_grid[y1][x1] = temp;
}

// /* Changes the */
// void swap_grids() {
//     grid_current = next_grid;
//     grid_next    = (*grid_next == grid1) ? &grid2 : &grid1;
// }

void init_grid() {
    for (size_t j = 0; j < GRID_HEIGHT; j++) {
        for (size_t i = 0; i < GRID_WIDTH; i++) {
            current_grid[j][i] = EMPTY;
            int dx = i - GRID_WIDTH / 2;
            int dy = j - GRID_HEIGHT / 2;
            // if(dx * dx + dy * dy < 1600) particle_grid[j][i] = SAND;
            if(dx * dx + dy * dy <= GRID_HEIGHT * GRID_HEIGHT / 10) current_grid[j][i] = (j < GRID_HEIGHT / 2) ? SAND : WATER;
            // if(dx < 160 && dx > -160 && dy < 160 && dy > -160) particle_grid[j][i] = (j < GRID_HEIGHT / 2) ? SAND : WATER;
        }
    }
}

void copy_grid(uint32_t *grid_dest, uint32_t *grid_src) {
    for (size_t i = 0; i < GRID_WIDTH * GRID_HEIGHT; i++) {
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GRID_WIDTH, GRID_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, current_grid);
}

void update_sand(unsigned int x, unsigned int y) {
    if (y <= 0) return;
    if (current_grid[y-1][x] == EMPTY) {                               // Down
        swap_cell(x, y, x, y - 1);
    } else if (x > 0 && current_grid[y-1][x-1] == EMPTY) {             // Down left
        swap_cell(x, y, x - 1, y - 1);
    } else if (x < GRID_WIDTH && current_grid[y-1][x+1] == EMPTY) {    // Down right
        swap_cell(x, y, x + 1, y - 1);
    }
}

void update_water(unsigned int x, unsigned int y) {
    if (y <= 0) return;
    if (current_grid[y-1][x] == EMPTY) {                                       // Down
        swap_cell(x, y, x, y - 1);
    } else if (x < GRID_WIDTH && current_grid[y-1][x+1] == EMPTY) {  // Down right
        swap_cell(x, y, x + 1, y - 1);
    } else if (x > 0 && current_grid[y - 1][x - 1] == EMPTY) {                 // Down left
        swap_cell(x, y, x - 1, y - 1);
    } else if (x < GRID_WIDTH && current_grid[y][x+1] == EMPTY) {    // Right
        swap_cell(x, y, x + 1, y);
    } else if (x > 0 && current_grid[y][x-1] == EMPTY) {                       // Left
        swap_cell(x, y, x - 1, y);
    }
}


/* Updates a singular particle based on its type rules */
void update_particle(unsigned int c, unsigned int r) {
    switch (current_grid[r][c]) {
        case EMPTY:
            break;
        case SAND:
            update_sand(c, r); break;
        case WATER:
            update_water(c, r); break;
        case SMOKE:
            break;
    }
}

/* Update everything in simulation */
void update() {
    // for (int j = GRID_HEIGHT - 1; j >= 0; j--) {
    //     for (int i = GRID_WIDTH - 1; i >= 0; i--) {
    for (unsigned int i = 0; i < GRID_WIDTH; i++) {
        for (unsigned int j = 0; j < GRID_HEIGHT; j++) {
            update_particle(i, j);
        }
    }
    // swap_grids();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GRID_WIDTH, GRID_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, current_grid);
}

void load() {
    copy_grid((uint32_t*)current_grid, (uint32_t*)grid_saved);
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
    copy_grid((uint32_t*)grid_saved, (uint32_t*)current_grid);
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