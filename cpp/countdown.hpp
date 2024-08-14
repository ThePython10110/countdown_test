/*******************************************************************************
 * countdown.hpp
 * Defines the Countdown class
 * Also defines the event structure
 * And also the theme structure
 * And also various useful functions
 *******************************************************************************/

#include <map>
#include <ncurses.h>
#include <nlohmann/json.hpp>
#include <string>

namespace Countdown {

struct event {
  std::string title;
  struct tm time;
  std::string details;
  std::string repeat;
  std::string theme;
};

bool hasHappened(struct tm *eventToCheck);
bool hasHappened(event *eventToCheck);
struct tm getRepeat(const event &eventToRepeat);
std::string time2string(const struct tm &timeStruct);
struct tm string2time(const std::string &timeString);
struct tm getNthWeekdayAfterDate(const struct tm &startDate, const int &weekdayNum,
                           const int &n);
int getWeekdayNum(const struct tm &dayToCheck, const bool &ofYear);

enum weekdays { SUN, MON, TUE, WED, THU, FRI, SAT };

class Countdown {
public:
  event getEvent(const int &index) const;
  void addEvent(event event);
  void setEvent(int index, event event);
  bool loadData();
  void saveData() const;

private:
  std::string filename;
  std::map<std::string, event> events;
};

} // namespace Countdown
