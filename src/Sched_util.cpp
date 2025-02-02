//
// Created by kone on 2/1/2025.
//

#include "Sched_util.h"

#include <chrono>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
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

struct Sched_Time {
    int hours;
    int minutes;
    int seconds;
};

struct Sched_Event {
    int event;
    int startH;
    int startM;
    int endH;
    int endM;
};

Sched_Time Sched_GetCurrentTime() {
    const auto currentTime = std::chrono::system_clock::now();
    auto tp = currentTime.time_since_epoch() -
              std::chrono::duration_cast<std::chrono::days>(currentTime.time_since_epoch());
    auto h = std::chrono::duration_cast<std::chrono::hours>(tp);
    tp -= h;
    h += std::chrono::duration_cast<std::chrono::hours>(GMT_OFFSET);
    const auto m = std::chrono::duration_cast<std::chrono::minutes>(tp);
    tp -= m;
    const auto s = std::chrono::duration_cast<std::chrono::seconds>(tp);

    return Sched_Time{static_cast<int>(h.count()), static_cast<int>(m.count()), static_cast<int>(s.count())};
}

Sched_Event Sched_ConvertEvent(json eventJson) {
    return Sched_Event{eventJson[0], eventJson[1], eventJson[2], eventJson[3], eventJson[4]};
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

const char* Sched_GetCurrentEvent(json schedJson, const bool bLunch) {
    auto [hours, minutes, seconds] = Sched_GetCurrentTime();
    const char *currentEventName = nullptr;
    bool currentEventFound = false;
    for (json i: schedJson["data"]["schedule"][static_cast<int>(bLunch)]) {
        auto [event, startH, startM, endH, endM] = Sched_ConvertEvent(i);
        if (hours > startH && hours < endH && minutes > startM && minutes < endM) {
            currentEventName = Sched_GetEventName(event);
            currentEventFound = true;
            continue;
        }
        if (!currentEventFound) {
            currentEventName = ("Go to " + std::string(Sched_GetEventName(event))).c_str();
            break;
        }
    }

    return currentEventName;
}

int Sched_GetHrsLeft(json schedJson, const bool bLunch) {
    auto [hours, minutes, seconds] = Sched_GetCurrentTime();
    int hoursLeft = 0;
    bool currentEventFound = false;
    for (json i: schedJson["data"]["schedule"][static_cast<int>(bLunch)]) {
        auto [event, startH, startM, endH, endM] = Sched_ConvertEvent(i);
        if (hours > startH && hours < endH && minutes > startM && minutes < endM) {
            hoursLeft = endH - hours;
            currentEventFound = true;
            continue;
        }
        if (!currentEventFound) {
            hoursLeft = startH - hours;
            break;
        }
    }
    return hoursLeft;
}

int Sched_GetMinsLeft(json schedJson, const bool bLunch) {
    auto [hours, minutes, seconds] = Sched_GetCurrentTime();
    int hoursLeft = 0;
    bool currentEventFound = false;
    for (json i: schedJson["data"]["schedule"][static_cast<int>(bLunch)]) {
        auto [event, startH, startM, endH, endM] = Sched_ConvertEvent(i);
        if (hours > startH && hours < endH && minutes > startM && minutes < endM) {
            hoursLeft = endM - minutes;
            currentEventFound = true;
            continue;
        }
        if (!currentEventFound) {
            hoursLeft = startM - minutes;
            break;
        }
    }
    return hoursLeft;
}

int Sched_GetSecsLeft() {
    auto [hours, minutes, seconds] = Sched_GetCurrentTime();
    return 59 - seconds;
}

std::string Sched_PadTime(const int time, const int padLength) {
    if (std::string str = std::to_string(time); str.length() < padLength) {
        str.insert(0, padLength - str.length(), '0');
        return str;
    }
    else {
        return str;
    }
}