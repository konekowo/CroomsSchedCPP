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

void Settings::drawBooleanSetting(bool settingValue, const std::string& settingName, const std::string& settingID) {
    SDL_FRect settingTitle = textManager->RenderText(currentFont, settingID + ".title",
        settingName + ": ", 10 + currentX, currentY, {255, 255, 255, 255}, 0.5f);
    bool showPercentageHovering = isHovering(settingID + ".value", 20 + currentX, currentY, 380,
                         settingTitle.h);
    textManager->RenderText(currentFont, settingID + ".value",
        settingValue ? "On" : "Off", settingTitle.w + settingTitle.x,
        currentY, settingValue ? selectedColor : showPercentageHovering? hoverColor: unSelectedColor, 0.5f);
    currentY += settingTitle.h + 5;
}

void Settings::drawOptionsSetting(const std::string& settingName, const std::string& settingID, const std::string& currentValue,
                                  std::string* values, int valuesLength) {
    SDL_FRect settingTitle = textManager->RenderText(currentFont, settingID + ".title",
        settingName + ": ", 10 + currentX, currentY, {255, 255, 255, 255}, 0.5f);
    currentY += settingTitle.h + 5;
    for (int i = 0; i < valuesLength; ++i) {
        std::string valueID = settingID;
        valueID += ".value." + values[i];
        SDL_FRect valueOption = textManager->RenderText(currentFont, valueID,
        values[i], 20 + currentX, currentY,
        currentValue == values[i] ? selectedColor:
        isHovering(valueID, 20 + currentX, currentY, 380,
                         settingTitle.h)
                    ? hoverColor
                    : unSelectedColor, 0.5f);
        currentY += valueOption.h;
    }
    currentY += 5;
}

std::string Settings::getTextBoxSetting(const std::string &textBoxID) {
    for (auto &[key, value] : this->periodAliases) {
        if (textBoxID == "settings.periodAliases." + key + ".value") {
            return value;
        }
    }
    return "";
}


void Settings::changeTextBoxSetting(const std::string &textBoxID, const std::string& str) {
    for (auto &[key, value] : this->periodAliases) {
        if (textBoxID == "settings.periodAliases." + key + ".value") {
            this->periodAliases[key] = str;
        }
    }
}


void Settings::drawTextSetting(const std::string& settingValue, const std::string& settingName, const std::string& settingID) {
    SDL_FRect settingTitle = textManager->RenderText(currentFont, settingID + ".title",
        settingName + ": ", 10 + currentX, currentY, {255, 255, 255, 255}, 0.5f);

    SDL_FRect textBoxUnderlineDimensions = {settingTitle.w + settingTitle.x, currentY + settingTitle.h, 390 - settingTitle.x - settingTitle.w, 2};

    if (currentSelectedTextBox == settingID + ".value") {
        SDL_SetRenderDrawColor(renderer, selectedColor.r, selectedColor.g, selectedColor.b, selectedColor.a);
    } else if(isHovering(settingID + ".value", textBoxUnderlineDimensions.x, textBoxUnderlineDimensions.y - settingTitle.h,
        textBoxUnderlineDimensions.w, settingTitle.h)) {
        SDL_SetRenderDrawColor(renderer, hoverColor.r, hoverColor.g, hoverColor.b, hoverColor.a);
    }
    else {
        SDL_SetRenderDrawColor(renderer, unSelectedColor.r, unSelectedColor.g, unSelectedColor.b, unSelectedColor.a);
    }

    SDL_RenderFillRect(renderer, &textBoxUnderlineDimensions);


    SDL_FRect settingValueDimensions = textManager->RenderText(currentFont, settingID + ".value",
        settingValue, settingTitle.w + settingTitle.x, currentY, {255, 255, 255, 255}, 0.5f);

    if (currentSelectedTextBox == settingID + ".value") {
        SDL_FRect textCursorDimensions = {settingValueDimensions.w + settingValueDimensions.x + 1, currentY, 2, settingTitle.h};
        SDL_RenderFillRect(renderer, &textCursorDimensions);
    }

    currentY += settingTitle.h + 5;
}


