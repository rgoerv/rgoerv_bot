#include <tgbot/tgbot.h>
#include <curl/curl.h>

#include <boost/beast.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <ctime>
#include <stdexcept>

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

boost::property_tree::ptree GetWeatherData(std::string response) {
    std::stringstream ss(response);
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(ss, pt);
    return pt;
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