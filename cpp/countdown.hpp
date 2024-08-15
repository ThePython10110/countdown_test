/*******************************************************************************
 * countdown.hpp
 * Defines the Countdown class
 * Also defines the event structure
 * And also the theme structure
 * And also various useful functions
 *******************************************************************************/

#include <ctime>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#ifndef COUNTDOWN_HPP
#define COUNTDOWN_HPP

using json = nlohmann::json;

void to_json(nlohmann ::json &nlohmann_json_j,
             const struct tm &nlohmann_json_t);
void from_json(const nlohmann ::json &nlohmann_json_j,
               struct tm &nlohmann_json_t);

bool operator<(const struct tm &lhs, const struct tm &rhs);
bool operator>(const struct tm &lhs, const struct tm &rhs);
bool operator>=(const struct tm &lhs, const struct tm &rhs);
bool operator<=(const struct tm &lhs, const struct tm &rhs);

namespace Countdown {

struct event {
  std::string title;
  struct tm time;
  std::string details;
  std::string repeat;
  std::string theme;
};

struct theme {
  std::string bg;
  std::string fg;
  std::vector<std::map<std::string, std::string>> events;
};

bool hasHappened(struct tm *eventToCheck);
bool hasHappened(event *eventToCheck);
struct tm getRepeat(const event &eventToRepeat);
std::string time2string(struct tm &timeStruct);
struct tm string2time(const std::string &timeString);
struct tm getNthWeekdayFromDate(const struct tm &startDate,
                                const int &weekdayNum, const int &n);
int getWeekdayNum(const struct tm &dayToCheck, const bool &ofYear);
std::string getCountdownString(const event &e);
std::string zeroPad(const std::string &str, int length = 2);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(event, title, time, details, repeat, theme)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(theme, bg, fg, events)

class CountdownData {
public:
  CountdownData(std::string _filename) : filename(_filename) { loadData(); }
  event getEvent(int index) const;
  void addEvent(event eventToAdd);
  void setEvent(int index, event eventToSet);
  void loadData();
  void saveData() const;
  void sortEvents();

private:
  std::string filename;
  json data; // Why not?
  json themes;
};

} // namespace Countdown

#endif // !COUNTDOWN_HPP