void Settings::SettingsIterate() {
    currentY = 5;
    this->currentHovered = "";
    SDL_SetRenderDrawColor(renderer, 15, 15, 20, 255);
    SDL_RenderClear(renderer);
    SDL_FRect titleDimensions = textManager->RenderText(currentFont, "settings.title",
        "Settings", 10 + currentX, currentY, {255, 255, 255, 255}, 1);
    currentY += titleDimensions.h + 10;

    drawOptionsSetting("Theme", "settings.theme",
        this->theme == LIGHT ? themeValueStrings[0] : themeValueStrings[1], themeValueStrings, 2);
    drawOptionsSetting("Lunch", "settings.lunch",
        this->defaultLunch == LUNCH_A ? lunchValueStrings[0] : lunchValueStrings[1], lunchValueStrings, 2);

    drawBooleanSetting(this->showProgressBar, "Show Progress Bar", "settings.showProgressBar");
    drawBooleanSetting(this->showPercentage, "Show Percentage", "settings.showPercentage");
    drawBooleanSetting(this->showSeconds, "Show Seconds", "settings.showSeconds");

    SDL_FRect periodAliasDimensions = textManager->RenderText(currentFont, "settings.periodAliases.title",
        "Period Aliases: ", 10 + currentX, currentY, {255, 255, 255, 255}, 0.5f);
    currentY += periodAliasDimensions.h + 5;
    currentX += 10;
    for (const auto& periodAlias : this->periodAliases) {
        drawTextSetting(periodAlias.second, periodAlias.first, "settings.periodAliases." + periodAlias.first);
    }
    currentX -= 10;


    SDL_RenderPresent(renderer);
}

void Settings::OnMouseDown() {
    if (this->currentHovered == "settings.theme.value.Light") {
        this->theme = LIGHT;
    } else if (this->currentHovered == "settings.theme.value.Dark") {
        this->theme = DARK;
    } else if (this->currentHovered == "settings.lunch.value.Lunch A") {
        this->defaultLunch = LUNCH_A;
        this->currentLunch = this->defaultLunch;
    } else if (this->currentHovered == "settings.lunch.value.Lunch B") {
        this->defaultLunch = LUNCH_B;
        this->currentLunch = this->defaultLunch;
    } else if (this->currentHovered == "settings.showPercentage.value") {
        this->showPercentage = !this->showPercentage;
    } else if (this->currentHovered == "settings.showProgressBar.value") {
        this->showProgressBar = !this->showProgressBar;
    }
    else if (this->currentHovered == "settings.showSeconds.value") {
        this->showSeconds = !this->showSeconds;
    }
    bool selectedTextBox = false;
    for (const auto &key: this->periodAliases | std::views::keys) {
        if (this->currentHovered == "settings.periodAliases." + key + ".value") {
            this->currentSelectedTextBox = "settings.periodAliases." + key + ".value";
            SDL_StartTextInput(window);
            selectedTextBox = true;
        }
    }
    if (!selectedTextBox) {
        if (!this->currentSelectedTextBox.empty()) {
            SDL_StopTextInput(window);
        }
        this->currentSelectedTextBox = "";
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
                if (event->button.button == SDL_BUTTON_LEFT) {
                    this->OnMouseDown();
                }
                break;
            case SDL_EVENT_KEY_DOWN:
                if(event->key.key == SDLK_BACKSPACE) {
                    std::string str = getTextBoxSetting(this->currentSelectedTextBox);
                    if (str.length() > 1) {
                        str.pop_back();
                    } else {
                        str = "";
                    }
                    changeTextBoxSetting(this->currentSelectedTextBox, str);
                }
            break;
            case SDL_EVENT_TEXT_INPUT:
                std::string str = getTextBoxSetting(this->currentSelectedTextBox);
                str.append(event->text.text);
                changeTextBoxSetting(this->currentSelectedTextBox, str);
            break;
        }
    }
}

