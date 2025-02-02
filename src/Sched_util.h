#ifndef SCHED_UTIL_H
#define SCHED_UTIL_H
#include <nlohmann/json_fwd.hpp>

const char* Sched_GetCurrentEvent(nlohmann::json schedJson, bool bLunch);
int Sched_GetHrsLeft(nlohmann::json schedJson, bool bLunch);
int Sched_GetMinsLeft(nlohmann::json schedJson, bool bLunch);
int Sched_GetSecsLeft();
std::string Sched_PadTime(int time, int padLength);

#endif //SCHED_UTIL_H
