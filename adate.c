//Amiga ADate Program - DBanfi 2024
#include <exec/types.h>
#include <exec/io.h>
#include <exec/ports.h>
#include <devices/timer.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TEMPLATE "UTC/S,FORMAT/K,DATE/K,TIME/K,SET/S,EPOCH/K,HELP/S"
#define OPT_UTC 0
#define OPT_FORMAT 1
#define OPT_DATE 2
#define OPT_TIME 3
#define OPT_SET 4
#define OPT_EPOCH 5
#define OPT_HELP 6  // Add an index for the new HELP option

struct Options {
    BOOL utc;
    char *format;
    char *datestr;
    char *timestr;
    BOOL set;
    char *epoch;
    BOOL help; 
};

BOOL parse_arguments(LONG *args, struct Options *opts);
void free_arguments(struct Options *opts);
BOOL parse_date_string(const char *datestr, int *year, int *month, int *day, int *hours, int *minutes, struct TimeVal *tv);
int calculate_days_since_1978(int year, int month, int day);
void calculate_year_month_day(int days, int *year, int *month, int *day);
void set_system_time(struct TimeVal *tv);
void show_date(const struct TimeVal *tv, const char *format);
BOOL adjust_date_time(struct TimeVal *tv, const struct Options *opts);
int DaysInMonth(int year, int month);
void DateStampToDate(const struct TimeVal *tv, int *year, int *month, int *day);
void convert_unix_to_timeval(const char *epoch, struct TimeVal *tv);
void show_date_unix(const struct TimeVal *tv, const char *format);
void timeval_to_datestamp(const struct TimeVal *tv, struct DateStamp *ds);
int calculate_day_of_year(int year, int month, int day);
/* Calculate the day of the year (001–366) for a given date.*/
int calculate_week_number(int year, int month, int day, int week_start);
/* Calculate the week number of the year.*/
int month_name_to_number(const char *month_name);
/* Convert a three-letter month name to its corresponding month number.*/

