#include <tgbot/tgbot.h>
#include <curl/curl.h>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <ctime>

#include "json.h"

namespace TimeManagement {

struct mini_time {
    explicit mini_time(std::string_view time /*[operand(1)][hour(2)]:[minute(2)](lenght = 6)*/) {
        minus = time[0] == '-' ? true : false;
        hour = static_cast<uint16_t>(std::stoi(static_cast<std::string>(time.substr(1, 2))));
        minute = static_cast<uint16_t>(std::stoi(static_cast<std::string>(time.substr(4, 2))));
    }

    uint16_t hour:4;    // [0, 14]
    uint16_t minute:6;  // [0, 59]
    bool minus = false;
};

struct Hasher {
    size_t operator()(const mini_time& time) const {
        return std::hash<uint16_t>{}(time.hour) * 37 * 0 +
                std::hash<uint16_t>{}(time.minute) * 37 * 1 +
                std::hash<bool>{}(time.minus) * 37 * 2;
    }
};

const std::unordered_map<std::string_view, int> wday_to_posw = 
{
    {"Mon", 0},
    {"Tue", 1},
    {"Wed", 2},
    {"Thu", 3},
    {"Fri", 4},
    {"Sat", 5},
    {"Sun", 6}
};

const std::map<int,  mini_time> timezone_utc =
{
    {-43200, mini_time{"-12:00"}},
    {-39600, mini_time{"-11:00"}},
    {-36000, mini_time{"-10:00"}},
    {-34200, mini_time{"-09:30"}},
    {-32400, mini_time{"-09:00"}},
    {-28800, mini_time{"-08:00"}},
    {-25200, mini_time{"-07:00"}},
    {-21600, mini_time{"-06:00"}},
    {-18000, mini_time{"-05:00"}},
    {-16200, mini_time{"-04:30"}},
    {-14400, mini_time{"-04:00"}},
    {-12600, mini_time{"-03:00"}},
    {-10800, mini_time{"-03:00"}},
    {-7200, mini_time{"-02:00"}},
    {-3600, mini_time{"-01:00"}},
    {0, mini_time{"00:00"}},
    {3600, mini_time{"+01:00"}},
    {7200, mini_time{"+02:00"}},
    {10800, mini_time{"+03:00"}},
    {12600, mini_time{"+03:30"}},
    {14400, mini_time{"+04:00"}},
    {16200, mini_time{"+04:30"}},
    {18000, mini_time{"+05:00"}},
    {19800, mini_time{"+05:30"}},
    {20700, mini_time{"+05:45"}},
    {21600, mini_time{"+06:00"}},
    {23400, mini_time{"+06:30"}},
    {25200, mini_time{"+07:00"}},
    {28800, mini_time{"+08:00"}},
    {32400, mini_time{"+09:00"}},
    {34200, mini_time{"+09:30"}},
    {36000, mini_time{"+10:00"}},
    {37800, mini_time{"+10:30"}},
    {39600, mini_time{"+11:00"}},
    {41400, mini_time{"+11:30"}},
    {43200, mini_time{"+12:00"}},
    {45900, mini_time{"+12:45"}},
    {46800, mini_time{"+13:00"}},
    {50400, mini_time{"+14:00"}}
};

inline const std::string GetStringTime(const int timezone) {
    time_t unix_time = time(NULL);
    tm* utc_time = gmtime(&unix_time);
    const mini_time shift_time = timezone_utc.at(timezone);

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

} // namespace TimeManagement

namespace Common {

std::unordered_set<std::string> commands = {"start", "time", "weather", "sleep"};

inline bool HandCommand(std::string request_text) {
    for(const auto& command : commands) {
        if(!StringTools::startsWith('/' + command, request_text)) {
            return false;
        }
    }
    return true;
}

json::Document GetWeatherData(std::string response) {
    std::istringstream is(response);
    return json::Load(is);
}

} // namespace Common

namespace CurlReceiver {

static size_t Writer(char* buffer, size_t size, size_t nmemb, std::string* html) {
    size_t result = 0;
    if(buffer != NULL) {
        html->append(buffer, size * nmemb);
        result = size * nmemb;
    }
    return result;
}

inline std::string GetResponse(const char* link) {
    CURL* curl;
    std::string data;
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, link);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Writer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return data;
}

} // namepspace CurlReceiver