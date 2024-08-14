# Countdown Test

This is a programming experiment where my goal is to learn languages by making the same countdown app in several of them. I'll start with C++, then move on to whatever other programming language I want to learn (or relearn). Maybe Python, Bash, Rust, JavaScript (HTML frontend?), I don't know. I just think it would be a good way to learn programming languages. And honestly, I've never made a TUI before, so that will probably be a good thing to learn.

## The features:

I will hopefully be able to implement all of these...

- All data stored in JSON.
- Themes stored in separate files in a `themes` subdirectory.
- Fields to be stored for each event:
  - Date/time: Stored as `"YYYY-MM-DDThh:mm:ss"`. Dates will be stored in UTC, and time zone offsets will be calculated when the program is run based on the system time zone.
  - Title: The title of the event, not allowing newlines.
  - Details (optional): Details of the event, allowing newlines
  - Repeat (optional): If the event repeats, this shows when. More information further down.
  - Theme (optional): A theme (either an existing theme name or a full theme object) that overrides the global theme (see Themes below).
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
        "title": "Christmas",
        "time": "2024-12-25T00:00:00",
        "repeat": "Y",
    ]
}
```

## Repeats

I made a separate section for this because I felt like it. Basically, the repeat
value can be stored in two different ways:

- `"[#]u"`
This is the simplest method. `[#]` is an optional number that defaults to 1 if
omitted. `u` is a unit, either `s|m|h|D|W|M|Y` (second/minute/hour/day/week/month/year).
After the event happens, this amount is added to it. For example, `"Y"` would
schedule an event as yearly, and `"5D"` would schedule an event to be every five
days.
- `"Wu[#][u]"`
  Here, `"W"` is a literal "W" character. Both units must be either `M|Y` (the
last one is optional, assumed to be `Y` if omitted). It's basically just saying
"the same occurence of the same day of the week of the `u` every `[#]` `[u]`."
Which is incredibly confusing. For example, to encode Thanksgiving (the fourth
Thursday in November), you would create an event set to Thanksgiving one year,
then set the repeat to be `"WM"`. This will set it to be on the fourth Thursday
of November every year(assuming that the original event is set to the fourth
Thursday in November). It's confusing, but that's the best way I can think of
doing it.

### Notes
Leap years are always respected, since it's literally just incrementing the year
number. If an event ends up scheduled (for example) on the ninth Sunday of the
month or the seventy-fourth Friday of the year, it will overflow into future
months/years.

## Themes

This is the basic format of a theme:

```json
{
    "global": {
        "bg": "transparent",
        "fg": "#ffffff"
    },
    "events": [
            {
                "time": "1D",
                "fg": "#000000",
                "bg": "#ffffff"
            },
            {
                "time": "1W",
                "fg": "#ffff00"
            },
            {
                "fg": "#00ffff"
            }
        ]
    ]
}
```

`time` is the amount of time before the event where the style will be active, in
the same format as the first "repeat" format (`"[#]u"`). Shorter times have
precedence, thus a style with `"1D"` will override all properties of a style
with `"1W"`. If `time` is omitted, it is the default (always active, lowest
precedence). Here's some pseudocode:

```
for style in sorted_events { // assume sorted_events has shortest times first.
    if (not style.time) or (style.time <= time) {
        for property, value in style {
            current_style[property] = value
        }
    }
}
```

If you're confused, I don't care... it makes enough sense for me to code it. The
theme shown above has a global style that sets it to white on transparent.
Events are cyan by default. Events that are <= 1 week away are yellow, and
events that are >= 1 day away are white on black.

More than `fg` and `bg` may eventually be added (such as `font` for GUI
versions, if I ever make GUI versions). I would also like to add a `format`
field where you could actually choose how the events are formatted, but I
couldn't think of a good way to do that.