int main() {
    LONG args[7] = {0}; // Update array size to match all arguments in TEMPLATE
    struct Options opts = {0}; // Ensure all fields are initialized to 0/NULL
    struct RDArgs *rdargs = NULL;
    struct TimeVal tv = {0};

    // Initialize options explicitly
    opts.utc = FALSE;
    opts.format = NULL;
    opts.datestr = NULL;
    opts.timestr = NULL;
    opts.set = FALSE;
    opts.epoch = NULL;
    opts.help = FALSE;

    // Variables for parsed date and time
    int year = 0, month = 0, day = 0, hours = 0, minutes = 0;

    rdargs = ReadArgs(TEMPLATE, args, NULL);
    if (!rdargs) {
        PrintFault(IoErr(), "Error parsing arguments");
        printf("Usage: adate [UTC/S] [FORMAT/K] [DATE/K] [TIME/K] [SET/S] [EPOCH/K]\n");
        return RETURN_ERROR;
    }

    parse_arguments(args, &opts);
    FreeArgs(rdargs);
    
       /*Help Option Seclected*/

    if (opts.help) {
    printf("ADate v1.0 10/12/24 - Usage: adate [OPTIONS]\n"
           "\nOPTIONS:\n"
           "  UTC        - Display time in UTC.\n"
           "  FORMAT     - Custom output format (e.g., %%Y-%%m-%%d %%H:%%M:%%S).\n"
           "  DATE       - Parse and display a specific date/time.\n"
           "               Supported formats:\n"
           "                 - ISO 8601: YYYY-MM-DDTHH:MM:SS\n"
           "                 - RFC 2822: Day, DD Mon YYYY HH:MM:SS +ZZZZ\n"
           "                 - Custom: YYYY-MM-DD HH:MM:SS\n"
           "  TIME       - Specify the time (used with DATE).\n"
           "  SET        - Set the system date/time to the provided date/time.\n"
           "  EPOCH      - Specify time as seconds since the Amiga epoch (1978-01-01).\n"
           "  HELP       - Display this help message.\n"
           "\nFORMAT SPECIFIERS:\n"
           "  %%Y  - Year (4 digits)\n"
           "  %%y  - Year (2 digits)\n"
           "  %%m  - Month (01-12)\n"
           "  %%d  - Day of the month (01-31)\n"
           "  %%H  - Hour (00-23)\n"
           "  %%M  - Minute (00-59)\n"
           "  %%S  - Second (00-59)\n"
           "  %%a  - Abbreviated weekday name (e.g., Sun)\n"
           "  %%A  - Full weekday name (e.g., Sunday)\n"
           "  %%n  - Newline\n"
           "  %%t  - Tab\n"
           "\nEXAMPLES:\n"
           "  adate date=\"2024-12-10T14:23:45\"\n"
           "  adate date=\"Tue, 10 Dec 2024 14:23:45 +0100\"\n"
           "  adate epoch=\"@0\"\n");
    return RETURN_OK;
}

    if (opts.epoch) {

    convert_unix_to_timeval(opts.epoch, &tv);
    // Convert to DateStamp (optional for debugging)
    struct DateStamp ds;
    timeval_to_datestamp(&tv, &ds);

    // Display the converted date
   
    }
    // Handle 'set' command
    if (opts.set) {
        if (opts.datestr) {
            // Parse provided date string
            if (!parse_date_string(opts.datestr, &year, &month, &day, &hours, &minutes, &tv)) {
                printf("Error: Failed to parse date string '%s'.\n", opts.datestr);
                return RETURN_ERROR;
            }
            tv.tv_secs = calculate_days_since_1978(year, month, day) * 86400 +
                         hours * 3600 + minutes * 60;
            tv.tv_micro = 0;

        } else {
            // Default to current system time
            struct MsgPort *timerPort = CreateMsgPort();
            struct TimeRequest *timerReq = (struct TimeRequest *)CreateIORequest(timerPort, sizeof(struct TimeRequest));
            if (timerReq && OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)timerReq, 0) == 0) {
                timerReq->tr_node.io_Command = TR_GETSYSTIME;
                DoIO((struct IORequest *)timerReq);
                tv = timerReq->tr_time;
                CloseDevice((struct IORequest *)timerReq);
                } else {
                printf("Error: Failed to retrieve current system time.\n");
                return RETURN_ERROR;
            }
            if (timerReq) DeleteIORequest((struct IORequest *)timerReq);
            if (timerPort) DeleteMsgPort(timerPort);
        }
        // Set the system time
        set_system_time(&tv);
        return RETURN_OK;
    }

    // Show date without setting it
    if (opts.datestr) {
        if (!parse_date_string(opts.datestr, &year, &month, &day, &hours, &minutes, &tv)) {
            printf("Error: Failed to parse date string '%s'.\n", opts.datestr);
            return RETURN_ERROR;
        }
        tv.tv_secs = calculate_days_since_1978(year, month, day) * 86400 +
                     hours * 3600 + minutes * 60;
        tv.tv_micro = 0;

        show_date(&tv, opts.format ? opts.format : "%Y-%m-%d %H:%M:%S");
        return RETURN_OK;
        }

    // Handle epoch conversion
    if (opts.epoch) {

    convert_unix_to_timeval(opts.epoch, &tv);  // Convert Unix epoch to AmigaOS timeval

    // Optional: Convert to DateStamp for AmigaDOS compatibility
    struct DateStamp ds;
    timeval_to_datestamp(&tv, &ds);

    // Show the converted date
    show_date(&tv, opts.format ? opts.format : "%Y-%m-%d %H:%M:%S");  // Use existing show_date
    return RETURN_OK;
}


    // Default case: Show current system time
    struct MsgPort *timerPort = CreateMsgPort();
    struct TimeRequest *timerReq = (struct TimeRequest *)CreateIORequest(timerPort, sizeof(struct TimeRequest));
    if (timerReq && OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)timerReq, 0) == 0) {
        timerReq->tr_node.io_Command = TR_GETSYSTIME;
        DoIO((struct IORequest *)timerReq);
        tv = timerReq->tr_time;
        CloseDevice((struct IORequest *)timerReq);
    }
    if (timerReq) DeleteIORequest((struct IORequest *)timerReq);
    if (timerPort) DeleteMsgPort(timerPort);
    show_date(&tv, opts.format ? opts.format : "%Y-%m-%d %H:%M:%S");
	free_arguments(&opts);
	
    return RETURN_OK;
}

