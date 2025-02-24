#pragma once
#include <map>


#include "TextManager.h"


#include <SDL3/SDL_render.h>
#include <string>
#include <vector>

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

    float currentX = 0;
    float currentY = 0;
    std::string currentHovered;
    std::string currentSelectedTextBox;
    bool isHovering(std::string settingsValue, float x, float y, float width, float height);

    void drawBooleanSetting(bool settingValue, const std::string& settingName, const std::string& settingID);
    void drawOptionsSetting(const std::string &settingName, const std::string &settingID,
                            const std::string &currentValue, std::string *values, int valuesLength);
    void drawTextSetting(const std::string& settingValue, const std::string& settingName, const std::string& settingID);
    std::string getTextBoxSetting(const std::string &textBoxID);
    void changeTextBoxSetting(const std::string &textBoxID, std::string str);
    std::string *themeValueStrings = new std::string[]{"Light", "Dark"};
    std::string *lunchValueStrings = new std::string[]{"Lunch A", "Lunch B"};
public:
    Theme theme = DARK;
    bool showProgressBar = true;
    bool showSeconds = true;
    bool showPercentage = false;
    std::string fontLocation = "./assets/fonts/SegoeUI.ttf";
    Lunch defaultLunch = LUNCH_A;
    Lunch currentLunch = LUNCH_A;
    std::pmr::map<std::string, std::string> periodAliases = {
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
    [[nodiscard]] bool isSettingsOpen() const {
        return this->isOpen;
    }
    [[nodiscard]] bool SettingsWindowHasFocus() const {
        return this->hasFocus;
    }
    void OpenSettings();
    void CloseSettings();
    void RaiseWindow() const;
    void PollEvent(SDL_Event* event);
    void SettingsIterate();
    void OnMouseDown();
};

