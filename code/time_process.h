#include <cstdint>          // uint16_t
#include <ctime>            // time_t, tm
#include <string_view>      // std::string_view
#include <unordered_map>    // std::unordered_map
#include <map>              // std::map
#include <string>           // std::stoi, std::string
#include <stdexcept>        // std::invalid_argument
#include <functional>       // std::invoke

using namespace std::literals;

namespace time_processor {

const std::string MY_PATH = "/home/rgoerv/rgoerv_bot/lession_photos/"s;
const std::string BLUE_WEEK_FOLDER = "blue_week/"s;
const std::string RED_WEEK_FOLDER = "red_week/"s;

const std::string_view TODAY_RU = "сегодня"sv;
const std::string_view TOMORROW_RU = "завтра"sv;

const std::string SUFFIX_FOR_WEEKDAY = "day.jpg"s;

const std::unordered_map<int, const std::string> posw_to_weekday = {
    {0, "Mon"s},
    {1, "Tues"s},
    {2, "Wednes"s},
    // {3, "Thurs"s}, 
    {4, "Fri"s}
};

const std::unordered_map<std::string_view, int> wday_ru_to_posw = {
    {"пн"sv, 0},
    {"вт"sv, 1},
    {"ср"sv, 2},
    // {"чт"sv, 3},
    {"пт"sv, 4}
};

struct time_cut {
    explicit time_cut(std::string_view time /*[operand(1)][hour(2)]:[minute(2)](lenght = 6)*/) {
        minus = time[0] == '-' ? true : false;
        hour = static_cast<uint16_t>(std::stoi(static_cast<std::string>(time.substr(1, 2))));
        minute = static_cast<uint16_t>(std::stoi(static_cast<std::string>(time.substr(4, 2))));
    }

    uint16_t hour:4;    // [0, 14]
    uint16_t minute:6;  // [0, 59]
    bool minus = false;
};

struct time_cut_hasher {
    size_t operator()(const time_cut& time) const {
        return std::hash<uint16_t>{}(time.hour) * 37 * 0 +
                std::hash<uint16_t>{}(time.minute) * 37 * 1 +
                std::hash<bool>{}(time.minus) * 37 * 2;
    }
}; // don't used

const std::unordered_map<int, time_cut> timezone_utc =
{
    {-43200, time_cut{"-12:00"}},
    {-39600, time_cut{"-11:00"}},
    {-36000, time_cut{"-10:00"}},
    {-34200, time_cut{"-09:30"}},
    {-32400, time_cut{"-09:00"}},
    {-28800, time_cut{"-08:00"}},
    {-25200, time_cut{"-07:00"}},
    {-21600, time_cut{"-06:00"}},
    {-18000, time_cut{"-05:00"}},
    {-16200, time_cut{"-04:30"}},
    {-14400, time_cut{"-04:00"}},
    {-12600, time_cut{"-03:00"}},
    {-10800, time_cut{"-03:00"}},
    {-7200,  time_cut{"-02:00"}},
    {-3600,  time_cut{"-01:00"}},
    {0,      time_cut{"00:00"}},
    {3600,   time_cut{"+01:00"}},
    {7200,   time_cut{"+02:00"}},
    {10800,  time_cut{"+03:00"}},
    {12600,  time_cut{"+03:30"}},
    {14400,  time_cut{"+04:00"}},
    {16200,  time_cut{"+04:30"}},
    {18000,  time_cut{"+05:00"}},
    {19800,  time_cut{"+05:30"}},
    {20700,  time_cut{"+05:45"}},
    {21600,  time_cut{"+06:00"}},
    {23400,  time_cut{"+06:30"}},
    {25200,  time_cut{"+07:00"}},
    {28800,  time_cut{"+08:00"}},
    {32400,  time_cut{"+09:00"}},
    {34200,  time_cut{"+09:30"}},
    {36000,  time_cut{"+10:00"}},
    {37800,  time_cut{"+10:30"}},
    {39600,  time_cut{"+11:00"}},
    {41400,  time_cut{"+11:30"}},
    {43200,  time_cut{"+12:00"}},
    {45900,  time_cut{"+12:45"}},
    {46800,  time_cut{"+13:00"}},
    {50400,  time_cut{"+14:00"}}
};

inline const std::string GetStringTime(const int timezone) {
    time_t unix_time = time(NULL);
    tm* utc_time = gmtime(&unix_time);
    const time_cut shift_time = timezone_utc.at(timezone);

    if(shift_time.minus) {
        utc_time->tm_min -= shift_time.minute;
        utc_time->tm_hour -= shift_time.hour;

        if(utc_time->tm_min < 0) {
            utc_time->tm_hour -= 1;
            utc_time->tm_min += 60; 
        }
        if(utc_time->tm_hour < 0 ) {
            utc_time->tm_hour += 24;
        }
    }
    else {
        utc_time->tm_min += shift_time.minute;
        utc_time->tm_hour += shift_time.hour;

        if(utc_time->tm_min > 60) {
            utc_time->tm_hour += 1;
            utc_time->tm_min -= 60;
        }
        if(utc_time->tm_hour > 23) {
            utc_time->tm_hour -= 24;
        }
    }

    const std::string str_time = static_cast<std::string>(asctime(utc_time)).substr(11, 5);
    return str_time;
}

inline const std::string GetPhotoPath(std::string_view day) {
    time_t unix_time = time(NULL);
    tm* localtime_ = localtime(&unix_time);

    int week_day = 0;
    if(day == TODAY_RU) {
        week_day = localtime_->tm_wday - 1;
    } else if (day == TOMORROW_RU) {
        week_day = localtime_->tm_wday;
    } else {
        if(wday_ru_to_posw.count(day) > 0) {
            week_day = wday_ru_to_posw.at(day);
        } else {
            throw std::invalid_argument("In this day lession уке!");
        }
    }

    const int week_number = (localtime_->tm_yday - week_day) / 7;
    std::string result_path = MY_PATH + ((week_number % 2) > 0 ? BLUE_WEEK_FOLDER : RED_WEEK_FOLDER);
    
    const std::string wday_str = std::invoke([&week_day](){
        if(posw_to_weekday.count(week_day) > 0) {
            return posw_to_weekday.at(week_day);
        }   
        else {
            throw std::invalid_argument("In this day lession уке!!");
        }
    });
    
    result_path += wday_str + SUFFIX_FOR_WEEKDAY;
    return result_path;
} 

} // namespace TimeManagement