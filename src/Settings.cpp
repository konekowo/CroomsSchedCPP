#include "Settings.h"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <utility>

static const SDL_Color selectedColor = {255, 255, 255, 255};
static const SDL_Color unSelectedColor = {150, 150, 150, 255};
static const SDL_Color hoverColor = {190, 190, 190, 255};

Settings::Settings(const std::string &saveFilePath) {
    this->saveFilePath = saveFilePath;
    Load();
    Save();
}

void Settings::Save() {
    auto settingsJson = nlohmann::json();

    settingsJson["theme"] = this->theme;
    settingsJson["showProgressBar"] = this->showProgressBar;
    settingsJson["showSeconds"] = this->showSeconds;
    settingsJson["showPercentage"] = this->showPercentage;
    settingsJson["fontLocation"] = this->fontLocation;
    settingsJson["defaultLunch"] = this->defaultLunch;
    const nlohmann::json periodAliases(this->periodAliases);
    settingsJson["periodAliases"] = periodAliases;

    std::ofstream jsonFile(saveFilePath);
    jsonFile << std::setw(4) << settingsJson;
}

void Settings::Load() {
    std::ifstream jsonFile(saveFilePath);
    if (!std::filesystem::exists(saveFilePath)) return;

    auto settingsJson = nlohmann::json::parse(jsonFile);

    if (settingsJson["theme"].is_number_integer()) {
        this->theme = settingsJson["theme"];
    }
    if (settingsJson["showProgressBar"].is_boolean()) {
        this->showProgressBar = settingsJson["showProgressBar"];
    }
    if (settingsJson["showSeconds"].is_boolean()) {
        this->showSeconds = settingsJson["showSeconds"];
    }
    if (settingsJson["showPercentage"].is_boolean()) {
        this->showPercentage = settingsJson["showPercentage"];
    }
    if (settingsJson["fontLocation"].is_string()) {
        this->fontLocation = settingsJson["fontLocation"];
    }
    if (settingsJson["defaultLunch"].is_number_integer()) {
        this->defaultLunch = settingsJson["defaultLunch"];
    }
    this->currentLunch = this->defaultLunch;
    if (settingsJson["periodAliases"].is_object()) {
        for (const auto &key: this->periodAliases | std::views::keys) {
            if (settingsJson["periodAliases"][key].is_string()) {
                this->periodAliases[key] = settingsJson["periodAliases"][key];
            }
        }
    }
}

void Settings::OpenSettings() {
    if (!this->isOpen) {
        isOpen = true;
        if (!SDL_CreateWindowAndRenderer("Crooms Bell Schedule Settings", 400, 800, SDL_WINDOW_HIGH_PIXEL_DENSITY,
                                         &window, &renderer)) {
            SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
                                         }
        SDL_RaiseWindow(window);
        textManager = new TextManager(renderer);
        currentFont = TTF_OpenFont(this->fontLocation.c_str(), 32);
        if (currentFont == nullptr) {
            SDL_Log("TTF_OpenFont() Error: %s", SDL_GetError());
            exit(1);
        }
    }
}

void Settings::CloseSettings() {
    if (this->isOpen) {
        isOpen = false;
        hasFocus = false;

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_CloseFont(currentFont);
        delete textManager;

        renderer = nullptr;
        window = nullptr;
        currentFont = nullptr;
        textManager = nullptr;
        currentHovered = "";
        mouseX = 0;
        mouseY = 0;

        Save();
        Load();
    }
}

void Settings::RaiseWindow() const {
    if (window != nullptr) {
        SDL_RaiseWindow(window);
    }
}

bool Settings::isHovering(std::string settingsValue, float x, float y, float width, float height) {
    bool result = mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height;
    if (result) {
        this->currentHovered = std::move(settingsValue);
    }
    return result;
}