BOOL parse_arguments(LONG *args, struct Options *opts) {
    opts->utc = args[OPT_UTC] != 0;
    opts->format = args[OPT_FORMAT] ? strdup((char *)args[OPT_FORMAT]) : NULL; // Ensure strdup works
    opts->datestr = args[OPT_DATE] ? strdup((char *)args[OPT_DATE]) : NULL;
    opts->timestr = args[OPT_TIME] ? strdup((char *)args[OPT_TIME]) : NULL;
    opts->set = args[OPT_SET] != 0;
    opts->epoch = args[OPT_EPOCH] ? strdup((char *)args[OPT_EPOCH]) : NULL;
    opts->help = args[OPT_HELP] != 0;  // Parse the HELP option
    return TRUE;
}

void free_arguments(struct Options *opts) {
    if (opts) {
		if (opts->format) free(opts->format);
		if (opts->datestr) free(opts->datestr);
		if (opts->timestr) free(opts->timestr);
		if (opts->epoch) free(opts->epoch);
	}
}

BOOL adjust_date_time(struct TimeVal *tv, const struct Options *opts) {
    struct MsgPort *timerPort = CreateMsgPort();
    if (!timerPort) return FALSE;

    struct TimeRequest *timerReq = (struct TimeRequest *)CreateIORequest(timerPort, sizeof(struct TimeRequest));
    if (!timerReq) {
        DeleteMsgPort(timerPort);
        return FALSE;
    }

    if (OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)timerReq, 0) != 0) {
        DeleteIORequest((struct IORequest *)timerReq);
        DeleteMsgPort(timerPort);
        return FALSE;
    }

    timerReq->tr_node.io_Command = TR_GETSYSTIME;
    DoIO((struct IORequest *)timerReq);
    *tv = timerReq->tr_time;

    if (opts->datestr) {
        int year, month, day, hours = (tv->tv_secs % 86400) / 3600, minutes = (tv->tv_secs % 3600) / 60;
        if (!parse_date_string(opts->datestr, &year, &month, &day, &hours, &minutes, tv)) {
            CloseDevice((struct IORequest *)timerReq);
            DeleteIORequest((struct IORequest *)timerReq);
            DeleteMsgPort(timerPort);
            return FALSE;
        }
        int days = calculate_days_since_1978(year, month, day);
        tv->tv_secs = days * 86400 + hours * 3600 + minutes * 60;
    }

    CloseDevice((struct IORequest *)timerReq);
    DeleteIORequest((struct IORequest *)timerReq);
    DeleteMsgPort(timerPort);
    return TRUE;
}

void convert_unix_to_timeval(const char *epoch, struct TimeVal *tv) {
    if (!epoch) return;

    if (epoch[0] == '@') {
        // Handle Amiga epoch explicitly
        if (tv) {
        long long amiga_seconds = atoll(epoch + 1); // Convert value after '@' to seconds
        tv->tv_secs = (ULONG)amiga_seconds; // Set the time value directly
        tv->tv_micro = 0;
        return;
    }


    // Handle Unix epoch
    long long unix_seconds = atoll(epoch);
    long long amiga_seconds = unix_seconds - 252460800; // Adjust for 8 years

    // Clamp to Amiga range if needed
    if (amiga_seconds < 0) amiga_seconds = 0;
    if (amiga_seconds > 4294967295LL) amiga_seconds = 4294967295LL;

    if (tv) {
        tv->tv_secs = (ULONG)amiga_seconds;
        tv->tv_micro = 0;
    }
    }
}

