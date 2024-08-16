# Countdown Test

This is a programming experiment where my goal is to learn languages by making
the same countdown app in several of them. I'll start with C++, then move on to
whatever other programming language I want to learn (or relearn). Maybe Python,
Bash, Rust, JavaScript (HTML frontend?), I don't know. I just think it would be
a good way to learn programming languages. And honestly, I've never made a TUI
before, so that will probably be a good thing to learn.

## The features:

I will hopefully be able to implement all of these...

- All data stored in JSON.
- Themes stored in separate files in a `themes` subdirectory.
- Fields to be stored for each event:
  - Date/time: Stored as `"YYYY-MM-DDThh:mm:ss"`.
  - Title: The title of the event, not allowing newlines.
  - Details (optional): Details of the event, allowing newlines
  - Repeat (optional): If the event repeats, this shows when. More information
    further down.
  - Theme (optional): A theme (either an existing theme name or a full theme
    object) that overrides the global theme (see Themes below).
- User actions:
  - Keyboard and mouse compatibility
  - Edit all fields of an event
  - Add events
  - Remove events (with confirmation).
  - Add, remove, and edit themes (more details on themes below).
- Command line argument to choose config file and theme

## JSON structure:

```json
{
  "theme": "default",
  "events": [
    {
      "title": "Christmas",
      "time": "2024-12-25T00:00:00",
      "repeat": "Y",
      "theme": "special_christmas_theme",
      "details": "A holiday that exists"
    }
  ]
}
```
`"theme"` can either be the name (without the extension) of a JSON file in the
`themes` directory, or a theme JSON object.

For more information about the JSON format, check the schemas in the `schemas`
directory.

## Repeats

I made a separate section for this because I felt like it. Basically, the repeat
value can be stored in two different ways:

- `"[#]u"`
  This is the simplest method. `[#]` is an optional number that defaults to 1 if
  omitted. `u` is a unit, either `s|m|h|D|W|M|Y|C`. After the event happens, this
  amount is added to it. For example, `"Y"` would schedule an event as yearly,
  and `"5D"` would schedule an event to be every five days.
- `"Wu[#][u]"`
  Here, `"W"` is a literal "W" character. Both units must be either `M|Y` (the
  last one is optional, assumed to be `Y` if omitted). It's basically just
  saying "the same occurence of the same day of the week of the `u` every `[#]`
  `[u]`. Which is incredibly confusing. For example, to encode Thanksgiving (the
  fourth Thursday in November), you would create an event set to Thanksgiving
  one year, then set the repeat to be `"WM"`. This will set it to be on the
  fourth Thursday of November every year(assuming that the original event is set
  to the fourth Thursday in November). It's confusing, but that's the best way I
  can think of doing it.

### Notes

Leap years are always respected, since it's literally just incrementing the year
number.

If an event ends up scheduled (for example) on the ninth Sunday of the
month or the 202nd day of March, it will overflow into future months/years.

All dates are stored and processed internally as UTC, only being converted to
local time when necessary.

## Themes

This is the basic format of a theme:

```json
{
    "bg": "transparent",
    "fg": "#ffffff"
    "events": [
            {
                "time": "3D",
                "fg": "#000000",
                "bg": "#ffffff"
            },
            {
                "time": "W",
                "fg": "#ffff00"
            },
            {
                "fg": "#00ffff"
            }
        ]
    ]
}
```
Themes are all about overrides. The default theme (defined in `countdown.hpp`)
is overridden by the top-level theme in the given theme file, which is then
overridden by the `"events"` objects (shorter times have higher precedence, but
are only active if the event is <= that far away), and finally those are
overridden by the `"theme"` property of the events. The `"time"` property is the
same format as the first `"repeat"` format.

If you're confused, I don't care... it makes enough sense for me to code it. The
theme shown above has a global style that sets it to white on transparent.
Events are cyan by default. Events that are <= 1 week away are yellow, and
events that are <= 3 days away are white on black. Of course, this theme would
also be overridden by any event-specific themes...

More than `fg` and `bg` may eventually be added (such as `font` for GUI
versions, if I ever make GUI versions).