void Settings::SettingsIterate() {
    this->currentHovered = "";
    SDL_SetRenderDrawColor(renderer, 15, 15, 20, 255);
    SDL_RenderClear(renderer);
    SDL_FRect titleDimensions = textManager->RenderText(currentFont, "settings.title",
        "Settings", 10, 5, {255, 255, 255, 255}, 1);

    /** ---- THEME ---- **/
    SDL_FRect themeTitleDimensions = textManager->RenderText(currentFont, "settings.theme.title",
        "Theme:", 10, titleDimensions.y + titleDimensions.h + 10, {255, 255, 255, 255}, 0.5f);
    SDL_FRect themeLightDimensions = textManager->RenderText(currentFont, "settings.theme.value.light",
        "Light", 20, themeTitleDimensions.y + themeTitleDimensions.h + 5,
        this->theme == LIGHT ? selectedColor :
        isHovering("theme.value.light", 20, themeTitleDimensions.h + themeTitleDimensions.y + 5, 380,
                         themeTitleDimensions.h)
                    ? hoverColor
                    : unSelectedColor, 0.5f);
    SDL_FRect themeDarkDimensions = textManager->RenderText(currentFont, "settings.theme.value.dark",
    "Dark", 20, themeLightDimensions.y + themeLightDimensions.h, this->theme == DARK ? selectedColor :
    isHovering("theme.value.dark", 20, themeLightDimensions.h + themeLightDimensions.y, 380,
                     themeLightDimensions.h)
                ? hoverColor
                : unSelectedColor, 0.5f);

    /** ---- LUNCH ---- **/

    SDL_FRect lunchTitleDimensions = textManager->RenderText(currentFont, "settings.lunch.title",
        "Default Lunch:", 10, themeDarkDimensions.y + themeDarkDimensions.h + 5, {255, 255, 255, 255}, 0.5f);
    SDL_FRect lunchADimensions = textManager->RenderText(currentFont, "settings.lunch.value.a",
        "Lunch A", 20, lunchTitleDimensions.y + lunchTitleDimensions.h + 5,
        this->defaultLunch == LUNCH_A ? selectedColor :
        isHovering("lunch.value.a", 20, lunchTitleDimensions.h + lunchTitleDimensions.y + 5, 380,
                         lunchTitleDimensions.h)
                    ? hoverColor
                    : unSelectedColor, 0.5f);
    SDL_FRect lunchBDimensions = textManager->RenderText(currentFont, "settings.lunch.value.b",
    "Lunch B", 20, lunchADimensions.y + lunchADimensions.h, this->defaultLunch == LUNCH_B ? selectedColor :
    isHovering("lunch.value.b", 10, lunchADimensions.h + lunchADimensions.y, 380,
                     lunchADimensions.h)
                ? hoverColor
                : unSelectedColor, 0.5f);

    /** ---- SHOW PERCENTAGE ---- **/

    SDL_FRect showPercentageTitleDimensions = textManager->RenderText(currentFont, "settings.showPercentage.title",
        "Show Percentage: ", 10, lunchBDimensions.y + lunchBDimensions.h + 5, {255, 255, 255, 255}, 0.5f);
    bool showPercentageHovering = isHovering("showPercentage.value", 20, lunchBDimensions.h + lunchBDimensions.y + 5, 380,
                         lunchBDimensions.h);
    SDL_FRect showPercentageDimensions = textManager->RenderText(currentFont, "settings.showPercentage.value",
        this->showPercentage ? "On" : "Off", showPercentageTitleDimensions.w + showPercentageTitleDimensions.x,
        lunchBDimensions.y + lunchBDimensions.h + 5,this->showPercentage ? selectedColor : showPercentageHovering? hoverColor: unSelectedColor, 0.5f);

    /** ---- SHOW PROGRESS BAR ---- **/

    SDL_FRect showProgressBarTitleDimensions = textManager->RenderText(currentFont, "settings.showProgressBar.title",
        "Show Progress Bar: ", 10, showPercentageDimensions.y + showPercentageDimensions.h + 5, {255, 255, 255, 255}, 0.5f);
    bool showProgressBarHovering = isHovering("showProgressBar.value", 20, showPercentageDimensions.h + showPercentageDimensions.y + 5, 380,
                         showPercentageDimensions.h);
    SDL_FRect showProgressBarDimensions = textManager->RenderText(currentFont, "settings.showProgressBar.value",
        this->showProgressBar ? "On" : "Off", showProgressBarTitleDimensions.w + showProgressBarTitleDimensions.x,
        showPercentageDimensions.y + showPercentageDimensions.h + 5,
        this->showProgressBar ? selectedColor : showProgressBarHovering? hoverColor: unSelectedColor, 0.5f);

    /** ---- SHOW SECONDS ---- **/

    SDL_FRect showSecondsTitleDimensions = textManager->RenderText(currentFont, "settings.showSeconds.title",
        "Show Seconds: ", 10, showProgressBarDimensions.y + showProgressBarDimensions.h + 5, {255, 255, 255, 255}, 0.5f);
    bool showSecondsHovering = isHovering("showSeconds.value", 20, showProgressBarDimensions.h + showProgressBarDimensions.y + 5, 380,
                         showProgressBarDimensions.h);
    SDL_FRect showSecondsDimensions = textManager->RenderText(currentFont, "settings.showSeconds.value",
        this->showSeconds ? "On" : "Off", showSecondsTitleDimensions.w + showSecondsTitleDimensions.x,
        showProgressBarDimensions.y + showProgressBarDimensions.h + 5,
        this->showSeconds ? selectedColor : showSecondsHovering? hoverColor: unSelectedColor, 0.5f);

    /** ---- PERIOD ALIASES ---- **/
    SDL_FRect periodAliasesTitleDimensions = textManager->RenderText(currentFont, "settings.periodAliases.title",
        "Period Aliases: ", 10, showSecondsDimensions.y + showSecondsDimensions.h + 5, {255, 255, 255, 255}, 0.5f);
    float currentY = periodAliasesTitleDimensions.h + periodAliasesTitleDimensions.y + 5;
    for (const auto& periodAlias : periodAliases) {
        SDL_FRect periodAliasDimensions = textManager->RenderText(currentFont, "settings.periodAliases.values."
                                                                               ""
                                                                               "" + periodAlias.first,
            periodAlias.first, 20, currentY, {255, 255, 255, 255}, 0.5f);
        currentY += periodAliasDimensions.h + 5;

    }

    SDL_RenderPresent(renderer);
}