void timeval_to_datestamp(const struct TimeVal *tv, struct DateStamp *ds) {
    // Convert AmigaOS tv_secs to DateStamp format
    ds->ds_Days = tv->tv_secs / 86400;       // Days since 1978-01-01
    ds->ds_Minute = (tv->tv_secs % 86400) / 60;  // Minutes past midnight
    ds->ds_Tick = (tv->tv_secs % 60) * 50;   // Ticks in the current minute

}


void show_date_unix(const struct TimeVal *tv, const char *format) {
    struct TimeVal unix_tv = *tv;

    // Adjust back from Amiga epoch to Unix epoch
    unix_tv.tv_secs += 252460800;

    show_date(&unix_tv, format);
}





const char *day_names[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char *day_abbr[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

int calculate_day_of_week(int year, int month, int day) {
    // Days since January 1, 1978 (Amiga epoch)
    int days_since_1978 = calculate_days_since_1978(year, month, day);

    // January 1, 1978 was a Sunday, so we offset by 7
    return (days_since_1978 + 7) % 7; // 0 = Sunday, 1 = Monday, ..., 6 = Saturday
}

int calculate_day_of_year(int year, int month, int day) {
    int days = 0;
    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Check for leap year
    if (month > 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) {
        days_in_month[1] = 29;
    }

    for (int i = 0; i < month - 1; i++) {
        days += days_in_month[i];
    }
    days += day;

    return days;
}

int calculate_week_number(int year, int month, int day, int week_start) {
    int day_of_year = calculate_day_of_year(year, month, day);

    // Calculate the day of the week for January 1st
    int jan1_weekday = (calculate_day_of_week(year, 1, 1) - week_start + 7) % 7;

    // Adjust the day of year to the nearest week start
    int adjusted_day = day_of_year + jan1_weekday;

    return adjusted_day / 7;
}



BOOL parse_date_string(const char *datestr, int *year, int *month, int *day, int *hours, int *minutes, struct TimeVal *tv) {
    int seconds = 0;                  // Declare 'seconds' for time parsing
    char month_name[4] = {0};         // Declare 'month_name' for RFC 2822 parsing
    *hours = *minutes = 0;            // Default time values

    // Set the current year if not already initialized
    if (!*year) {
        struct TimeVal now;
        struct MsgPort *timerPort = CreateMsgPort();
        struct TimeRequest *timerReq = (struct TimeRequest *)CreateIORequest(timerPort, sizeof(struct TimeRequest));
        if (timerReq && OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)timerReq, 0) == 0) {
            timerReq->tr_node.io_Command = TR_GETSYSTIME;
            DoIO((struct IORequest *)timerReq);
            now = timerReq->tr_time;
            CloseDevice((struct IORequest *)timerReq);
        }
        if (timerReq) DeleteIORequest((struct IORequest *)timerReq);
        if (timerPort) DeleteMsgPort(timerPort);

        DateStampToDate(&now, year, NULL, NULL);
    }

    // Handle POSIX Timestamp: "@<epoch>"
    if (datestr[0] == '@') {
        long long unix_seconds = atoll(datestr + 1); // Parse the value after '@'
        long long amiga_seconds = unix_seconds - 252460800; // Adjust for Unix to Amiga epoch

        // Clamp to valid Amiga range
        if (amiga_seconds < 0) {
            amiga_seconds = 0; // Set to the start of Amiga epoch
        } else if (amiga_seconds > 4294967295LL) {
            amiga_seconds = 4294967295LL; // Clamp to max 32-bit value
        }

        tv->tv_secs = (ULONG)amiga_seconds;
        tv->tv_micro = 0;

        // Calculate date and time from Amiga epoch seconds
        calculate_year_month_day(tv->tv_secs / 86400, year, month, day); // Days since 1978-01-01
        *hours = (tv->tv_secs % 86400) / 3600; // Extract hours
        *minutes = (tv->tv_secs % 3600) / 60;  // Extract minutes
        int seconds = tv->tv_secs % 60;        // Extract seconds
        return TRUE;
    }

    //Detect UK - USA shot format
    if (strlen(datestr) == 8 && isdigit(datestr[0])) {
        int y, m, d;

        // Try parsing as YYYYMMDD
        if (sscanf(datestr, "%4d%2d%2d", &y, &m, &d) == 3 && m >= 1 && m <= 12 && d >= 1 && d <= DaysInMonth(y, m)) {
            *year = y;
            *month = m;
            *day = d;
            return TRUE;
        }

        // If YYYYMMDD failed, try parsing as DDMMYYYY
        if (sscanf(datestr, "%2d%2d%4d", &d, &m, &y) == 3 && m >= 1 && m <= 12 && d >= 1 && d <= DaysInMonth(y, m)) {
            *year = y;
            *month = m;
            *day = d;
            return TRUE;
        }

        printf("Error: Invalid date format '%s'.\n", datestr);
        return FALSE;
    }

  // Check for YYYY-MM-DD
    if (strlen(datestr) >= 10 && datestr[4] == '-') {
        if (sscanf(datestr, "%4d-%2d-%2d", year, month, day) == 3) {
            if (*month >= 1 && *month <= 12 && *day >= 1 && *day <= DaysInMonth(*year, *month)) {
                return TRUE;
            }
            printf("Error: Invalid date format 'YYYY-MM-DD': %s\n", datestr);
            return FALSE;
        }
    }

    // Check for DD-MM-YYYY
    if (strlen(datestr) >= 10 && datestr[2] == '-') {
        if (sscanf(datestr, "%2d-%2d-%4d", day, month, year) == 3) {
            if (*month >= 1 && *month <= 12 && *day >= 1 && *day <= DaysInMonth(*year, *month)) {
                return TRUE;
            }
            printf("Error: Invalid date format 'DD-MM-YYYY': %s\n", datestr);
            return FALSE;
        }
    }

  // Handle combined date-time: "YYYYMMDD HH:MM" (USA format)
    if (isdigit(datestr[0]) && strlen(datestr) >= 13) { // Check if first character is a digit and string is long enough
        int temp_year;
        sscanf(datestr, "%4d", &temp_year); // Extract first 4 digits as the year
        if (temp_year >= 1978 && temp_year <= 9999) { // Valid year range for Amiga
            if (sscanf(datestr, "%4d%2d%2d %2d:%2d", year, month, day, hours, minutes) == 5) {
                if (*month >= 1 && *month <= 12 && *day >= 1 && *day <= DaysInMonth(*year, *month) &&
                    *hours >= 0 && *hours <= 23 && *minutes >= 0 && *minutes <= 59) {
                    return TRUE;
                }
                printf("Error: Invalid combined date/time format (YYYYMMDD HH:MM).\n");
                return FALSE;
            }
        }
    }

    // Handle UK format: "DDMMYYYY HH:MM"
    if (isdigit(datestr[0]) && strlen(datestr) >= 13) { // Check if first character is a digit and string is long enough
        if (sscanf(datestr, "%2d%2d%4d %2d:%2d", day, month, year, hours, minutes) == 5) {
            if (*month >= 1 && *month <= 12 && *day >= 1 && *day <= DaysInMonth(*year, *month) &&
                *hours >= 0 && *hours <= 23 && *minutes >= 0 && *minutes <= 59) {
                return TRUE;
            }
            printf("Error: Invalid UK combined date/time format (DDMMYYYY HH:MM).\n");
            return FALSE;
        }
    }

  // RFC 2822
    if (strchr(datestr, ',') && sscanf(datestr, "%*[^,], %2d %3s %4d %2d:%2d:%2d %*s", day, month_name, year, hours, minutes, &seconds) == 6) {
        *month = month_name_to_number(month_name);
        if (*month >= 1 && *month <= 12 && *day >= 1 && *day <= DaysInMonth(*year, *month) &&
            *hours >= 0 && *hours <= 23 && *minutes >= 0 && *minutes <= 59) {
            tv->tv_secs = calculate_days_since_1978(*year, *month, *day) * 86400 +
                          (*hours) * 3600 + (*minutes) * 60 + seconds;
            tv->tv_micro = 0;
            return TRUE;
        }
        printf("Error: Invalid RFC 2822 date: %s\n", datestr);
    }

    // ISO 8601
    if (strchr(datestr, 'T')) {
        int scanned = sscanf(datestr, "%4d-%2d-%2dT%2d:%2d:%2d", year, month, day, hours, minutes, &seconds);
        if (scanned >= 3 && *month >= 1 && *month <= 12 && *day >= 1 && *day <= DaysInMonth(*year, *month) &&
            *hours >= 0 && *hours <= 23 && *minutes >= 0 && *minutes <= 59) {
            tv->tv_secs = calculate_days_since_1978(*year, *month, *day) * 86400 +
                          (*hours) * 3600 + (*minutes) * 60 + seconds;
            tv->tv_micro = 0;
            return TRUE;
        }
        printf("Error: Invalid ISO 8601 date: %s\n", datestr);
    }
    // Custom Email-Friendly Format
    if (sscanf(datestr, "%4d-%2d-%2d %2d:%2d:%2d", year, month, day, hours, minutes, &seconds) == 6 &&
        *month >= 1 && *month <= 12 && *day >= 1 && *day <= DaysInMonth(*year, *month) &&
        *hours >= 0 && *hours <= 23 && *minutes >= 0 && *minutes <= 59) {
        tv->tv_secs = calculate_days_since_1978(*year, *month, *day) * 86400 +
                      (*hours) * 3600 + (*minutes) * 60 + seconds;
        tv->tv_micro = 0;
        return TRUE;
    }

       // Handle "yesterday", "today", "tomorrow"
        if (strcasecmp(datestr, "yesterday") == 0 || 
        strcasecmp(datestr, "today") == 0 || 
        strcasecmp(datestr, "tomorrow") == 0) {
    
        struct MsgPort *timerPort = CreateMsgPort();
        struct TimeRequest *timerReq = (struct TimeRequest *)CreateIORequest(timerPort, sizeof(struct TimeRequest));
        if (timerReq && OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)timerReq, 0) == 0) {
        timerReq->tr_node.io_Command = TR_GETSYSTIME;
        DoIO((struct IORequest *)timerReq);
        *tv = timerReq->tr_time; // Get current system time
        CloseDevice((struct IORequest *)timerReq);
    }
    if (timerReq) DeleteIORequest((struct IORequest *)timerReq);
    if (timerPort) DeleteMsgPort(timerPort);

    // Adjust tv_secs for relative dates
    if (strcasecmp(datestr, "yesterday") == 0) {
        tv->tv_secs -= 86400; // Subtract one day
    } else if (strcasecmp(datestr, "tomorrow") == 0) {
        tv->tv_secs += 86400; // Add one day
    }

    // Calculate year, month, day, and retain the time of day
    calculate_year_month_day(tv->tv_secs / 86400, year, month, day);

    // Extract time of day (hours, minutes, seconds) from tv_secs
    *hours = (tv->tv_secs % 86400) / 3600;
    *minutes = (tv->tv_secs % 3600) / 60;
    int seconds = tv->tv_secs % 60;
    return TRUE;
}

    // Handle custom format strings (+%Y-%m-%d %H:%M:%S)
    if (datestr[0] == '+') {
        printf("Custom formatting is not supported in this path.\n");
        return FALSE;
    }

    // Handle UK format: "DDMMYYYY"
    if (sscanf(datestr, "%2d%2d%4d", day, month, year) == 3) {
        if (*month >= 1 && *month <= 12 && *day >= 1 && *day <= DaysInMonth(*year, *month)) {
            return TRUE;
        }
        printf("Error: Invalid UK date format (DDMMYYYY).\n");
        return FALSE;
    }

    // Handle MMDDhhmm format
    if (sscanf(datestr, "%2d%2d%2d%2d", month, day, hours, minutes) == 4) {
        int max_day = DaysInMonth(*year, *month);
        if (*month >= 1 && *month <= 12 &&
            *day >= 1 && *day <= max_day &&
            *hours >= 0 && *hours <= 23 &&
            *minutes >= 0 && *minutes <= 59) {
            return TRUE;
        }
        printf("Error: Invalid MMDDhhmm format.\n");
        return FALSE;
    }

    printf("Error: Invalid date format '%s'. Supported formats include:\n", datestr);
    printf("  - YYYYMMDD HH:MM\n  - YYYY-MM-DD HH:MM\n  - MMDDhhmm\n  - DD-MM-YYYY\n  - DDMMYYYY\n");
    return FALSE;
}


