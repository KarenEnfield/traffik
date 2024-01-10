

#include "tfk_logger.h"


tfk_logger::tfk_logger(std::string console_name): console_name_(console_name){
        
    // Set the log level for all loggers
 
    // Get log level from environment variable, default to info if not set
    const char* logLevelStr = std::getenv("TFK_LOG_LEVEL");
    std::string str{"info"};

    if (logLevelStr!=nullptr) // if there is a level specified
        str.assign(logLevelStr);
      
    stringToLogLevel(str);
        
    
    
#ifdef USE_SPDLOG    

    // Create a console logger
    console_logger_ = spdlog::stdout_logger_mt(console_name);
       
    // Log messages with different severity levels
    console_logger_->trace("This is a trace message.");
    console_logger_->debug("This is a debug message.");
    console_logger_->info("This is an information message.");
    console_logger_->warn("This is a warning message.");
    console_logger_->error("This is an error message.");
    console_logger_->critical("This is a critical message.");
  
    // Create a file logger
    // _file_logger = spdlog::rotating_logger_mt("file_logger", "logs/mylogfile", 1048576 * 5, 3);
   
    // Set the log level for both loggers
    // spdlog::set_level(spdlog::level::debug);

    // Log messages
    //_console_logger->info("This is an information message on the console.");
    //_file_logger->warn("This is a warning message logged to the file.");

    // Use formatting
    // int value = 42;
    //_console_logger->error("An error message with a formatted value: {}", value);

    // Flush loggers (this can be important for asynchronous loggers)
    // spdlog::flush_all();
#endif

  
}


// Helper function to convert a string to CustomLogLevel
void tfk_logger::stringToLogLevel(std::string levelStr)
{
    std::transform(levelStr.begin(), levelStr.end(), levelStr.begin(), ::tolower);
    
#ifdef USE_SPDLOG
        // set spdlog
        if (levelStr == "trace") spdlog::set_level(spdlog::level::trace);
        else if (levelStr == "debug")  spdlog::set_level(spdlog::level::debug);
        else if (levelStr == "info")  spdlog::set_level(spdlog::level::info);
        else if (levelStr == "warn")  spdlog::set_level(spdlog::level::warn);
        else if (levelStr == "err")  spdlog::set_level(spdlog::level::err);
        else if (levelStr == "critical")  spdlog::set_level(spdlog::level::critical);
        else if (levelStr == "off")  spdlog::set_level(spdlog::level::off);

        // Default to info if the input is not recognized
        else   spdlog::set_level(spdlog::level::info);

#endif
        // set the internal logging level;
        if (levelStr == "trace") log_level_= CustomLogLevel::trace;
        else if (levelStr == "debug") log_level_= CustomLogLevel::debug;
        else if (levelStr == "info") log_level_= CustomLogLevel::info;
        else if (levelStr == "warn") log_level_= CustomLogLevel::warn;
        else if (levelStr == "err") log_level_= CustomLogLevel::err;
        else if (levelStr == "critical") log_level_= CustomLogLevel::critical;
        else if (levelStr == "off") log_level_= CustomLogLevel::off;

        // Default to info if the input is not recognized
        else {
            log_level_= CustomLogLevel::info;
            levelStr = "info";
        }

        log_level_str_ = levelStr;

}
