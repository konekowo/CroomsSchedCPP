#pragma once
#include <chrono>
#include <nlohmann/json_fwd.hpp>

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

enum Lunch { LUNCH_A = 0, LUNCH_B = 1 };

class Schedule {
    std::string status;
    std::chrono::duration<long long, std::ratio<1, 1000000000>> responseTime{};
    Schedule_Data data;
    Lunch lunch;

public:
    explicit Schedule(nlohmann::json json);
    std::string GetCurrentEvent();
    std::string GetStatus() { return this->status; }
    [[nodiscard]] std::chrono::duration<long long, std::ratio<1, 1000000000>> GetResponseTime() const {
        return this->responseTime;
    }
    Schedule_Data GetData() { return this->data; }
    int GetSecondsLeft();
    void SetLunch(const Lunch lunch) { this->lunch = lunch; }
    [[nodiscard]] Lunch GetLunch() const { return this->lunch; }
};

std::string Sched_PadTime(int time, int padLength);
