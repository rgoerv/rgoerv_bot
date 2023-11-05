#include <time.h>
#include <fstream>
#include <string>

template<class Handler>
class LOG {
public:
    LOG() : unix_time(time(NULL)), localtime_(localtime(&unix_time)) {
        LOG_STREAM.open("log.txt", std::ios::binary|std::ios::out|std::ios::app);
    }

    // for the stude black magic, variadic templates
    template<class Arg, class... Args>
    static std::string& make_string(std::string& str, const Arg& arg, const Args&... args) {
        if constexpr (sizeof...(args) == 1) {
            str += std::string(arg);
        }
        else {
            str += std::string(arg) + " ";
        }

        if constexpr (sizeof...(args) != 0) {
            return make_string(str, args...);
        }
        return str;
    }

    template<class... Args>
    void write(const Args&... args){
        std::string_view str_time = static_cast<std::string_view>(asctime(localtime_));
        str_time.remove_suffix(5);
        const std::string with_time =  std::invoke([&str_time, &args...](){
            std::string str = static_cast<std::string>(str_time) + " ";
            str += make_string(str, args...);
            return str;
        });

        LOG_STREAM.write(reinterpret_cast<const char*>(with_time.data()), with_time.size());
    }

    ~LOG() {
        LOG_STREAM.close();
    }

private:
    time_t unix_time;
    tm* localtime_;

    Handler* bot_handler = nullptr;
    std::ofstream LOG_STREAM;
};