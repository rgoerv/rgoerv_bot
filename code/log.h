#include <time.h>
#include <fstream>
#include <string>

template<class Handler>
class LOG {
public:
    LOG() : unix_time(time(NULL)), localtime_(localtime(&unix_time)) {
        LOG_STREAM.open("log.txt", std::ios::binary|std::ios::out|std::ios::app);
    }

    // write(const std::string& data){
    //     const std::string with_time = static_cast<std::string>(asctime(localtime_)) + data;
    //     LOG_STREAM.write(reinterpret_cast<const char*>(with_time.data()), with_time.size());
    // }

    template<class... Args>
    void write(const Args&... args){
        const std::string with_time = static_cast<std::string>(asctime(localtime_)) + args...;
        LOG_STREAM.write(reinterpret_cast<const char*>(with_time.data()), with_time.size());
    }

    ~LOG {

    }

private:
    time_t unix_time;
    tm* localtime_;

    Handler* bot_handler = nullptr;
    std::ofstream LOG_STREAM;
};