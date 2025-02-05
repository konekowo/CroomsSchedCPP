#define SDL_MAIN_USE_CALLBACKS 1
#define USE_TASKBAR_LEFT_POSITION false
#define FETCH_TRIES 50

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

static float scale = 1.0f;
static int windowWidth = static_cast<int>(std::round(250 * scale));
static int windowHeight = static_cast<int>(std::round(47 * scale));
static int windowX = 0;
static int windowY = 0;
static int currentWinX;
static int currentWinY;
static int currentWinWidth;
static int currentWinHeight;
static int fetchTry = 0;
static int elipsesCount = 0;
static int elipsesTimer = 0;
static TTF_Font *currentFont;
static SDL_Color fontColor = {255, 255, 255, 255};
static SDL_Color fontColorSeconds = {255, 255, 255, 100};
static Schedule *schedule = nullptr;
static TextManager *textManager = nullptr;


void CalculateWindowPosAndSize(SDL_Window *window) {
    windowWidth = static_cast<int>(std::round(250 * scale));
    windowHeight = static_cast<int>(std::round(47 * scale));

    const SDL_DisplayMode *displayMode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(window));
#if (defined(USE_TASKBAR_LEFT_POSITION) && USE_TASKBAR_LEFT_POSITION == true)
    windowX = *const_cast<int *>(&displayMode->w) - 550;
#else
    windowX = 0;
#endif
    windowY = static_cast<int>(std::round(static_cast<float>(displayMode->h) - static_cast<float>(windowHeight)));
}

