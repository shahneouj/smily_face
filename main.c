#include <SDL3/SDL.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 200

#define SCALE 2

static uint32_t framebuffer[WINDOW_WIDTH * WINDOW_HEIGHT];

void put_pixel(int x, int y, uint32_t color) {
  if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT) {
    return;
  }

  framebuffer[y * WINDOW_WIDTH + x] = color;
}

void clear_framebuffer(uint32_t color) {
  for (int y = 0; y < WINDOW_HEIGHT; y++) {
    for (int x = 0; x < WINDOW_WIDTH; x++) {
      put_pixel(x, y, color);
    }
  }
}

void draw_smiley(int cx, int cy, int blink) {
  uint32_t yellow = 0x00FFFF00;
  uint32_t black = 0x00000000;

  int radius = 60;

  /*
   * -------------------------
   * Face
   * -------------------------
   */

  for (int y = cy - radius; y <= cy + radius; y++) {
    for (int x = cx - radius; x <= cx + radius; x++) {

      int dx = x - cx;
      int dy = y - cy;

      if (dx * dx + dy * dy <= radius * radius) {
        put_pixel(x, y, yellow);
      }
    }
  }

  /*
   * -------------------------
   * Eyes
   * -------------------------
   */

  if (!blink) {

    int eye_radius = 7;

    // Left eye
    for (int y = cy - 25; y <= cy - 10; y++) {
      for (int x = cx - 25; x <= cx - 10; x++) {

        int dx = x - (cx - 17);
        int dy = y - (cy - 17);

        if (dx * dx + dy * dy <= eye_radius * eye_radius) {

          put_pixel(x, y, black);
        }
      }
    }

    // Right eye
    for (int y = cy - 25; y <= cy - 10; y++) {
      for (int x = cx + 10; x <= cx + 25; x++) {

        int dx = x - (cx + 17);
        int dy = y - (cy - 17);

        if (dx * dx + dy * dy <= eye_radius * eye_radius) {

          put_pixel(x, y, black);
        }
      }
    }

  } else {

    /*
     * Closed eyes
     */

    for (int x = cx - 25; x <= cx - 10; x++) {
      put_pixel(x, cy - 17, black);
    }

    for (int x = cx + 10; x <= cx + 25; x++) {
      put_pixel(x, cy - 17, black);
    }
  }

  /*
   * -------------------------
   * Smile
   * -------------------------
   */

  for (int y = cy + 5; y <= cy + 35; y++) {
    for (int x = cx - 35; x <= cx + 35; x++) {

      int dx = x - cx;
      int dy = y - cy;

      int outer = dx * dx + dy * dy;
      int inner = dx * dx + (dy - 3) * (dy - 3);

      if (outer <= 35 * 35 && inner >= 28 * 28) {

        put_pixel(x, y, black);
      }
    }
  }
}

int main(void) {
  /*
   * Target:
   *
   * 60 frames per second
   *
   * 1 second / 60 frames
   * = 0.016666 seconds per frame
   */

  const double target_frame_time = 1.0 / 60.0;

  /*
   * -------------------------
   * SDL initialization
   * -------------------------
   */

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    printf("SDL_Init failed: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  /*
   * -------------------------
   * Create window
   * -------------------------
   */

  SDL_Window *window = SDL_CreateWindow(
      "SDL3 Framebuffer", WINDOW_WIDTH * SCALE, WINDOW_HEIGHT * SCALE, 0);

  if (window == NULL) {
    printf("SDL_CreateWindow failed: %s\n", SDL_GetError());

    SDL_Quit();
    return EXIT_FAILURE;
  }

  /*
   * -------------------------
   * Create renderer
   * -------------------------
   */

  SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

  if (renderer == NULL) {
    printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());

    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_FAILURE;
  }

  /*
   * -------------------------
   * Create texture
   * -------------------------
   */

  SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XRGB8888,
                                           SDL_TEXTUREACCESS_STREAMING,
                                           WINDOW_WIDTH, WINDOW_HEIGHT);

  if (texture == NULL) {
    printf("SDL_CreateTexture failed: %s\n", SDL_GetError());

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_FAILURE;
  }

  /*
   * -------------------------
   * Event system
   * -------------------------
   */

  SDL_Event event;

  uint8_t is_running = 1;

  /*
   * -------------------------
   * Main game loop
   * -------------------------
   */

  while (is_running) {

    /*
     * Start measuring this frame
     */

    uint64_t start = SDL_GetPerformanceCounter();

    /*
     * -------------------------
     * 1. Handle events
     * -------------------------
     */

    while (SDL_PollEvent(&event)) {

      if (event.type == SDL_EVENT_QUIT) {
        is_running = 0;
      }
    }

    /*
     * -------------------------
     * 2. Update
     * -------------------------
     */

    float time = SDL_GetTicks() / 1000.0f;

    /*
     * Make the smiley bounce.
     *
     * sin() gives:
     *
     * -1 ... 0 ... +1
     *
     * Multiplying by 20 gives:
     *
     * -20 ... 0 ... +20
     */

    int y = WINDOW_HEIGHT / 2 + (int)(sin(time * 3.0f) * 20.0f);

    /*
     * Blink for 0.15 seconds
     * every 4 seconds.
     */

    int blink = fmod(time, 4.0f) < 0.15f;

    /*
     * -------------------------
     * 3. Draw framebuffer
     * -------------------------
     */

    clear_framebuffer(0x00000000);

    draw_smiley(WINDOW_WIDTH / 2, y, blink);

    /*
     * -------------------------
     * 4. Upload framebuffer
     * -------------------------
     */

    SDL_UpdateTexture(texture, NULL, framebuffer,
                      WINDOW_WIDTH * sizeof(uint32_t));

    /*
     * -------------------------
     * 5. Render
     * -------------------------
     */

    SDL_RenderClear(renderer);

    /*
     * The framebuffer is:
     *
     * 320 x 200
     *
     * The window is:
     *
     * 640 x 400
     *
     * Therefore render the texture
     * at exactly 2x size.
     */

    SDL_FRect destination = {0.0f, 0.0f, WINDOW_WIDTH * SCALE,
                             WINDOW_HEIGHT * SCALE};

    SDL_RenderTexture(renderer, texture, NULL, &destination);

    SDL_RenderPresent(renderer);

    /*
     * -------------------------
     * 6. Frame timing
     * -------------------------
     */

    uint64_t end = SDL_GetPerformanceCounter();

    double elapsed =
        (double)(end - start) / (double)SDL_GetPerformanceFrequency();

    /*
     * If the frame finished faster
     * than 1/60 second, sleep for
     * the remaining time.
     */

    if (elapsed < target_frame_time) {

      uint32_t delay = (uint32_t)((target_frame_time - elapsed) * 1000.0);

      SDL_Delay(delay);
    }
  }

  /*
   * -------------------------
   * Cleanup
   * -------------------------
   */

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();

  return EXIT_SUCCESS;
}
