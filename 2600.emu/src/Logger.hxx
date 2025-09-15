#pragma once

// TODO: redirect into imagine logger system

#include <string_view>

class Logger {

  public:

    enum class Level {
      ERR = 0, // cannot use ERROR???
      INFO = 1,
      DEBUG = 2,
      MIN = ERR,
      MAX = DEBUG
    };

  public:
    constexpr Logger() {}

    static Logger& instance()
    {
    	static Logger l;
    	return l;
    };

    static void log(std::string_view message, Level level) {}

    static void error(std::string_view message) {}

    static void info(std::string_view message) {}

    static void debug(std::string_view message) {}

    void setLogParameters(int logLevel, bool logToConsole) {}
    void setLogParameters(Level logLevel, bool logToConsole) {}
};
