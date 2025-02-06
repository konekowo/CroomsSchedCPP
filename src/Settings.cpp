#include "Settings.h"

#include <nlohmann/json.hpp>
#include <fstream>

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
    settingsJson["fontLocation"] = this->fontLocation;
    settingsJson["defaultLunch"] = this->defaultLunch;
    const nlohmann::json periodAliases(this->periodAliases);
    settingsJson["periodAliases"] = periodAliases;

    std::ofstream jsonFile(saveFilePath);
    jsonFile << settingsJson;
}

void Settings::Load() {
    std::ifstream jsonFile(saveFilePath);
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
    if (settingsJson["fontLocation"].is_string()) {
        this->fontLocation = settingsJson["fontLocation"];
    }
    if (settingsJson["defaultLunch"].is_number_integer()) {
        this->defaultLunch = settingsJson["defaultLunch"];
    }
    if (settingsJson["periodAliases"].is_object()) {
        for (const auto &key: this->periodAliases | std::views::keys) {
            if (settingsJson["periodAliases"][key].is_string()) {
                this->periodAliases[key] = settingsJson["periodAliases"][key];
            }
        }
    }
}