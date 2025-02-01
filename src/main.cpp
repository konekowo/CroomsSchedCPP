#define SDL_MAIN_USE_CALLBACKS 1
#include <thread>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static SDL_Window *window = nullptr;
static SDL_Renderer *renderer = nullptr;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetHint(SDL_HINT_FORCE_RAISEWINDOW, "true");

    if (!SDL_CreateWindowAndRenderer("Crooms Bell Schedule", 250, 45,
                                     SDL_WINDOW_TRANSPARENT | SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_NOT_FOCUSABLE
                                     , &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    const SDL_DisplayMode *displayMode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(window));

    SDL_SetWindowPosition(window, *((int *) (&displayMode->w)) - 550, *((int *) (&displayMode->h)) - 45);

    SDL_RaiseWindow(window);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    switch (event->type) {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_TERMINATING:
            return SDL_APP_SUCCESS;
        default:;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    SDL_RaiseWindow(window);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(renderer, 0, 5, "Hello World!");

    SDL_RenderPresent(renderer);

    SDL_Delay(100);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
}