
# ADate - Amiga Date Utility

## Overview

**ADate** is a versatile date and time utility for the Commodore Amiga platform. It allows you to display, convert, and set system date and time with a range of supported formats and features. With epoch handling, custom formats, and support for multiple date standards (ISO 8601, RFC 2822, and more), ADate is designed for developers, system administrators, and Amiga enthusiasts who need precise date-time manipulation.

---

## Features

- **Date and Time Display**
  - Display the current system date and time.
  - Customize output with user-defined formats.

- **Date Parsing**
  - Parse and display specific dates using various input formats:
    - **ISO 8601**: `YYYY-MM-DDTHH:MM:SS`
    - **RFC 2822**: `Day, DD Mon YYYY HH:MM:SS +ZZZZ`
    - **Custom Formats**: `YYYY-MM-DD`, `DD-MM-YYYY`, `YYYYMMDD HH:MM`
    - **Relative Dates**: `yesterday`, `today`, `tomorrow`

- **Epoch Conversion**
  - Convert between Amiga epoch (`1978-01-01`) and Unix epoch (`1970-01-01`).
  - Specify epochs directly using `epoch="@<value>"` (Amiga) or `epoch="<value>"` (Unix).

- **System Time Setting**
  - Set the Amiga system time to a specific date and time using the `SET` option.

- **Help Option**
  - Access detailed usage instructions and examples with the `HELP` option.

- **Advanced Format Options**
  - Customize date and time output with specifiers like `%Y`, `%m`, `%d`, `%H`, `%M`, `%S`, and more.
  - Linux-style `+FORMAT` input is supported (e.g., `format="+%F %T"`).

---

## Usage

### Basic Syntax
```bash
adate [OPTIONS]
```

### Options

| Option         | Description                                                                 |
|----------------|-----------------------------------------------------------------------------|
| `UTC`          | Display time in UTC.                                                       |
| `FORMAT/K`     | Custom output format (e.g., `%Y-%m-%d %H:%M:%S`).                          |
| `DATE/K`       | Specify a date to parse and display. Supports multiple formats.            |
| `TIME/K`       | Specify time (used with `DATE`).                                           |
| `SET/S`        | Set the system date/time to the provided value.                            |
| `EPOCH/K`      | Specify epoch time. Use `@<value>` for Amiga epoch or `<value>` for Unix.  |
| `HELP/S`       | Display usage instructions and examples.                                   |

### Format Specifiers
Use the `FORMAT` option to customize the output. Supported specifiers include:

| Specifier | Description                      |
|-----------|----------------------------------|
| `%Y`      | Year (4 digits)                 |
| `%y`      | Year (2 digits)                 |
| `%m`      | Month (01–12)                   |
| `%d`      | Day of the month (01–31)        |
| `%H`      | Hour (00–23)                    |
| `%M`      | Minute (00–59)                  |
| `%S`      | Second (00–59)                  |
| `%a`      | Abbreviated weekday name        |
| `%A`      | Full weekday name               |
| `%D`      | Date as `%m/%d/%y`              |
| `%F`      | Date as `%Y-%m-%d`              |
| `%j`      | Day of the year (001–366)       |
| `%U`      | Week number (Sunday start)      |
| `%W`      | Week number (Monday start)      |
| `%p`      | AM/PM                           |
| `%P`      | am/pm                           |
| `%R`      | Time as `%H:%M`                 |
| `%r`      | 12-hour time with AM/PM         |
| `%s`      | Seconds since Unix epoch        |
| `%T`      | Time as `%H:%M:%S`              |
| `%I`      | Hour (01–12)                    |
| `%B`      | Full month name                 |
| `%C`      | Century                         |
| `%c`      | Locale-style date and time      |
| `%x`      | Locale-style date               |
| `%X`      | Locale-style time               |
| `%z`      | Timezone offset (`+0000`)       |
| `%Z`      | Timezone name (`UTC`)           |
| `%%`      | Literal percent sign            |

---

## Examples

### Display Current Date and Time
```bash
adate
```

### Display Date in UTC
```bash
adate utc
```

### Custom Format
```bash
adate format="%Y-%m-%d %H:%M:%S"
adate format="+%F %T"
adate date="+%s"
```

### Parse a Specific Date
```bash
adate date="2024-12-10T14:23:45"
adate date="Tue, 10 Dec 2024 14:23:45 +0100"
adate date="12-12-2012"
```

### Use Epoch
```bash
adate epoch="@123456"
adate epoch="1672531199"
```

### Set System Date and Time
```bash
adate date="2024-12-10T14:23:45" set
```

### Show Help
```bash
adate help
```

---

## Installation

1. Compile the program using your preferred Amiga C compiler.
2. Place the binary in a directory included in your `PATH` (e.g., `C:`).

---

## Contributing

Contributions are welcome! Feel free to submit issues, feature requests, or pull requests on the GitHub repository.

---

## License

This project is licensed under the MIT License.

---

## Linux `date` Compatibility Gaps

ADate supports many common formatting tokens, but it is not yet a full replacement for GNU/Linux `date`.

- No equivalent to GNU `date -d` free-form natural-language parsing beyond fixed strings (`today`, `tomorrow`, `yesterday`).
- No timezone database handling (`TZ`, named zones, DST rules); `%z` and `%Z` are currently fixed to `+0000` / `UTC`.
- No nanosecond specifier (`%N`) or RFC-3339 convenience flags.
- No long-option GNU CLI flags (`--date`, `--set`, `--utc`, `--iso-8601`, etc.); AmigaDOS keyword options are used instead.
- No locale-driven translations for month/day names (output names are fixed in English).
