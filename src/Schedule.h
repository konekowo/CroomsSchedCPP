#pragma once
#include <SDL3/SDL_pixels.h>
#include <chrono>
#include <nlohmann/json_fwd.hpp>

#include "Settings.h"

struct Sched_Event {
    int event;
    int startS;
    int endS;
};

struct Schedule_Data {
    std::string id;
    std::string msg;
    std::vector<std::vector<Sched_Event>> schedule;
};

class Schedule {
    std::string status;
    std::chrono::duration<long long, std::ratio<1, 1000000000>> responseTime{};
    Schedule_Data data;
    Settings* settings;
    [[nodiscard]] const char* GetEventName(int event) const;
    const char* GetEventAliasName(const char* eventName) const;
public:
    explicit Schedule(nlohmann::json json, Settings* settings);
    std::string GetCurrentEvent();
    std::string GetStatus() { return this->status; }
    [[nodiscard]] std::chrono::duration<long long, std::ratio<1, 1000000000>> GetResponseTime() const {
        return this->responseTime;
    }
    Schedule_Data GetData() { return this->data; }
    int GetSecondsLeft();
    int GetEventSeconds();
    static std::string PadTime(int time, int padLength);
    [[nodiscard]] SDL_Color CalculateTextColor(int secondsRemaining) const;
    [[nodiscard]] SDL_Color CalculateProgressBarColor(int secondsRemaining) const;
};