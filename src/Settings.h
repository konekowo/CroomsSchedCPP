#pragma once
#include "TextManager.h"


#include <SDL3/SDL_render.h>
#include <string>
#include <unordered_map>

enum Theme {
    DARK = 0,
    LIGHT = 1
};
enum Lunch {
    LUNCH_A = 0,
    LUNCH_B = 1
};

class Settings {
    std::string saveFilePath;
    void Load();
    bool isOpen = false;
    bool hasFocus = false;
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    TextManager *textManager = nullptr;
    TTF_Font *currentFont = nullptr;
    float mouseX = 0;
    float mouseY = 0;
    std::string currentHovered;
public:
    Theme theme = DARK;
    bool showProgressBar = true;
    bool showSeconds = true;
    std::string fontLocation = "./assets/fonts/SegoeUI.ttf";
    Lunch defaultLunch = LUNCH_A;
    Lunch currentLunch = LUNCH_A;
    std::pmr::unordered_map<std::string, std::string> periodAliases = {
        {"Nothing", "Nothing"},
        {"Period 1", "Period 1"},
        {"Period 2", "Period 2"},
        {"Period 3", "Period 3"},
        {"Period 4", "Period 4"},
        {"Period 5", "Period 5"},
        {"Period 6", "Period 6"},
        {"Period 7", "Period 7"},
        {"Morning", "Morning"},
        {"Welcome", "Welcome"},
        {"Lunch", "Lunch"},
        {"Homeroom", "Homeroom"},
        {"Dismissal", "Dismissal"},
        {"After School", "After School"},
        {"End", "End"},
        {"Break", "Break"},
        {"PSAT/SAT", "PSAT/SAT"}
    };
    explicit Settings(const std::string &saveFilePath);
    void Save();
    bool isSettingsOpen() const {
        return this->isOpen;
    }
    bool SettingsWindowHasFocus() const {
        return this->hasFocus;
    }
    void OpenSettings();
    void CloseSettings();
    void PollEvent(SDL_Event* event);
    void SettingsIterate();
    void OnMouseDown();
    bool isHovering(std::string settingsValue, float x, float y, float width, float height);
};