int calculate_days_since_1978(int year, int month, int day) {
    int days = 0;
    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (year >= 1978) {
        // Forward calculation for years >= 1978
        for (int y = 1978; y < year; y++) {
            days += (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) ? 366 : 365;
        }
        for (int m = 0; m < month - 1; m++) {
            days += days_in_month[m];
            if (m == 1 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) {
                days++;
            }
        }
        days += day - 1;
    } else {
        // Backward calculation for years < 1978
        for (int y = 1977; y >= year; y--) {
            days -= (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) ? 366 : 365;
        }
        for (int m = 11; m >= month; m--) {
            days -= days_in_month[m];
            if (m == 1 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) {
                days--;
            }
        }
        days -= (days_in_month[month - 1] - day); // Subtract days remaining in the current month
    }

    return days;
}



int DaysInMonth(int year, int month) {
    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) {
        return 29; // Leap year adjustment
    }
    if (month < 1 || month > 12) {
        return 0; // Invalid month
    }
    return days_in_month[month - 1];
}


void DateStampToDate(const struct TimeVal *tv, int *year, int *month, int *day) {
    calculate_year_month_day(tv->tv_secs / (24 * 60 * 60), year, month, day);
}

void set_system_time(struct TimeVal *tv) {
    struct MsgPort *timerPort = CreateMsgPort();
    if (!timerPort) {
        printf("Error: Unable to create message port.\n");
        return;
    }

    struct TimeRequest *timerReq = (struct TimeRequest *)CreateIORequest(timerPort, sizeof(struct TimeRequest));
    if (!timerReq) {
        printf("Error: Unable to create IO request.\n");
        DeleteMsgPort(timerPort);
        return;
    }

    if (OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)timerReq, 0) != 0) {
        printf("Error: Unable to open timer device.\n");
        DeleteIORequest((struct IORequest *)timerReq);
        DeleteMsgPort(timerPort);
        return;
    }

    timerReq->tr_node.io_Command = TR_SETSYSTIME;
    timerReq->tr_time = *tv;
    if (DoIO((struct IORequest *)timerReq) != 0) {
        printf("Error: Failed to set system time.\n");
    } else {
        printf("System time successfully updated.\n");
    }

    CloseDevice((struct IORequest *)timerReq);
    DeleteIORequest((struct IORequest *)timerReq);
    DeleteMsgPort(timerPort);
}

