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

namespace common {

const std::unordered_set<std::string> commands = {"start", "time", "weather", "sleep"};

inline bool HandCommand(std::string request_text) {
    for(const auto& command : commands) {
        if(!StringTools::startsWith('/' + command, request_text)) {
            return false;
        }
    }
    return true;
}

void GetWeatherData(boost::property_tree::ptree& pt, std::string response) {
    std::stringstream ss(response);
    boost::property_tree::read_json(ss, pt);
}

} // namespace common

namespace http_processor {

inline void get_http_response(std::string& response_str, const std::string& in_city) {
    
    const std::string host_ip = u8"api.openweathermap.org";
    const std::string target = u8"/data/2.5/weather?q=" + in_city 
        + u8"&units=metric&lang=ru&appid=79d1ca96933b0328e1c7e3e7a26cb347";

    boost::asio::io_context ioc;

    boost::asio::ip::tcp::resolver resolver(ioc);
    boost::asio::ip::tcp::socket socket(ioc);
    boost::asio::connect(socket, resolver.resolve(host_ip, "80"));
            
    boost::beast::http::request<boost::beast::http::string_body> req(boost::beast::http::verb::get, target, 11);
    req.set(boost::beast::http::field::host, host_ip);
    req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    boost::beast::http::write(socket, req);
     
    {
        boost::beast::flat_buffer buffer;
        boost::beast::http::response<boost::beast::http::dynamic_body> res;
        boost::beast::http::read(socket, buffer, res);
        response_str = boost::beast::buffers_to_string(res.body().data());
    }
}

} // namepspace http_processor