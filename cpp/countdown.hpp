/*******************************************************************************
 * countdown.hpp
 * Defines the Countdown class
 * Also defines the event structure
 * And also the theme structure
 * And also various useful functions
*******************************************************************************/

#include <ncurses.h>
#include <nlohmann/json.hpp>
#include <string>
#include <map>

namespace Countdown {

struct event {
  std::string title;
  struct tm time;
  std::string details;
  std::string repeat;
  std::string theme;
};

bool hasHappened(const struct tm &t);
bool hasHappened(event e);
struct tm getRepeat(event e);
struct tm getRepeat(struct tm t);

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