void calculate_year_month_day(int days, int *year, int *month, int *day) {
    *year = 1978;

    if (days < 0) {
        // Handle dates before 1978
        while (days < 0) {
            (*year)--;  // Go to the previous year
            int leap = ((*year % 4 == 0 && (*year % 100 != 0)) || (*year % 400 == 0)) ? 1 : 0;
            days += 365 + leap;
        }
    } else {
        // Handle dates after 1978
        while (days >= 365) {
            int leap = ((*year % 4 == 0 && (*year % 100 != 0)) || (*year % 400 == 0)) ? 1 : 0;
            if (days < 365 + leap) break;
            days -= 365 + leap;
            (*year)++;
        }
    }

    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    for (*month = 1; *month <= 12; (*month)++) {
        int dim = days_in_month[*month - 1];
        if (*month == 2 && ((*year % 4 == 0 && (*year % 100 != 0)) || (*year % 400 == 0))) {
            dim++;  // Leap year adjustment
        }
        if (days < dim) break;
        days -= dim;
    }

    *day = days + 1;
}

int month_name_to_number(const char *month_name) {
    static const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                                   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    for (int i = 0; i < 12; i++) {
        if (strcasecmp(month_name, months[i]) == 0) return i + 1;
    }
    return -1; // Invalid month
}

