/******************************************************************************
 * countdown.cpp
 * The main code
 ******************************************************************************/

#include "countdown.hpp"
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream> // TODO: REMOVE, only for testing
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;
namespace fs = std::filesystem;

void to_json(json &j, const struct tm &t) {
  struct tm nonConstTimeStruct = t;
  j = Countdown::time2string(nonConstTimeStruct);
}
void from_json(const json &j, struct tm &t) {
  std::string timeString = j.template get<std::string>();
  t = Countdown::string2time(timeString);
}

bool operator<(const struct tm &lhs, const struct tm &rhs) {
  struct tm nonConstLhs = lhs;
  struct tm nonConstRhs = rhs;
  return mktime(&nonConstLhs) < mktime(&nonConstRhs);
}
bool operator>(const struct tm &lhs, const struct tm &rhs) { return rhs < lhs; }
bool operator>=(const struct tm &lhs, const struct tm &rhs) {
  return !(lhs < rhs);
}
bool operator<=(const struct tm &lhs, const struct tm &rhs) {
  return !(rhs < lhs);
}

namespace Countdown {

void to_json(json &j, const event &e) {
  j["title"] = e.title;
  j["time"] = e.time;
  if (e.details != "")
    j["details"] = e.details;
  if (e.repeat != "")
    j["repeat"] = e.repeat;
  if (e.theme != "")
    j["theme"] = e.theme;
}
void from_json(const json &j, event &e) {
  j.at("title").get_to(e.title);
  j.at("time").get_to(e.time);
  if (j.contains("details"))
    j.at("details").get_to(e.details);
  else
    e.details = "";
  if (j.contains("repeat"))
    j.at("repeat").get_to(e.repeat);
  else
    e.repeat = "";
  if (j.contains("theme"))
    j.at("theme").get_to(e.theme);
  else
    e.theme = "";
}

void to_json(json &j, const theme &t) {
  j["bg"] = t.bg;
  j["fg"] = t.fg;
  if (!t.events.empty())
    j["events"] = t.events;
}
void from_json(const json &j, theme &t) {
  j.at("bg").get_to(t.bg);
  j.at("fg").get_to(t.fg);
  if (j.contains("events"))
    j.at("events").get_to(t.events);
  else
   t.events = {};
}

bool operator<(const event &lhs, const event &rhs) {
  return lhs.time < rhs.time;
}
bool operator>(const event &lhs, const event &rhs) { return rhs < lhs; }
bool operator>=(const event &lhs, const event &rhs) { return !(lhs < rhs); }
bool operator<=(const event &lhs, const event &rhs) { return !(rhs < lhs); }

bool hasHappened(struct tm *eventToCheck) {
  return mktime(eventToCheck) < std::time(0);
}

bool hasHappened(event *eventToCheck) {
  return hasHappened(&(eventToCheck->time));
}

std::string getCountdownString(const std::string &title,
                               const struct tm &eventTime) {
  std::string result = title;
  result += " is in ";

  struct tm gmEventTime = eventTime;
  time_t eventTimestamp = timegm(&gmEventTime);
  gmEventTime = *gmtime(&eventTimestamp);
  time_t nowTimestamp = time(nullptr);
  struct tm gmNowTime = *gmtime(&nowTimestamp);

  int yearDifference = gmEventTime.tm_year - gmNowTime.tm_year;
  int dayDifference = gmEventTime.tm_yday - gmNowTime.tm_yday;

  bool addCommas = false;

  if (yearDifference > 0 && dayDifference >= 0) {
    result += std::to_string(yearDifference);
    result += " year";
    if (yearDifference != 1)
      result += 's';
    addCommas = true;
  }

  if (dayDifference > 0) {
    if (addCommas)
      result += ", ";
    result += std::to_string(dayDifference);
    result += " day";
    if (dayDifference != 1)
      result += 's';
    addCommas = true;
  }

  time_t timeWithoutDays = (eventTimestamp - nowTimestamp) % (60 * 60 * 24);
  if (timeWithoutDays > 0) {
    int seconds = timeWithoutDays % 60;
    int minutes = (timeWithoutDays % 3600) / 60;

    int hours = (timeWithoutDays / 3600);
    if (addCommas)
      result += ',';
    result += ' ';
    result += zeroPad(std::to_string(hours));
    result += ':';
    result += zeroPad(std::to_string(minutes));
    result += ':';
    result += zeroPad(std::to_string(seconds));
  }
  return result;
}

std::string getCountdownString(const event &e) {
  return getCountdownString(e.title, e.time);
}

std::string zeroPad(const std::string &str, int length) {
  std::string result = str;
  int zeroes = length - result.length();
  if (zeroes > 0)
    result.insert(0, zeroes, '0');
  return result;
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
      newTime.tm_mday += repeatNum;
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

std::string time2string(const time_t &t) {
  struct tm annoyingStruct = *gmtime(&t);
  return time2string(annoyingStruct);
}

struct tm string2time(const std::string &timeString) {
  char timeCharArr[20];
  strcpy(timeCharArr, timeString.c_str());

  struct tm timeStruct;
  strptime(timeCharArr, "%FT%T", &timeStruct);
  timeStruct.tm_isdst = -1;
  time_t timeT = mktime(&timeStruct);

  timeStruct = *gmtime(&timeT);
  return timeStruct;
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
  event e = data["events"].at(index);
  return e;
}
void CountdownData::addEvent(event eventToAdd) {
  data["events"].push_back(eventToAdd);
  sortEvents();
}
void CountdownData::setEvent(int index, event eventToSet) {
  data["events"][index] = eventToSet;
}
void CountdownData::sortEvents() {
  sort(data["events"].begin(), data["events"].end());
}

void CountdownData::loadData() {
  std::ifstream jsonFile(filename);
  data = json::parse(jsonFile); // Will crash on error
  if (fs::is_directory("themes"))
    for (const auto &entry : fs::recursive_directory_iterator("themes")) {
      fs::path themeFilename = entry.path();
      if (themeFilename.extension() == ".json") {
        std::ifstream themeFile(themeFilename);
        if (json::accept(themeFile))
          themes[themeFilename.stem()] = json::parse(themeFile);
        else
          std::cerr << "Invalid JSON: " << themeFilename << std::endl;
      }
    }
  else
    themes["default"] = DEFAULT_THEME;
}

void CountdownData::saveData() const {
  std::ofstream jsonFile(filename);
  jsonFile << std::setw(4) << data << std::endl;
}
std::pair<std::string, std::string>
CountdownData::getTheme(const event &e) const {}

} // namespace Countdown

int main() {
  // Setup ncurses
  /*initscr();*/
  /*cbreak();*/
  /*noecho();*/
  /*keypad(stdscr, TRUE);*/

  Countdown::CountdownData countdown("data.json");
  // struct tm repeat = Countdown::getRepeat(countdown.getEvent(0));
  std::cout << Countdown::getCountdownString(countdown.getEvent(0))
            << std::endl;
  countdown.saveData();

  return 0;
}
