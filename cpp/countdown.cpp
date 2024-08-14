#include "countdown.hpp"
#include <chrono>
#include <ctime>
#include <ctype.h>
#include <iostream> // TODO: REMOVE, only for testing
#include <ncurses.h>

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
    return getNthWeekdayAfterDate(newTime, eventToRepeat.time.tm_wday,
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
      newTime.tm_yday += 7*repeatNum;
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
      throw std::invalid_argument(&"Invalid unit" [ eventToRepeat.repeat[i]]);
    };
  }
}

std::string time2string(const struct tm &timeStruct);
struct tm string2time(const std::string &timeString);
struct tm getNthWeekdayAfterDate(const struct tm &startDate,
                                 const int &weekdayNum, const int &n);
int getWeekdayNum(const struct tm &dayToCheck, const bool &ofYear);

} // namespace Countdown

int main() {
  // Setup ncurses
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  return 0;
}
