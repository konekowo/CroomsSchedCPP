#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <thread>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>


/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = nullptr;
static SDL_Renderer *renderer = nullptr;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Example Renderer Clear", "1.0", "com.example.renderer-clear");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("examples/renderer/clear", 250, 45,
        SDL_WINDOW_TRANSPARENT | SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP
        , &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    const SDL_DisplayMode *displayMode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(window));

    SDL_SetWindowPosition(window, *((int*)(&displayMode->w)) - 550, *((int*)(&displayMode->h)) - 45);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

    /* clear the window to the draw color. */
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(renderer, 0, 0, "Hello World!");

    /* put the newly-cleared rendering on the screen. */
    SDL_RenderPresent(renderer);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}
