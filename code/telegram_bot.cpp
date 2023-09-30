#include "common.h"
#include "not_public.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <tgbot/tgbot.h>

#include <stdio.h>
#include <time.h>

#include <string>
#include <string_view>
#include <iostream>
#include <cstdio>
#include <cstdlib>

using namespace std::literals;

const std::string ERROR_CODE = "404";
bool work_permisson = true;

int main() {
    TgBot::Bot bot(BOT_TOKEN);

    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        if(message->chat->firstName.empty()) {
            bot.getApi().sendMessage(message->chat->id, "Hi!");
        } else {
            bot.getApi().sendMessage(message->chat->id, "Hi, " + message->chat->firstName + '!');
        }
    });

    bot.getEvents().onCommand("sleep", [&bot](TgBot::Message::Ptr message){
        if(message->chat->id == ADMIN_ID) {
            bot.getApi().sendMessage(message->chat->id, "Goodbye! Zzz...");
            work_permisson = false;
            return;
        } else {
            bot.getApi().sendMessage(message->chat->id, "You don't have authority for perform this command.");
        }
    });

    bot.getEvents().onCommand("weather", [&bot](TgBot::Message::Ptr message){
        std::string_view weather_query = message->text;
        const std::string weather_str = "/weather";

        if(!(weather_str.size() + 1 < weather_query.size())) {
            bot.getApi().sendMessage(message->chat->id, "Please, write a city name.");
            return;
        }

        std::string city = message->text.substr("/weather"s.size() + 1, message->text.npos);

        const std::string url = u8"https://api.openweathermap.org/data/2.5/weather?q=" + city 
                        + u8"&units=metric&lang=ru&appid=79d1ca96933b0328e1c7e3e7a26cb347";

        const boost::property_tree::ptree& response = Common::GetWeatherData(CurlReceiver::GetResponse(url.c_str()));

        if(StringTools::startsWith(response.get_child("cod").data(), ERROR_CODE)) {
            bot.getApi().sendMessage(message->chat->id, response.get_child("message").data());
            return;
        }
        
        const std::string temp = std::to_string(static_cast<int>(response.get_child("main").get<double>("temp")));
        const std::string feels_like = std::to_string(static_cast<int>(response.get_child("main").get<double>("feels_like")));
        const std::string str_time = TimeManagement::GetStringTime(response.get<int>("timezone"));
        
        bot.getApi().sendMessage(message->chat->id, 
        ("Right now, weather in " + city + " : " + temp 
        + "\nFeels like : " + feels_like
        + "\nTime in this city : " + str_time));
    });

    bot.getEvents().onCommand("lession", [&bot](TgBot::Message::Ptr message) {
        // сегодня завтра пн вт ср чт пт // текущей недели

        std::string_view lession_query = message->text;
        const std::string lession_str = "/lession";
        if(!(lession_str.size() + 1 < lession_query.size())) {
            bot.getApi().sendMessage(message->chat->id, "Please, write correct query(сегодня завтра пн вт ср чт пт).");
            return;
        }

        std::string query_day = message->text.substr("/weather"s.size() + 1, message->text.npos);

        const std::string photoFilePath = "Friday.jpg";
        const std::string photoMimeType = "image/jpeg";

        bot.getApi().sendPhoto(message->chat->id, TgBot::InputFile::fromFile(photoFilePath, photoMimeType));
        bot.getApi().setChatAdministratorCustomTitle(message->chat->id, ADMIN_ID, "aboba");
    });

    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if(!Common::HandCommand(message->text) || message->text.empty()) {
            return;
        }
        bot.getApi().sendMessage(message->chat->id, "I don't understand this : " + message->text + ". Please, write correct query.");
    });

    //
    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (work_permisson) {
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
    }
    return 0;
}