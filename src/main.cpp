#define SDL_MAIN_USE_CALLBACKS 1
#define USE_TASKBAR_LEFT_POSITION false
#define WINDOW_WIDTH 250
#define WINDOW_HEIGHT 47

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <string>

#include "Sched_util.h"
#include "Text_util.h"

using json = nlohmann::json;
static SDL_Window *window = nullptr;
static SDL_Renderer *renderer = nullptr;

static int windowX = 0;
static int windowY = 0;
static int currentWinX;
static int currentWinY;
static TTF_Font* currentFont;
static SDL_Color fontColor = { 255, 255, 255, 255 };
static SDL_Color fontColorSeconds = { 255, 255, 255, 100 };
static json schedule = nullptr;
static std::string lastDayType = "";
static const char *lastEvent = "";
static int lastHoursLeft = 0;
static int lastMinsLeft = 0;
static int lastSecsLeft = 0;

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

    if (!SDL_CreateWindowAndRenderer("Crooms Bell Schedule", WINDOW_WIDTH, WINDOW_HEIGHT,
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

    cpr::Response r = Get(cpr::Url{"https://api.croomssched.tech/today"});
    if (r.status_code != 200) {
        SDL_Log("Failed to fetch schedule! Error: Server returned %s", std::to_string(r.status_code).c_str());
        return SDL_APP_FAILURE;
    }

    schedule = json::parse(r.text);

    if (schedule["status"] != "OK") {
        SDL_Log("Failed to fetch schedule! Error: Expected \"OK\" in JSON file status property, but got %s instead.",
            to_string(schedule["status"]).c_str());
        return SDL_APP_FAILURE;
    }

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

    const std::string dayType = to_string(schedule["data"]["msg"]);
    const char *event = Sched_GetCurrentEvent(schedule, false);
    const int hoursLeft = Sched_GetHrsLeft(schedule, false);
    const int minsLeft = Sched_GetMinsLeft(schedule, false);
    const int secsLeft = Sched_GetSecsLeft();

    RenderText(currentFont, "display.dayType", dayType.substr(1, dayType.length() - 2).c_str(), 10, 5, fontColor, 0.43f);

    SDL_FRect eventName = RenderText(currentFont,
        "display.classTimeLeft.eventName", (std::string(event) + ", Time Left: ").c_str(),
        10, 22, fontColor, 0.43f);



    const char* hrsMins;

    if (hoursLeft != 0) {
        hrsMins = (Sched_PadTime(hoursLeft, 2) + ":" +
            Sched_PadTime(minsLeft, 2)).c_str();
    } else {
        hrsMins = Sched_PadTime(minsLeft, 2).c_str();
    }

    SDL_FRect hrsMinsDimensions = RenderText(currentFont,
        "display.classTimeLeft.HrsMins", hrsMins, eventName.x + eventName.w, eventName.y, fontColor, 0.43f);

    const char* secs = (":" + Sched_PadTime(secsLeft, 2)).c_str();

    RenderText(currentFont, "display.classTimeLeft.Seconds", secs,
        hrsMinsDimensions.x + hrsMinsDimensions.w, hrsMinsDimensions.y, fontColorSeconds, 0.43f);


    SDL_RenderPresent(renderer);

    if (std::string(lastEvent) != std::string(event)) {
        DestroyText("display.classTimeLeft.eventName");
    }
    if (lastDayType != dayType) {
        DestroyText("display.dayType");
    }
    if (lastHoursLeft != hoursLeft) {
        DestroyText("display.classTimeLeft.HrsMins");
    }
    if (lastMinsLeft != minsLeft) {
        DestroyText("display.classTimeLeft.HrsMins");
    }
    if (lastSecsLeft != secsLeft) {
        DestroyText("display.classTimeLeft.Seconds");
    }

    lastEvent = event;
    lastDayType = dayType;
    lastHoursLeft = hoursLeft;
    lastMinsLeft = minsLeft;
    lastSecsLeft = secsLeft;

    SDL_Delay(200);
    return SDL_APP_CONTINUE;
}



void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    TTF_Quit();
}