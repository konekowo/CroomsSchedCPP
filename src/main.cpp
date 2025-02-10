#define SDL_MAIN_USE_CALLBACKS 1
#define USE_TASKBAR_LEFT_POSITION false
#define SETTINGS_FILE_PATH "./settings.json"
#define SCHEDULE_JSON_URL "https://api.croomssched.tech/today"
#define FETCH_TRIES 50
#define USE_LARGE_TEXT false

#if USE_LARGE_TEXT == true
#define BELL_FONT_SIZE 0.75f
#else
#define BELL_FONT_SIZE 0.43f
#endif

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <string>

#include "Schedule.h"
#include "Settings.h"
#include "TextManager.h"

using json = nlohmann::json;

static const auto GMT_OFFSET = std::chrono::hours(-5);

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
static bool isFetching = false;
static Settings *settings;
static TTF_Font *currentFont;
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

void FetchSchedule0() {
    if (fetchTry > FETCH_TRIES) {
        SDL_Log("Failed to fetch schedule! Exceeded %d tries, exiting!", FETCH_TRIES);
        exit(1);
    }
    auto r = GetCallback([](const cpr::Response& res) {
        if (res.status_code != 200) {
            SDL_Log("Failed to fetch schedule! Error: Server returned %s", std::to_string(res.status_code).c_str());
            return FetchSchedule0();
        }
        const auto jsonSchedule = json::parse(res.text);
        schedule = new Schedule(jsonSchedule, settings);
        if (schedule->GetStatus() != "OK" && schedule != nullptr) {
            SDL_Log("Failed to fetch schedule! Error: Expected \"OK\" in JSON file status property, but got %s instead.",
                    schedule->GetStatus().c_str());
            delete schedule;
            schedule = nullptr;
            return FetchSchedule0();
        }
        isFetching = false;
    }, cpr::Url{SCHEDULE_JSON_URL});
    fetchTry++;
}

void FetchSchedule() {
    if (!isFetching) {
        isFetching = true;
        FetchSchedule0();
    }
}


SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    settings = new Settings(SETTINGS_FILE_PATH);

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

    currentFont = TTF_OpenFont(settings->fontLocation.c_str(), 32);
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
    SDL_WindowID windowID = SDL_GetWindowID(window);
    if (settings != nullptr) {
        if (settings->isSettingsOpen()) {
            settings->PollEvent(event);
        }
    }
    if (event->window.windowID == windowID) {
        switch (event->type) {
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (settings != nullptr) {
                    settings->OpenSettings();
                    settings->RaiseWindow();
                }
            return SDL_APP_CONTINUE;
            case SDL_EVENT_QUIT:
            case SDL_EVENT_TERMINATING:
                return SDL_APP_SUCCESS;
            default:;
        }
    }
    return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppIterate(void *appstate) {
    if (std::string(SDL_GetPlatform()) == "Windows") {
        if (settings != nullptr) {
            if (!settings->SettingsWindowHasFocus()){
                SDL_RaiseWindow(window);
            }
        } else {
            SDL_RaiseWindow(window);
        }
    }

    SDL_Color fontColor;
    switch (settings->theme) {
        case LIGHT:
            fontColor = {0, 0, 0, 255};
            break;
        case DARK:
        default:
            fontColor = {255, 255, 255, 255};
            break;
    }

    if (schedule != nullptr && !isFetching) {
        const auto resTime = std::chrono::duration_cast<std::chrono::days>(schedule->GetResponseTime());
        // give the server 5 seconds to make sure it returns the correct schedule
        if (const auto now =
                    std::chrono::duration_cast<std::chrono::days>(
                    std::chrono::system_clock::now().time_since_epoch() + GMT_OFFSET - std::chrono::seconds(5) );
            now > resTime) {
            SDL_Log("Current Schedule is outdated. Fetching new schedule!");
            delete schedule;
            schedule = nullptr;
            FetchSchedule();
        }
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

    if (schedule != nullptr && !isFetching) {
        const int timeLeft = schedule->GetSecondsLeft();
        const int totalEventTime = schedule->GetEventSeconds();
        const std::string scheduleEventName = schedule->GetCurrentEvent();

        float percentage = ((static_cast<float>(totalEventTime) - static_cast<float>(timeLeft)) / static_cast<float>(totalEventTime)) * 100;
        const std::string dayType = schedule->GetData().msg;
        // TODO: add setting to hide percentage
        std::string event = scheduleEventName + ", Time Left: ";
        if (settings->showPercentage) {
            event = std::format("{:.2f}", percentage) + "% - " + event;
        }
        const int hoursLeft = timeLeft / 60 / 60;
        const int minLeft = (timeLeft - hoursLeft * 60 * 60) / 60;
        const int secsLeft = timeLeft - minLeft * 60 - hoursLeft * 60 * 60;
        const SDL_Color schedColor = schedule->CalculateTextColor(timeLeft);

#if USE_LARGE_TEXT == true
        const SDL_FRect eventName = {5, 0, 0, 0};
        const SDL_FRect dayTypeText = {};
#else
        // ReSharper disable once CppUseStructuredBinding
        const SDL_FRect dayTypeText = textManager->RenderText(currentFont, "display.dayType", dayType, 10, static_cast<float>(windowHeight) - 6 - dimensions.h * 2, schedColor, 0.43f * scale);

        // ReSharper disable once CppUseStructuredBinding

        const SDL_FRect eventName =
                textManager->RenderText(currentFont, "display.classTimeLeft.eventName",
                    event, 10, static_cast<float>(windowHeight) - 7 - dayTypeText.h, schedColor, 0.43f * scale);
#endif


        std::string hrsMins;

        if (hoursLeft != 0) {
            hrsMins = Schedule::PadTime(hoursLeft, 2) + ":" + Schedule::PadTime(minLeft, 2);
        } else {
            hrsMins = Schedule::PadTime(minLeft, 2);
        }

        // ReSharper disable once CppUseStructuredBinding
        const SDL_FRect hrsMinsDimensions =
                textManager->RenderText(currentFont, "display.classTimeLeft.HrsMins", hrsMins, eventName.x + eventName.w,
                                        eventName.y, schedColor, BELL_FONT_SIZE * scale);

        const char *secs = (":" + Schedule::PadTime(secsLeft, 2)).c_str();
        if (settings->showSeconds) {
            textManager->RenderText(currentFont, "display.classTimeLeft.Seconds", secs,
                                    hrsMinsDimensions.x + hrsMinsDimensions.w, hrsMinsDimensions.y, {schedColor.r, schedColor.g, schedColor.b, 100}, BELL_FONT_SIZE * scale);
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        if (settings->showProgressBar) {
            // ReSharper disable once CppUseStructuredBinding
            const SDL_Color progressBarColor = schedule->CalculateProgressBarColor(timeLeft);
            SDL_SetRenderDrawColor(renderer, progressBarColor.r, progressBarColor.g, progressBarColor.b, 100);
            const auto progressBarBG = SDL_FRect{0, static_cast<float>(windowHeight) - scale * 2, static_cast<float>(windowWidth), scale * 2};
            SDL_RenderFillRect(renderer, &progressBarBG);
            SDL_SetRenderDrawColor(renderer, progressBarColor.r, progressBarColor.g, progressBarColor.b, progressBarColor.a);
            const auto progressBar = SDL_FRect{0, static_cast<float>(windowHeight) - scale * 2,
                static_cast<float>(windowWidth) * (static_cast<float>(totalEventTime - timeLeft) / static_cast<float>(totalEventTime)), scale * 2 + 10};
            SDL_RenderFillRect(renderer, &progressBar);
        }
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

    if (settings != nullptr) {
        if (settings->isSettingsOpen()) {
            settings->SettingsIterate();
            return SDL_APP_CONTINUE;
        }
    }
    SDL_Delay(200);
    return SDL_APP_CONTINUE;
}


void SDL_AppQuit(void *appstate, SDL_AppResult result) { TTF_Quit(); }
