#define SDL_MAIN_USE_CALLBACKS 1
#define USE_TASKBAR_LEFT_POSITION false
#include <string>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "Text_util.h"


static SDL_Window *window = nullptr;
static SDL_Renderer *renderer = nullptr;

static int windowX = 0;
static int windowY = 0;
static int currentWinX;
static int currentWinY;
static TTF_Font* currentFont;
static SDL_Color fontColor = { 255, 255, 255, 255 };
static SDL_Color fontColorSeconds = { 255, 255, 255, 100 };

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!TTF_Init()) {
        SDL_Log("Couldn't initialize SDL_ttf: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetHint(SDL_HINT_FORCE_RAISEWINDOW, "true");
    SDL_SetHint(SDL_HINT_APP_NAME, "Crooms Bell Schedule");


    if (!SDL_CreateWindowAndRenderer("Crooms Bell Schedule", 250, 47,
                                     SDL_WINDOW_TRANSPARENT | SDL_WINDOW_BORDERLESS |
                                     SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_NOT_FOCUSABLE
                                     , &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    const SDL_DisplayMode *displayMode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(window));

#if (defined(USE_TASKBAR_LEFT_POSITION) && USE_TASKBAR_LEFT_POSITION == true)
    windowX = *const_cast<int *>(&displayMode->w) - 550;
#else
    windowX = 0;
#endif

    windowY = *const_cast<int *>(&displayMode->h) - 47;

    currentFont = TTF_OpenFont("./assets/fonts/SegoeUI.ttf", 32);
    if (currentFont == nullptr) {
        SDL_Log("TTF_OpenFont() Error: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    TextUtil_init(renderer);

    SDL_SetWindowPosition(window, windowX, windowY);

    SDL_RaiseWindow(window);

    SDL_Log("Successfully loaded!");
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
    if (SDL_GetWindowPosition(window, &currentWinX, &currentWinY)) {
        if (currentWinX != windowX && currentWinY != windowY) {
            SDL_SetWindowPosition(window, windowX, windowY);
            SDL_Log("Window moved back to correct position");
        }
    }
    SDL_SetWindowPosition(window, windowX, windowY);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    RenderText(currentFont, "display.dayType", "Today is a NORMAL day.", 10, 5, fontColor, 0.415f);
    SDL_FRect classTimeLeftDimensions = RenderText(currentFont,
        "display.classTimeLeft.HrsMins", "English, Time Left: 45", 10, 20, fontColor, 0.415f);
    RenderText(currentFont, "display.classTimeLeft.Seconds", ":20",
        classTimeLeftDimensions.x + classTimeLeftDimensions.w, classTimeLeftDimensions.y, fontColorSeconds, 0.415f);


    SDL_RenderPresent(renderer);

    SDL_Delay(200);
    return SDL_APP_CONTINUE;
}



void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    TTF_Quit();
}