//
// Created by kone on 2/1/2025.
//

#include "Sched_util.h"

#include <chrono>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

static const auto GMT_OFFSET = std::chrono::hours(-5);

static const auto EVENT_STRING_NOTHING = "Nothing";
static const auto EVENT_STRING_PERIOD1 = "Period 1";
static const auto EVENT_STRING_PERIOD2 = "Period 2";
static const auto EVENT_STRING_PERIOD3 = "Period 3";
static const auto EVENT_STRING_PERIOD4 = "Period 4";
static const auto EVENT_STRING_PERIOD5 = "Period 5";
static const auto EVENT_STRING_PERIOD6 = "Period 6";
static const auto EVENT_STRING_PERIOD7 = "Period 7";
static const auto EVENT_STRING_MORNING = "Morning";
static const auto EVENT_STRING_WELCOME = "Welcome";
static const auto EVENT_STRING_LUNCH = "Lunch";
static const auto EVENT_STRING_HOMEROOM = "Homeroom";
static const auto EVENT_STRING_DISMISSAL = "Dismissal";
static const auto EVENT_STRING_AFTER_SCHOOL = "After School";
static const auto EVENT_STRING_END = "End";
static const auto EVENT_STRING_BREAK = "Break";
static const auto EVENT_STRING_PSAT_SAT = "PSAT/SAT";

static constexpr auto EVENT_NOTHING = 0;
static constexpr auto EVENT_PERIOD1 = 1;
static constexpr auto EVENT_PERIOD2 = 2;
static constexpr auto EVENT_PERIOD3 = 3;
static constexpr auto EVENT_PERIOD4 = 4;
static constexpr auto EVENT_PERIOD5 = 5;
static constexpr auto EVENT_PERIOD6 = 6;
static constexpr auto EVENT_PERIOD7 = 7;
static constexpr auto EVENT_MORNING = 100;
static constexpr auto EVENT_WELCOME = 101;
static constexpr auto EVENT_LUNCH = 102;
static constexpr auto EVENT_HOMEROOM = 103;
static constexpr auto EVENT_DISMISSAL = 104;
static constexpr auto EVENT_AFTER_SCHOOL = 105;
static constexpr auto EVENT_END = 106;
static constexpr auto EVENT_BREAK = 107;
static constexpr auto EVENT_PSAT_SAT = 110;
static const SDL_Color NormalColor = {255, 255, 255};
static const SDL_Color RedColor = {255, 0, 0};
static const SDL_Color OrangeColor = {255, 127, 39};

int Sched_GetCurrentTimeSeconds() {
    const auto currentTime = std::chrono::system_clock::now();
    auto tp = currentTime.time_since_epoch();
    tp += std::chrono::duration_cast<std::chrono::hours>(GMT_OFFSET);
    tp -= std::chrono::duration_cast<std::chrono::days>(tp);
    const auto s = std::chrono::duration_cast<std::chrono::seconds>(tp);

    return static_cast<int>(s.count());
}

const char *Sched_GetEventName(const int event) {
    switch (event) {
        case EVENT_NOTHING:
            return EVENT_STRING_NOTHING;
        case EVENT_PERIOD1:
            return EVENT_STRING_PERIOD1;
        case EVENT_PERIOD2:
            return EVENT_STRING_PERIOD2;
        case EVENT_PERIOD3:
            return EVENT_STRING_PERIOD3;
        case EVENT_PERIOD4:
            return EVENT_STRING_PERIOD4;
        case EVENT_PERIOD5:
            return EVENT_STRING_PERIOD5;
        case EVENT_PERIOD6:
            return EVENT_STRING_PERIOD6;
        case EVENT_PERIOD7:
            return EVENT_STRING_PERIOD7;
        case EVENT_MORNING:
            return EVENT_STRING_MORNING;
        case EVENT_WELCOME:
            return EVENT_STRING_WELCOME;
        case EVENT_LUNCH:
            return EVENT_STRING_LUNCH;
        case EVENT_HOMEROOM:
            return EVENT_STRING_HOMEROOM;
        case EVENT_DISMISSAL:
            return EVENT_STRING_DISMISSAL;
        case EVENT_AFTER_SCHOOL:
            return EVENT_STRING_AFTER_SCHOOL;
        case EVENT_END:
            return EVENT_STRING_END;
        case EVENT_BREAK:
            return EVENT_STRING_BREAK;
        case EVENT_PSAT_SAT:
            return EVENT_STRING_PSAT_SAT;
        default:
            return EVENT_STRING_NOTHING;
    }
}

Sched_Event Sched_ConvertEvent(json eventJson) {
    const int startS = static_cast<int>(eventJson[0]) * 60 * 60 + static_cast<int>(eventJson[1]) * 60;
    const int endS = static_cast<int>(eventJson[3]) * 60 * 60 + static_cast<int>(eventJson[4]) * 60;
    return Sched_Event{eventJson[2], startS, endS};
}

Schedule::Schedule(nlohmann::json json) {
    this->status = json["status"];
    this->responseTime = std::chrono::system_clock::now().time_since_epoch();
    std::vector<std::vector<Sched_Event>> schedule;
    int iteration = 0;
    for (auto i: json["data"]["schedule"]) {
        std::vector<Sched_Event> events;
        for (auto j: json["data"]["schedule"][iteration]) { // NOLINT(*-for-range-copy)
            events.push_back(Sched_ConvertEvent(j));
        }
        schedule.push_back(events);
        iteration++;
    }
    this->data = {.id = json["data"]["id"], .msg = json["data"]["msg"], .schedule = schedule};
    this->lunch = LUNCH_A;
}

int Schedule::GetSecondsLeft() {
    const int seconds = Sched_GetCurrentTimeSeconds();
    int secondsLeft = 0;
    for (auto [event, startS, endS]: this->data.schedule.at(this->lunch)) {
        if (seconds > startS && seconds < endS) {
            secondsLeft = endS - seconds;
            break;
        }
        if (seconds < endS && seconds < startS) {
            secondsLeft = startS - seconds;
            break;
        }
    }
    return secondsLeft;
}

int Schedule::GetEventSeconds() {
    const int seconds = Sched_GetCurrentTimeSeconds();
    int eventSeconds = 0;
    int lastEndS = 0;
    for (auto [event, startS, endS]: this->data.schedule.at(this->lunch)) {
        if (seconds > startS && seconds < endS) {
            eventSeconds = endS - startS;
            break;
        }
        if (seconds < endS && seconds < startS) {
            eventSeconds = startS - lastEndS;
            break;
        }
        lastEndS = endS;
    }
    return eventSeconds;
}

std::string Schedule::GetCurrentEvent() {
    const int seconds = Sched_GetCurrentTimeSeconds();
    std::string eventName;
    for (auto [event, startS, endS]: this->data.schedule.at(this->lunch)) {
        if (seconds > startS && seconds < endS) {
            eventName = Sched_GetEventName(event);
            break;
        }
        if (seconds < endS && seconds < startS) {
            eventName = "Go to " + std::string(Sched_GetEventName(event));
            break;
        }
    }
    return eventName;
}

std::string Schedule::PadTime(const int time, const int padLength) {
    if (std::string str = std::to_string(time); str.length() < padLength) {
        str.insert(0, padLength - str.length(), '0');
        return str;
    } else {
        return str;
    }
}

SDL_Color Schedule::CalculateProgressBarColor(const int secondsRemaining)
{
    if (secondsRemaining <= 60) {
        return RedColor;
    }
    else if (secondsRemaining <= 60 * 5) { // 10 Minutes
        return OrangeColor;
    }
    else {
        return NormalColor;
    }
}