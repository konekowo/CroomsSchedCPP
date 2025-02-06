#pragma once
#include <string>
#include <unordered_map>

enum Theme {
    DARK = 0,
};
enum Lunch {
    LUNCH_A = 0,
    LUNCH_B = 1
};

class Settings {
    std::string saveFilePath;
    void Load();
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
};