void Settings::OnMouseDown() {
    if (this->currentHovered == "theme.value.light") {
        this->theme = LIGHT;
    } else if (this->currentHovered == "theme.value.dark") {
        this->theme = DARK;
    } else if (this->currentHovered == "lunch.value.a") {
        this->defaultLunch = LUNCH_A;
        this->currentLunch = this->defaultLunch;
    } else if (this->currentHovered == "lunch.value.b") {
        this->defaultLunch = LUNCH_B;
        this->currentLunch = this->defaultLunch;
    } else if (this->currentHovered == "showPercentage.value") {
        this->showPercentage = !this->showPercentage;
    }
    else if (this->currentHovered == "showProgressBar.value") {
        this->showProgressBar = !this->showProgressBar;
    }
    else if (this->currentHovered == "showSeconds.value") {
        this->showSeconds = !this->showSeconds;
    }
}


void Settings::PollEvent(SDL_Event* event) {
    SDL_WindowID windowID = SDL_GetWindowID(window);
    if (event->window.windowID == windowID) {
        switch (event->type) {
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                this->CloseSettings();
                break;
            case SDL_EVENT_WINDOW_FOCUS_GAINED:
                hasFocus = true;
                break;
            case SDL_EVENT_WINDOW_FOCUS_LOST:
                hasFocus = false;
                break;
            case SDL_EVENT_MOUSE_MOTION:
                this->mouseX = event->motion.x;
                this->mouseY = event->motion.y;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                this->OnMouseDown();
                break;
            default:;
        }
    }
}

