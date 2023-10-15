#include "common.h"
#include "time_process.h"
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
#include <fstream>

using namespace std::literals;

const std::string ERROR_CODE = "404";
bool work_permisson = true;

int main() {
    TgBot::Bot bot(static_cast<std::string>(BOT_TOKEN));

    time_t unix_time = time(NULL);
    tm* localtime_ = localtime(&unix_time);

    std::ofstream LOG("log.txt", std::ios::binary|std::ios::out|std::ios::app);

    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        try {
            if(message->chat->firstName.empty()) {
                bot.getApi().sendMessage(message->chat->id, "Hi!"s);
            } else {
                bot.getApi().sendMessage(message->chat->id, "Hi, " + message->chat->firstName + '!');
            }
        } catch(...) { return; } 
    });

    bot.getEvents().onCommand("sleep", [&bot](TgBot::Message::Ptr message){
        try {
            if(message->chat->id == ADMIN_ID) {
                bot.getApi().sendMessage(message->chat->id, "Goodbye! Zzz..."s);
                work_permisson = false;
                return;
            } else {
                bot.getApi().sendMessage(message->chat->id, "You don't have authority for perform this command."s);
            }
        } catch(...) { 
            std::cout << "Что-то пошло не так" << std::endl;
            return;
        }
    });

    bot.getEvents().onCommand("weather", [&bot](TgBot::Message::Ptr message){
        try {
            std::string_view weather_query = message->text;
            weather_query.remove_prefix("/weather"s.size());

            if(size_t cityname_begin = weather_query.find_first_not_of(' ');
                cityname_begin == weather_query.npos) {
                bot.getApi().sendMessage(message->chat->id, "Please, write a city name."s);
                return;
            } else {
                weather_query.remove_prefix(cityname_begin);
            }

            const std::string cityname = static_cast<std::string>(weather_query);

            std::string response_str;
            http_processor::get_http_response(response_str, cityname);
            boost::property_tree::ptree response;
            common::GetWeatherData(response, response_str);

            if(StringTools::startsWith(response.get_child("cod").data(), ERROR_CODE)) {
                bot.getApi().sendMessage(message->chat->id, response.get_child("message").data());
                return;
            }
        
            const std::string temp = std::to_string(static_cast<int>(response.get_child("main").get<double>("temp")));
            const std::string feels_like = std::to_string(static_cast<int>(response.get_child("main").get<double>("feels_like")));
            const std::string str_time = time_processor::GetStringTime(response.get<int>("timezone"));
        
            bot.getApi().sendMessage(message->chat->id, 
            ("Right now, weather in " + static_cast<std::string>(weather_query) + " : " + temp 
            + "\nFeels like : " + feels_like
            + "\nTime in this city : " + str_time));
        } catch(...) {
            std::cout << "Что-то пошло не так" << std::endl;
            return; 
        }
    });

    bot.getEvents().onCommand("lession", [&bot](TgBot::Message::Ptr message) {
        // сегодня завтра пн вт ср чт пт // текущей недели
        try {
            std::string_view lession_query = message->text;
            const std::string lession_str = "/lession";
            if(!(lession_str.size() + 1 < lession_query.size())) {
                bot.getApi().sendMessage(message->chat->id, "Please, write correct query(сегодня завтра пн вт ср чт пт).");
                return;
            }

            std::string query_day = message->text.substr("/lession"s.size() + 1, message->text.npos);

            const std::string photoFilePath = std::invoke([&bot, &message, &query_day](){
                try {
                    return time_processor::GetPhotoPath(query_day);
                } catch(const std::invalid_argument& e) {
                    bot.getApi().sendMessage(message->chat->id, e.what());
                    return ""s;
                }
            });

            if(photoFilePath.empty()) { return; }
        
            const std::string photoMimeType = "image/jpeg";
        
            bot.getApi().sendPhoto(message->chat->id, TgBot::InputFile::fromFile(photoFilePath, photoMimeType));
        } catch(...) {
            std::cout << "Что-то пошло не так" << std::endl;
            return;
        }
    });

    bot.getEvents().onAnyMessage([&bot, &localtime_, &LOG](TgBot::Message::Ptr message) {
        std::string info = static_cast<std::string>(asctime(localtime_)) + " USER "s + message->from->username + " WROTE: "s + message->text + "\n"s;
        LOG.write(reinterpret_cast<const char*>(info.data()), info.size());

        printf("User wrote %s\n", message->text.c_str());
        if(!common::HandCommand(message->text) || message->text.empty()) {
            return;
        }
        bot.getApi().sendMessage(message->chat->id, "I don't understand this : " + message->text + ". Please, write correct query.");
    });

    //
    try {
        std::string header_info = static_cast<std::string>(asctime(localtime_)) + " BOT USERNAME: "s + bot.getApi().getMe()->username + "\n"s;
        // LOG << header_info;
        LOG.write(reinterpret_cast<const char*>(header_info.data()), header_info.size());
        
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (work_permisson) {
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        std::string error_info = static_cast<std::string>(asctime(localtime_)) + " error: " + e.what();
        LOG.write(reinterpret_cast<const char*>(error_info.data()), error_info.size());
        printf("error: %s\n", e.what());
        LOG.close();
    }
    return 0;
}