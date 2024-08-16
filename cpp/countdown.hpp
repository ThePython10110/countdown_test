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

const json DEFAULT_THEME = {
  {"bg", "#1f2329"},
  {"fg", "#a0a8b7"},
  {"events", {
    {
      {"time", "D"},
      {"fg", "#e55561"}
    },
    {
      {"time", "W"},
      {"fg", "#e2b86b"}
    },
    {
      {"time", "M"},
      {"fg", "#4fa6ed"}
    },
    {
      {"fg", "#8ebd6b"}
    }
  }}
};
// Serializes a tm struct as JSON
void to_json(json &j, const struct tm &t);
// Deserializes a tm struct from JSON
void from_json(const json &j, struct tm &t);

// Compares two dates
bool operator<(const struct tm &lhs, const struct tm &rhs);
// Compares two dates
bool operator>(const struct tm &lhs, const struct tm &rhs);
// Compares two dates
bool operator>=(const struct tm &lhs, const struct tm &rhs);
// Compares two dates
bool operator<=(const struct tm &lhs, const struct tm &rhs);

namespace Countdown {

// Contains details about an event
struct event {
  std::string title;
  struct tm time;
  std::string details;
  std::string repeat;
  std::string theme;
};

// Contains details about a theme
struct theme {
  std::string bg;
  std::string fg;
  std::vector<std::map<std::string, std::string>> events;
};

// Serializes an event struct as JSON
void to_json(json &j, const event &e);
// Deserializes an event struct from JSON
void from_json(const json &j, event &e);
// Serializes a theme struct as JSON
void to_json(json &j, const theme &t);
// Deserializes a theme struct from JSON
void from_json(const json &j, theme &t);

// Compares two events
bool operator<(const event &lhs, const event &rhs);
// Compares two events
bool operator>(const event &lhs, const event &rhs);
// Compares two events
bool operator>=(const event &lhs, const event &rhs);
// Compares two events
bool operator<=(const event &lhs, const event &rhs);

// Checks if an event is in the past
bool hasHappened(struct tm *eventToCheck);
// Checks if an event is in the past
bool hasHappened(event *eventToCheck);

// Calculates the next occurence of an event with a repeat value
struct tm getRepeat(const event &eventToRepeat);

// Converts a tm struct to a string
std::string time2string(struct tm &timeStruct);
// Converts a string to a tm struct
struct tm string2time(const std::string &timeString);

// Given a weekday, a number, and a starting date, gives the nth matching
// weekday starting from the start date.
struct tm getNthWeekdayFromDate(const struct tm &startDate,
                                const int &weekdayNum, const int &n);
// Calculates the number of times the given day of the week has appeared
// previously in the month/year
int getWeekdayNum(const struct tm &dayToCheck, const bool &ofYear);

// Returns a string showing how long until an event happens
std::string getCountdownString(const event &e);

// Pads a string with zeroes
std::string zeroPad(const std::string &str, int length = 2);

// Stores data for a countdown
class CountdownData {
public:
  // Creates a CountdownData object
  CountdownData(std::string _filename) : filename(_filename) { loadData(); }
  // Gets the event with the specified index
  event getEvent(int index) const;
  // Appends an event to the list, then sorts the list
  void addEvent(event eventToAdd);
  // Sets the event at the specified index
  void setEvent(int index, event eventToSet);
  // Loads data from a file
  void loadData();
  // Saves data to a file
  void saveData() const;
  // Sorts the list of events
  void sortEvents();
  // Gets the theme for an event
  std::pair<std::string, std::string> getTheme(const event& e) const;

private:
  std::string filename;
  json data; // Why not?
  json themes;
};

} // namespace Countdown

#endif // !COUNTDOWN_HPP
