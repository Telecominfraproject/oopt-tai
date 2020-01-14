#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include "tai.h"
#include <mutex>
#include <string>
#include <map>

namespace tai {

    static const std::string to_string(tai_log_level_t level) {
        switch (level) {
        case TAI_LOG_LEVEL_DEBUG:
            return "DEBUG";
        case TAI_LOG_LEVEL_INFO:
            return "INFO";
        case TAI_LOG_LEVEL_NOTICE:
            return "INFO";
        case TAI_LOG_LEVEL_WARN:
            return "WARN";
        case TAI_LOG_LEVEL_ERROR:
            return "ERROR";
        case TAI_LOG_LEVEL_CRITICAL:
            return "CRITICAL";
        default:
            return std::to_string(static_cast<int>(level));
        }
    }

    class Logger {
        private:
            Logger() = default;
            ~Logger() = default;
        public:
            Logger(const Logger&) = delete;
            Logger& operator=(const Logger&) = delete;
            Logger(Logger&&) = delete;
            Logger& operator=(Logger&&) = delete;

            static Logger& get_instance() {
                static Logger logger;
                return logger;
            }

            tai_status_t set_log(tai_api_t tai_api_id, tai_log_level_t log_level, tai_log_fn log_fn) {
                std::unique_lock<std::mutex> lk(m_mtx);
                m_fn_map[tai_api_id] = {log_level, log_fn};
                return TAI_STATUS_SUCCESS;
            }

            template <typename ... Args>
            void log(tai_api_t tai_api_id, tai_log_level_t log_level, const std::string& file, int line, const std::string& function, const std::string& format, Args... args) {
                std::unique_lock<std::mutex> lk(m_mtx);
                auto it = m_fn_map.find(tai_api_id);
                if ( it == m_fn_map.end() && log_level >= TAI_LOG_LEVEL_ERROR ) {
                    goto def;
                } else if ( it != m_fn_map.end() && it->second.first <= log_level ){
                    if ( it->second.second != nullptr ) {
                        it->second.second(log_level, file.c_str(), line, function.c_str(), format.c_str(), args...);
                    } else {
                    def:
                        std::fprintf(stderr, ("%s [%s@%d]" + format + "\n").c_str(), to_string(log_level).c_str(), function.c_str(), line, args...);
                    }
                }
            }

        private:
            std::map<tai_api_t, std::pair<tai_log_level_t, tai_log_fn>> m_fn_map;
            std::mutex m_mtx;
    };

#define _TAI_LOG(api, level, format, ...) ::tai::Logger::get_instance().log(api, level, __FILE__, __LINE__, __PRETTY_FUNCTION__, format, ##__VA_ARGS__)
#define TAI_LOG(level, format, ...) _TAI_LOG(TAI_API_UNSPECIFIED, level, format, ##__VA_ARGS__)

#define TAI_DEBUG(format, ...)    TAI_LOG(TAI_LOG_LEVEL_DEBUG,    format, ##__VA_ARGS__)
#define TAI_INFO(format, ...)     TAI_LOG(TAI_LOG_LEVEL_INFO,     format, ##__VA_ARGS__)
#define TAI_WARN(format, ...)     TAI_LOG(TAI_LOG_LEVEL_WARN,     format, ##__VA_ARGS__)
#define TAI_ERROR(format, ...)    TAI_LOG(TAI_LOG_LEVEL_ERROR,    format, ##__VA_ARGS__)
#define TAI_CRITICAL(format, ...) TAI_LOG(TAI_LOG_LEVEL_CRITICAL, format, ##__VA_ARGS__)

}

#endif // __LOGGER_HPP__