void FetchSchedule() {
    if (fetchTry > FETCH_TRIES) {
        SDL_Log("Failed to fetch schedule! Exceeded %d tries, exiting!", FETCH_TRIES);
        exit(1);
    }
    auto r = GetCallback([](const cpr::Response& res) {
        if (res.status_code != 200) {
            SDL_Log("Failed to fetch schedule! Error: Server returned %s", std::to_string(res.status_code).c_str());
            return FetchSchedule();
        }
        const auto jsonSchedule = json::parse(res.text);
        schedule = new Schedule(jsonSchedule);
        if (schedule->GetStatus() != "OK" && schedule != nullptr) {
            SDL_Log("Failed to fetch schedule! Error: Expected \"OK\" in JSON file status property, but got %s instead.",
                    schedule->GetStatus().c_str());
            delete schedule;
            schedule = nullptr;
            return FetchSchedule();
        }
    }, cpr::Url{"https://api.croomssched.tech/today"});
    fetchTry++;
}

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

    if (!SDL_CreateWindowAndRenderer("Crooms Bell Schedule", windowWidth, windowHeight,
                                     SDL_WINDOW_TRANSPARENT | SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP |
                                             SDL_WINDOW_NOT_FOCUSABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY,
                                     &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    currentFont = TTF_OpenFont("./assets/fonts/SegoeUI.ttf", 32);
    if (currentFont == nullptr) {
        SDL_Log("TTF_OpenFont() Error: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    textManager = new TextManager(renderer);

    scale = SDL_GetWindowDisplayScale(window);
    CalculateWindowPosAndSize(window);

    SDL_SetWindowSize(window, windowWidth, windowHeight);
    SDL_SetWindowPosition(window, windowX, windowY);

    if (std::string(SDL_GetPlatform()) == "Windows") {
        SDL_RaiseWindow(window);
    }

    FetchSchedule();

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
    if (std::string(SDL_GetPlatform()) == "Windows") {
        SDL_RaiseWindow(window);
    }

    scale = SDL_GetWindowDisplayScale(window);
    CalculateWindowPosAndSize(window);
    SDL_SetWindowSize(window, windowWidth, windowHeight);
    if (SDL_GetWindowPosition(window, &currentWinX, &currentWinY)) {
        if (currentWinX != windowX || currentWinY != windowY) {
            SDL_SetWindowPosition(window, windowX, windowY);
            SDL_Log("Window moved back to correct position");
        }
    }
    if (SDL_GetWindowSize(window, &currentWinWidth, &currentWinHeight)) {
        if (currentWinWidth != windowWidth || currentWinHeight != windowHeight) {
            SDL_SetWindowSize(window, windowWidth, windowHeight);
            SDL_Log("Window scaled to correct scale");
        }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    // ReSharper disable once CppUseStructuredBinding
    const SDL_FRect dimensions = textManager->RenderText(currentFont, "dimensionText", "A", INT32_MAX, INT32_MAX, fontColor, 0.43f * scale);

    if (schedule != nullptr) {
        const std::string dayType = schedule->GetData().msg;
        const std::string event = schedule->GetCurrentEvent() + ", Time Left: ";
        const int timeLeft = schedule->GetSecondsLeft();
        const int eventTime = schedule->GetEventSeconds();
        const int hoursLeft = timeLeft / 60 / 60;
        const int minsLeft = (timeLeft - hoursLeft * 60 * 60) / 60;
        const int secsLeft = timeLeft - minsLeft * 60 - hoursLeft * 60 * 60;
        SDL_Color progressBarColor = Schedule::CalculateProgressBarColor(timeLeft);

        const SDL_FRect dayTypeText = textManager->RenderText(currentFont, "display.dayType", dayType, 10, static_cast<float>(windowHeight) - 6 - dimensions.h * 2, fontColor, 0.43f * scale);

        // ReSharper disable once CppUseStructuredBinding
        const SDL_FRect eventName =
                textManager->RenderText(currentFont, "display.classTimeLeft.eventName",
                    event, 10, static_cast<float>(windowHeight) - 7 - dayTypeText.h, fontColor, 0.43f * scale);


        std::string hrsMins;

        if (hoursLeft != 0) {
            hrsMins = Schedule::PadTime(hoursLeft, 2) + ":" + Schedule::PadTime(minsLeft, 2);
        } else {
            hrsMins = Schedule::PadTime(minsLeft, 2);
        }

        // ReSharper disable once CppUseStructuredBinding
        const SDL_FRect hrsMinsDimensions =
                textManager->RenderText(currentFont, "display.classTimeLeft.HrsMins", hrsMins, eventName.x + eventName.w,
                                        eventName.y, fontColor, 0.43f * scale);

        const char *secs = (":" + Schedule::PadTime(secsLeft, 2)).c_str();

        textManager->RenderText(currentFont, "display.classTimeLeft.Seconds", secs,
                                hrsMinsDimensions.x + hrsMinsDimensions.w, hrsMinsDimensions.y, fontColorSeconds, 0.43f * scale);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
        const auto progressBarBG = SDL_FRect{0, static_cast<float>(windowHeight) - scale * 2, static_cast<float>(windowWidth), scale * 2};
        SDL_RenderFillRect(renderer, &progressBarBG);
        SDL_SetRenderDrawColor(renderer, progressBarColor.r, progressBarColor.g, progressBarColor.b, 255);
        const auto progressBar = SDL_FRect{0, static_cast<float>(windowHeight) - scale * 2,
            static_cast<float>(windowWidth) * (static_cast<float>(eventTime - timeLeft) / static_cast<float>(eventTime)), scale * 2 + 10};
        SDL_RenderFillRect(renderer, &progressBar);
    } else {
        std::string loadingText = "Fetching Schedule";
        for (int i = 0; i < elipsesCount; ++i) {
            loadingText += ".";
        }
        textManager->RenderText(currentFont, "display.loading", loadingText,
            10, static_cast<float>(windowHeight) - 7 - dimensions.h, fontColor, 0.43f * scale);
        elipsesTimer += 200;
        if (elipsesTimer > 400) {
            elipsesCount++;
            elipsesTimer = 0;
        }
        if (elipsesCount > 3) {
            elipsesCount = 0;
        }
    }


    SDL_RenderPresent(renderer);

    SDL_Delay(200);
    return SDL_APP_CONTINUE;
}


void SDL_AppQuit(void *appstate, SDL_AppResult result) { TTF_Quit(); }