void show_date(const struct TimeVal *tv, const char *format) {
    int year, month, day;
    calculate_year_month_day(tv->tv_secs / (24 * 60 * 60), &year, &month, &day);

    int hours = (tv->tv_secs % 86400) / 3600;
    int minutes = (tv->tv_secs % 3600) / 60;
    int seconds = tv->tv_secs % 60;

    int dow = calculate_day_of_week(year, month, day);

    const char *day_abbr[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char *day_names[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

    char buffer[256] = {0}; // Final formatted output
    const char *f = format; // Pointer to format string
    char *out = buffer; // Pointer to write formatted output

    while (*f) {
        if (*f == '%') {
            f++;
            switch (*f) {
                case 'Y': out += sprintf(out, "%04d", year); break;
                case 'y': out += sprintf(out, "%02d", year % 100); break;
                case 'm': out += sprintf(out, "%02d", month); break;
                case 'd': out += sprintf(out, "%02d", day); break;
                case 'H': out += sprintf(out, "%02d", hours); break;
                case 'M': out += sprintf(out, "%02d", minutes); break;
                case 'S': out += sprintf(out, "%02d", seconds); break;
                case 'a': out += sprintf(out, "%s", day_abbr[dow]); break;
                case 'A': out += sprintf(out, "%s", day_names[dow]); break;
                case 'n': out += sprintf(out, "\n"); break;
                case 'E': out += sprintf(out, "1978-01-01"); break;
                // New options
                case 'C': out += sprintf(out, "%02d", year / 100); break; // Century
                case 'j': out += sprintf(out, "%03d", calculate_day_of_year(year, month, day)); break; // Day of the year
                case 'U': out += sprintf(out, "%02d", calculate_week_number(year, month, day, 0)); break; // Week (Sunday start)
                case 'W': out += sprintf(out, "%02d", calculate_week_number(year, month, day, 1)); break; // Week (Monday start)
                case 'p': out += sprintf(out, "%s", hours >= 12 ? "PM" : "AM"); break; // AM/PM
                case 'P': out += sprintf(out, "%s", hours >= 12 ? "pm" : "am"); break; // am/pm
                case 'z': out += sprintf(out, "+0000"); break; // Dummy timezone offset (can be customized)
                case 'Z': out += sprintf(out, "UTC"); break; // Dummy timezone name (can be customized)
                case 'c': out += sprintf(out, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hours, minutes, seconds); break; // Locale's date and time
                case 'x': out += sprintf(out, "%02d/%02d/%02d", month, day, year % 100); break; // Locale's date
                case 'X': out += sprintf(out, "%02d:%02d:%02d", hours, minutes, seconds); break; // Locale's time
                case '%': *out++ = '%'; break; // Literal '%'
                case 't': *out++ = '\t'; break; // Tab

                default:
                    *out++ = '%';
                    *out++ = *f;
                    break;
            }
        } else {
            *out++ = *f;
        }
        f++;
    }

    *out = '\0'; // Null-terminate the buffer
    printf("%s\n", buffer); // Print the final formatted output
}
