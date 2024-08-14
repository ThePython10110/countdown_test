/*******************************************************************************
 * countdown.cpp
 * The main code
 *******************************************************************************/

#include "countdown.hpp"
#include <chrono>
#include <ctime>
#include <ctype.h>
#include <fstream>
#include <iostream> // TODO: REMOVE, only for testing
#include <map>
#include <vector>

using json = nlohmann::json;

void to_json(nlohmann ::json &nlohmann_json_j,
             const struct tm &nlohmann_json_t) {
  struct tm nonConstTimeStruct = nlohmann_json_t;
  nlohmann_json_j = Countdown::time2string(nonConstTimeStruct);
}
void from_json(const nlohmann ::json &nlohmann_json_j,
               struct tm &nlohmann_json_t) {
  std::string timeString = nlohmann_json_j.template get<std::string>();
  nlohmann_json_t = Countdown::string2time(timeString);
}

namespace Countdown {

bool hasHappened(struct tm *eventToCheck) {
  return mktime(eventToCheck) < std::time(0);
}

bool hasHappened(event *eventToCheck) {
  return hasHappened(&(eventToCheck->time));
}

// Will crash if given an event without a repeat
struct tm getRepeat(const event &eventToRepeat) {
  char unit1;
  int repeatNum = 1;

  if (eventToRepeat.repeat[0] == 'W') {
    unit1 = eventToRepeat.repeat[1];
    char unit2 = 'Y';
    int i = 1;

    assert(unit1 == 'M' || unit1 == 'Y');
    // If there's a number, get it
    if (std::isdigit(eventToRepeat.repeat[i])) {
      std::string repeatNumStr = "";
      while (std::isdigit(eventToRepeat.repeat[i]) &&
             i < eventToRepeat.repeat.length()) {
        repeatNumStr += eventToRepeat.repeat[i];
        ++i;
      }
      repeatNum = stoi(repeatNumStr);
    }
    if (i < eventToRepeat.repeat.length()) {
      // Wu[#]u
      assert(i == eventToRepeat.repeat.length() - 1);
      unit2 = eventToRepeat.repeat[i];
      assert(unit2 == 'M' || unit2 == 'Y');
    }
    // same day of week every <repeatNum> months
    int currentWeekdayNum = getWeekdayNum(eventToRepeat.time, unit1 == 'M');
    int month, year;
    if (unit1 == 'M') {
      month = (eventToRepeat.time.tm_mon + repeatNum) % 12;
      year = eventToRepeat.time.tm_year + (repeatNum / 12);
    } else {
      month = eventToRepeat.time.tm_mon;
      year = eventToRepeat.time.tm_year + repeatNum;
    }
    struct tm newTime = {eventToRepeat.time.tm_sec,
                         eventToRepeat.time.tm_min,
                         eventToRepeat.time.tm_hour,
                         1, // tm_mday
                         month,
                         year};
    mktime(&newTime);
    return getNthWeekdayFromDate(newTime, eventToRepeat.time.tm_wday,
                                 currentWeekdayNum);
  } else {
    struct tm newTime = eventToRepeat.time;
    int i = 0;
    if (std::isdigit(eventToRepeat.repeat[0])) {
      std::string repeatNumStr = "";
      while (std::isdigit(eventToRepeat.repeat[i]) &&
             i < eventToRepeat.repeat.length()) {
        repeatNumStr += eventToRepeat.repeat[i];
        ++i;
      }
      repeatNum = stoi(repeatNumStr);
    }
    assert(i < eventToRepeat.repeat.length());
    switch (eventToRepeat.repeat[i]) {
    case 's':
      newTime.tm_sec += repeatNum;
      mktime(&newTime);
      return newTime;
    case 'm':
      newTime.tm_min += repeatNum;
      mktime(&newTime);
      return newTime;
    case 'h':
      newTime.tm_hour += repeatNum;
      mktime(&newTime);
      return newTime;
    case 'D':
      newTime.tm_yday += repeatNum;
      mktime(&newTime);
      return newTime;
    case 'W':
      newTime.tm_yday += 7 * repeatNum;
      mktime(&newTime);
      return newTime;
    case 'M':
      newTime.tm_mon += repeatNum;
      mktime(&newTime);
      return newTime;
    case 'Y':
      newTime.tm_year += repeatNum;
      mktime(&newTime);
      return newTime;
    default:
      throw std::invalid_argument(&"Invalid unit"[eventToRepeat.repeat[i]]);
    };
  }
}

std::string time2string(struct tm &timeStruct) {
  char buffer[20]; // in theory exactly enough for a null-terminated string of
                   // yyyy-mm-ddThh:mm:ss
  strftime(buffer, 100, "%FT%T", &timeStruct);
  return std::string(buffer);
}

struct tm string2time(const std::string &timeString) {
  char timeCharArr[20];
  strcpy(timeCharArr, timeString.c_str());

  struct tm timeStruct;
  strptime(timeCharArr, "%FT%T", &timeStruct);
  time_t timeT = mktime(&timeStruct);
  return *gmtime(&timeT);
}

struct tm getNthWeekdayFromDate(const struct tm &startDate,
                                const int &weekdayNum, const int &n) {
  int startWeekday = startDate.tm_wday;
  int daysUntilFirstMatch = (weekdayNum - startWeekday + 7) % 7;

  struct tm newTime = startDate;
  newTime.tm_yday += daysUntilFirstMatch + ((n - 1) * 7);
  mktime(&newTime);
  return newTime;
}

int getWeekdayNum(const struct tm &dayToCheck, const bool &ofYear) {
  // This took me forever to figure out.
  if (ofYear)
    return (dayToCheck.tm_yday + (7 - dayToCheck.tm_wday)) / 7;
  return (dayToCheck.tm_mday + (7 - dayToCheck.tm_wday)) / 7;
}

event CountdownData::getEvent(int index) const {
  return data["events"].at(index);
}
void CountdownData::addEvent(event eventToAdd) {
  data["events"].push_back(eventToAdd);
}
void CountdownData::setEvent(int index, event eventToSet) {
  data["events"][index] = eventToSet;
}
bool CountdownData::loadData() {
  std::ifstream jsonFile(filename);
  data = json::parse(jsonFile);
  return true;
}
void CountdownData::saveData() const {
  std::ofstream jsonFile(filename);
  jsonFile << std::setw(4) << data << std::endl;
}

} // namespace Countdown

int main() {
  // Setup ncurses
  /*initscr();*/
  /*cbreak();*/
  /*noecho();*/
  /*keypad(stdscr, TRUE);*/

  Countdown::CountdownData countdown("data.json");
  countdown.loadData();
  struct tm repeat = Countdown::getRepeat(countdown.getEvent(0));
  time_t now = time(0);
  struct tm localTime = *localtime(&now);
  struct tm gmTime = *gmtime(&now);
  std::cout << asctime(&repeat);
  std::cout << "Local Time: " << asctime(&localTime);
  std::cout << "GMT Time: " << asctime(&gmTime);
  std::cout << "Time Zone: " << localTime.tm_zone << "\n";
  std::cout << "DST: " << (localTime.tm_isdst > 0 ? "Yes" : "No") << "\n";

  return 0;
}
