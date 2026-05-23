#include "AdaDateTime.h"
#include "../state.h"

#include <algorithm>
#include <cassert>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

#define CHECK_INSTANCE_CALL if (args.find("this") == args.end()) return false

namespace {

struct DateParts { int year; int month; int day; };
struct TimeParts { int hour; int minute; int second; };
struct DateTimeParts { DateParts date; TimeParts time; };

NdaVariant stringValue(NdaState *state, const std::string &value)
{
    NdaVariant ret;
    ret.fromString(state->stringType(), value);
    return ret;
}

NdaVariant keyValue(NdaState *state, const std::string &key)
{
    return stringValue(state, key);
}

void setMember(NdaState *state, NdaVariant &object, const std::string &key, const NdaVariant &value)
{
    object.appendToDict(keyValue(state, key), value);
}

std::string memberString(NdaState *state, const NdaVariant &object, const std::string &key)
{
    if (object.type() != Nda::Dict)
        return std::string();

    auto memberKey = keyValue(state, key);
    if (!object.contains(memberKey))
        return std::string();

    auto copy = object;
    return copy.writeDictAccess(memberKey).toString();
}

std::string padInt(int value, int width)
{
    std::ostringstream out;
    out << std::setw(width) << std::setfill('0') << value;
    return out.str();
}

bool isLeap(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int daysInMonth(int year, int month)
{
    static const int days[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    if (month < 1 || month > 12)
        return 0;
    if (month == 2 && isLeap(year))
        return 29;
    return days[month - 1];
}

bool validDate(const DateParts &date)
{
    return date.year >= 1 && date.month >= 1 && date.month <= 12 && date.day >= 1 && date.day <= daysInMonth(date.year, date.month);
}

bool validTime(const TimeParts &time)
{
    return time.hour >= 0 && time.hour <= 23 && time.minute >= 0 && time.minute <= 59 && time.second >= 0 && time.second <= 59;
}

// Howard Hinnant's civil calendar algorithms, public domain.
int64_t daysFromCivil(int year, unsigned month, unsigned day)
{
    year -= month <= 2;
    const int era = (year >= 0 ? year : year - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(year - era * 400);
    const unsigned doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<int>(doe) - 719468;
}

DateParts civilFromDays(int64_t z)
{
    z += 719468;
    const int era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    int year = static_cast<int>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    const unsigned day = doy - (153 * mp + 2) / 5 + 1;
    const unsigned month = mp + (mp < 10 ? 3 : -9);
    year += month <= 2;
    return DateParts{year, static_cast<int>(month), static_cast<int>(day)};
}

std::string isoDate(const DateParts &date)
{
    if (!validDate(date))
        return std::string();
    return padInt(date.year, 4) + "-" + padInt(date.month, 2) + "-" + padInt(date.day, 2);
}

std::string isoTime(const TimeParts &time)
{
    if (!validTime(time))
        return std::string();
    return padInt(time.hour, 2) + ":" + padInt(time.minute, 2) + ":" + padInt(time.second, 2);
}

std::string isoDateTime(const DateTimeParts &dateTime)
{
    if (!validDate(dateTime.date) || !validTime(dateTime.time))
        return std::string();
    return isoDate(dateTime.date) + "T" + isoTime(dateTime.time);
}

int readFixedInt(const std::string &text, size_t &pos, int width, bool &ok)
{
    if (pos + static_cast<size_t>(width) > text.size()) {
        ok = false;
        return 0;
    }
    int value = 0;
    for (int i = 0; i < width; ++i) {
        char c = text[pos + i];
        if (c < '0' || c > '9') {
            ok = false;
            return 0;
        }
        value = value * 10 + (c - '0');
    }
    pos += width;
    return value;
}

bool parseByFormat(const std::string &text, const std::string &format, DateParts *date, TimeParts *time)
{
    DateParts d{0, 0, 0};
    TimeParts t{0, 0, 0};
    bool hasDate = false;
    bool hasTime = false;
    bool ok = true;
    size_t ti = 0;

    for (size_t fi = 0; fi < format.size() && ok;) {
        if (format.compare(fi, 4, "yyyy") == 0) {
            d.year = readFixedInt(text, ti, 4, ok);
            hasDate = true;
            fi += 4;
        } else if (format.compare(fi, 2, "yy") == 0) {
            d.year = 2000 + readFixedInt(text, ti, 2, ok);
            hasDate = true;
            fi += 2;
        } else if (format.compare(fi, 2, "MM") == 0) {
            d.month = readFixedInt(text, ti, 2, ok);
            hasDate = true;
            fi += 2;
        } else if (format.compare(fi, 2, "dd") == 0) {
            d.day = readFixedInt(text, ti, 2, ok);
            hasDate = true;
            fi += 2;
        } else if (format.compare(fi, 2, "HH") == 0) {
            t.hour = readFixedInt(text, ti, 2, ok);
            hasTime = true;
            fi += 2;
        } else if (format.compare(fi, 2, "mm") == 0) {
            t.minute = readFixedInt(text, ti, 2, ok);
            hasTime = true;
            fi += 2;
        } else if (format.compare(fi, 2, "ss") == 0) {
            t.second = readFixedInt(text, ti, 2, ok);
            hasTime = true;
            fi += 2;
        } else {
            if (ti >= text.size() || text[ti] != format[fi])
                return false;
            ++ti;
            ++fi;
        }
    }

    if (!ok || ti != text.size())
        return false;
    if (date) {
        if (!hasDate || !validDate(d))
            return false;
        *date = d;
    }
    if (time) {
        if (!hasTime || !validTime(t))
            return false;
        *time = t;
    }
    return true;
}

std::string formatByPattern(const std::string &format, const DateParts *date, const TimeParts *time)
{
    std::string out;
    for (size_t i = 0; i < format.size();) {
        if (date && format.compare(i, 4, "yyyy") == 0) {
            out += padInt(date->year, 4);
            i += 4;
        } else if (date && format.compare(i, 2, "yy") == 0) {
            out += padInt(date->year % 100, 2);
            i += 2;
        } else if (date && format.compare(i, 2, "MM") == 0) {
            out += padInt(date->month, 2);
            i += 2;
        } else if (date && format.compare(i, 2, "dd") == 0) {
            out += padInt(date->day, 2);
            i += 2;
        } else if (time && format.compare(i, 2, "HH") == 0) {
            out += padInt(time->hour, 2);
            i += 2;
        } else if (time && format.compare(i, 2, "mm") == 0) {
            out += padInt(time->minute, 2);
            i += 2;
        } else if (time && format.compare(i, 2, "ss") == 0) {
            out += padInt(time->second, 2);
            i += 2;
        } else {
            out += format[i++];
        }
    }
    return out;
}

DateParts parseIsoDate(const std::string &value)
{
    DateParts date{0, 0, 0};
    parseByFormat(value, "yyyy-MM-dd", &date, nullptr);
    return date;
}

TimeParts parseIsoTime(const std::string &value)
{
    TimeParts time{0, 0, 0};
    parseByFormat(value, "HH:mm:ss", nullptr, &time);
    return time;
}

DateTimeParts parseIsoDateTime(const std::string &value)
{
    DateTimeParts dateTime{{0, 0, 0}, {0, 0, 0}};
    parseByFormat(value, "yyyy-MM-ddTHH:mm:ss", &dateTime.date, &dateTime.time);
    return dateTime;
}

DateParts addDays(DateParts date, int64_t days)
{
    if (!validDate(date))
        return date;
    return civilFromDays(daysFromCivil(date.year, static_cast<unsigned>(date.month), static_cast<unsigned>(date.day)) + days);
}

TimeParts addSecs(TimeParts time, int64_t secs, int64_t *dayDelta = nullptr)
{
    if (!validTime(time))
        return time;
    int64_t total = static_cast<int64_t>(time.hour) * 3600 + time.minute * 60 + time.second + secs;
    int64_t days = total / 86400;
    int64_t rem = total % 86400;
    if (rem < 0) {
        rem += 86400;
        --days;
    }
    if (dayDelta)
        *dayDelta = days;
    return TimeParts{static_cast<int>(rem / 3600), static_cast<int>((rem / 60) % 60), static_cast<int>(rem % 60)};
}

DateParts currentDate()
{
    std::time_t now = std::time(nullptr);
    std::tm tm = *std::localtime(&now);
    return DateParts{tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday};
}

TimeParts currentTime()
{
    std::time_t now = std::time(nullptr);
    std::tm tm = *std::localtime(&now);
    return TimeParts{tm.tm_hour, tm.tm_min, tm.tm_sec};
}

DateTimeParts currentDateTime()
{
    std::time_t now = std::time(nullptr);
    std::tm tm = *std::localtime(&now);
    return DateTimeParts{{tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday}, {tm.tm_hour, tm.tm_min, tm.tm_sec}};
}

void setDateReturn(NdaState *state, NdaVariant &ret, const DateParts &date)
{
    ret.initType(state->typeByName("date"));
    setMember(state, ret, "value", stringValue(state, isoDate(date)));
}

void setTimeReturn(NdaState *state, NdaVariant &ret, const TimeParts &time)
{
    ret.initType(state->typeByName("time"));
    setMember(state, ret, "value", stringValue(state, isoTime(time)));
}

void setDateTimeReturn(NdaState *state, NdaVariant &ret, const DateTimeParts &dateTime)
{
    ret.initType(state->typeByName("datetime"));
    setMember(state, ret, "value", stringValue(state, isoDateTime(dateTime)));
}

DateParts dateFromObject(NdaState *state, const NdaVariant &object)
{
    return parseIsoDate(memberString(state, object, "value"));
}

TimeParts timeFromObject(NdaState *state, const NdaVariant &object)
{
    return parseIsoTime(memberString(state, object, "value"));
}

DateTimeParts dateTimeFromObject(NdaState *state, const NdaVariant &object)
{
    return parseIsoDateTime(memberString(state, object, "value"));
}

int64_t secondsFromTime(const TimeParts &time)
{
    return static_cast<int64_t>(time.hour) * 3600 + static_cast<int64_t>(time.minute) * 60 + time.second;
}

int64_t secondsFromDateTime(const DateTimeParts &dateTime)
{
    if (!validDate(dateTime.date) || !validTime(dateTime.time))
        return 0;
    return daysFromCivil(dateTime.date.year, static_cast<unsigned>(dateTime.date.month), static_cast<unsigned>(dateTime.date.day)) * 86400 + secondsFromTime(dateTime.time);
}

std::string stringArg(const Nda::FncValues &args, const char *name)
{
    auto it = args.find(name);
    if (it == args.end())
        return std::string();
    return it->second.toString();
}

int64_t intArg(const Nda::FncValues &args, const char *name, bool &ok)
{
    auto it = args.find(name);
    if (it == args.end()) {
        ok = false;
        return 0;
    }
    return it->second.toInt64(&ok);
}

}

namespace Nda {

void add_AdaDateTime_symbols(NdaState *state)
{
    assert(state);

    state->registerType("Date", "dict");
    state->registerType("Time", "dict");
    state->registerType("DateTime", "dict");

    state->bindFnc("date", "now", {}, [state](const Nda::FncValues&, NdaVariant &ret) -> bool {
        setDateReturn(state, ret, currentDate());
        return true;
    });
    state->bindFnc("date", "today", {}, [state](const Nda::FncValues&, NdaVariant &ret) -> bool {
        setDateReturn(state, ret, currentDate());
        return true;
    });
    state->bindFnc("date", "fromString", {{"text", "string", Nda::InMode}, {"format", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        DateParts date{0, 0, 0};
        parseByFormat(stringArg(args, "text"), stringArg(args, "format"), &date, nullptr);
        setDateReturn(state, ret, date);
        return true;
    });
    state->bindFnc("date", "toString", {{"format", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;
        DateParts date = dateFromObject(state, args.at("this"));
        ret.fromString(state->stringType(), formatByPattern(stringArg(args, "format"), &date, nullptr));
        return true;
    });
    state->bindFnc("date", "addDays", {{"days", "number", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;
        bool ok;
        int64_t days = intArg(args, "days", ok);
        if (!ok)
            return false;
        setDateReturn(state, ret, addDays(dateFromObject(state, args.at("this")), days));
        return true;
    });

    state->bindFnc("time", "now", {}, [state](const Nda::FncValues&, NdaVariant &ret) -> bool {
        setTimeReturn(state, ret, currentTime());
        return true;
    });
    state->bindFnc("time", "fromString", {{"text", "string", Nda::InMode}, {"format", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        TimeParts time{0, 0, 0};
        parseByFormat(stringArg(args, "text"), stringArg(args, "format"), nullptr, &time);
        setTimeReturn(state, ret, time);
        return true;
    });
    state->bindFnc("time", "toString", {{"format", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;
        TimeParts time = timeFromObject(state, args.at("this"));
        ret.fromString(state->stringType(), formatByPattern(stringArg(args, "format"), nullptr, &time));
        return true;
    });
    state->bindFnc("time", "addSecs", {{"secs", "number", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;
        bool ok;
        int64_t secs = intArg(args, "secs", ok);
        if (!ok)
            return false;
        setTimeReturn(state, ret, addSecs(timeFromObject(state, args.at("this")), secs));
        return true;
    });

    state->bindFnc("datetime", "now", {}, [state](const Nda::FncValues&, NdaVariant &ret) -> bool {
        setDateTimeReturn(state, ret, currentDateTime());
        return true;
    });
    state->bindFnc("datetime", "fromString", {{"text", "string", Nda::InMode}, {"format", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        DateTimeParts dateTime{{0, 0, 0}, {0, 0, 0}};
        parseByFormat(stringArg(args, "text"), stringArg(args, "format"), &dateTime.date, &dateTime.time);
        setDateTimeReturn(state, ret, dateTime);
        return true;
    });
    state->bindFnc("datetime", "toString", {{"format", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;
        DateTimeParts dateTime = dateTimeFromObject(state, args.at("this"));
        ret.fromString(state->stringType(), formatByPattern(stringArg(args, "format"), &dateTime.date, &dateTime.time));
        return true;
    });
    state->bindFnc("datetime", "addDays", {{"days", "number", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;
        bool ok;
        int64_t days = intArg(args, "days", ok);
        if (!ok)
            return false;
        DateTimeParts dateTime = dateTimeFromObject(state, args.at("this"));
        dateTime.date = addDays(dateTime.date, days);
        setDateTimeReturn(state, ret, dateTime);
        return true;
    });
    state->bindFnc("datetime", "addSecs", {{"secs", "number", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;
        bool ok;
        int64_t secs = intArg(args, "secs", ok);
        if (!ok)
            return false;
        DateTimeParts dateTime = dateTimeFromObject(state, args.at("this"));
        int64_t dayDelta = 0;
        dateTime.time = addSecs(dateTime.time, secs, &dayDelta);
        dateTime.date = addDays(dateTime.date, dayDelta);
        setDateTimeReturn(state, ret, dateTime);
        return true;
    });

    state->bindFnc("datetime", "secsTo", {{"other", "datetime", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;
        DateTimeParts self = dateTimeFromObject(state, args.at("this"));
        DateTimeParts other = dateTimeFromObject(state, args.at("other"));
        if (!validDate(self.date) || !validTime(self.time) || !validDate(other.date) || !validTime(other.time))
            return false;
        ret.fromNatural(state->naturalType(), secondsFromDateTime(other) - secondsFromDateTime(self));
        return true;
    });

    state->bindFnc("datetime", "setDate", {{"year", "natural", Nda::InMode}, {"month", "natural", Nda::InMode}, {"day", "natural", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;
        bool okYear;
        bool okMonth;
        bool okDay;
        DateParts date{static_cast<int>(intArg(args, "year", okYear)), static_cast<int>(intArg(args, "month", okMonth)), static_cast<int>(intArg(args, "day", okDay))};
        if (!okYear || !okMonth || !okDay || !validDate(date))
            return false;
        DateTimeParts dateTime = dateTimeFromObject(state, args.at("this"));
        if (!validTime(dateTime.time))
            return false;
        dateTime.date = date;
        setDateTimeReturn(state, ret, dateTime);
        return true;
    });

    state->bindFnc("datetime", "setDate", {{"date", "date", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;
        DateParts date = dateFromObject(state, args.at("date"));
        if (!validDate(date))
            return false;
        DateTimeParts dateTime = dateTimeFromObject(state, args.at("this"));
        if (!validTime(dateTime.time))
            return false;
        dateTime.date = date;
        setDateTimeReturn(state, ret, dateTime);
        return true;
    });

    state->bindFnc("datetime", "setTime", {{"hour", "natural", Nda::InMode}, {"minute", "natural", Nda::InMode}, {"second", "natural", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;
        bool okHour;
        bool okMinute;
        bool okSecond;
        TimeParts time{static_cast<int>(intArg(args, "hour", okHour)), static_cast<int>(intArg(args, "minute", okMinute)), static_cast<int>(intArg(args, "second", okSecond))};
        if (!okHour || !okMinute || !okSecond || !validTime(time))
            return false;
        DateTimeParts dateTime = dateTimeFromObject(state, args.at("this"));
        if (!validDate(dateTime.date))
            return false;
        dateTime.time = time;
        setDateTimeReturn(state, ret, dateTime);
        return true;
    });

    state->bindFnc("datetime", "setTime", {{"time", "time", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;
        TimeParts time = timeFromObject(state, args.at("time"));
        if (!validTime(time))
            return false;
        DateTimeParts dateTime = dateTimeFromObject(state, args.at("this"));
        if (!validDate(dateTime.date))
            return false;
        dateTime.time = time;
        setDateTimeReturn(state, ret, dateTime);
        return true;
    });
}

}
