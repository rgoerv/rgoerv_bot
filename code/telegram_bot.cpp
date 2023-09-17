#include "common.h"
#include "json.h"

#include <stdio.h>
#include <time.h>

#include <tgbot/tgbot.h>

#include <string>
#include <string_view>
#include <iostream>
#include <locale>
#include <cstdio>
#include <cstdlib>

#include <filesystem>

using namespace std::literals;

/* */

const int64_t admin_id = /* admin id */;
const std::string ERROR_CODE = "404";
const char BOT_TOKEN[] = ""/* bot token */;
bool work_permisson = true;

/* */

// std::string url = "https://api.openweathermap.org/data/2.5/weather?q=Москва&units=metric&lang=ru&appid=79d1ca96933b0328e1c7e3e7a26cb347";

int main() {

    TgBot::Bot bot(BOT_TOKEN);

    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        if(message->chat->firstName.empty()) {
            bot.getApi().sendMessage(message->chat->id, "Hi!");
        }
        else {
            bot.getApi().sendMessage(message->chat->id, "Hi, " + message->chat->firstName + '!');
        }
        
    });

    bot.getEvents().onCommand("sleep", [&bot](TgBot::Message::Ptr message){
        if(message->chat->id == admin_id) {
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

        const json::Document response = Common::GetWeatherData(CurlReceiver::GetResponse(url.c_str()));
        const auto& dict_data = response.GetRoot().AsDict();

        if(dict_data.at("cod").IsString() && StringTools::startsWith(dict_data.at("cod").AsString(), ERROR_CODE)) {
            bot.getApi().sendMessage(message->chat->id, dict_data.at("message").AsString());
            return;
        }
        
        const std::string temp = std::to_string(static_cast<int>(dict_data.at("main").AsDict().at("temp").AsDouble()));
        const std::string feels_like = std::to_string(static_cast<int>(dict_data.at("main").AsDict().at("feels_like").AsDouble()));
        const std::string str_time = TimeManagement::GetStringTime(dict_data.at("timezone").AsInt());
        
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

        std::string photoFilePath;
        const std::string photoMimeType = "image/jpeg";

        try {
            photoFilePath = TimeManagement::GetPhotoPath(query_day);
        } catch(const std::exception& e) {
            bot.getApi().sendMessage(message->chat->id, "In this day lessions don't exists"s);
            return;
        }

        // std::cout << std::boolalpha << std::filesystem::exists(std::filesystem::path(photoFilePath)) << std::endl;

        bot.getApi().sendPhoto(message->chat->id, TgBot::InputFile::fromFile(photoFilePath, photoMimeType));
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